// Native Windows UI for Bishop/Goddess password values.
// Self-contained: Encode + Decode + MD5.
//
// Build:
// g++ -std=c++17 -O2 -mwindows -o password_tool_ui.exe password_tool_ui.cpp -luser32 -lgdi32

#include <windows.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

// ============================================================
// UI IDs
// ============================================================

constexpr int kInput  = 1001;
constexpr int kOutput = 1002;
constexpr int kEncode = 1003;
constexpr int kDecode = 1004;
constexpr int kMd5    = 1005;

constexpr int kAppIcon = 1;

// ============================================================
// Legacy password codec
// ============================================================

constexpr std::array<std::pair<int, int>, 7> kSwaps = {
    std::pair{0, 13},
    std::pair{31, 25},
    std::pair{12, 30},
    std::pair{7, 19},
    std::pair{3, 21},
    std::pair{9, 20},
    std::pair{15, 18},
};

constexpr std::string_view kOutputAlphabet =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_[]>";

HWND input_box  = nullptr;
HWND output_box = nullptr;
HWND md5_box    = nullptr;

// ============================================================
// Legacy Encode / Decode
// ============================================================

int char_to_number(char character)
{
    const unsigned value =
        static_cast<unsigned char>(character) ^ 0x97U;

    return static_cast<int>(
        ((value << 7U) | (value >> 1U)) & 0x1FU
    );
}

char number_to_char(
    int number,
    std::string_view alphabet = kOutputAlphabet)
{
    for (char value : alphabet) {
        if (char_to_number(value) == number)
            return value;
    }

    throw std::runtime_error(
        "Cannot encode number"
    );
}

void swap_characters(std::string& value)
{
    for (const auto& [left, right] : kSwaps)
        std::swap(value[left], value[right]);
}

bool printable_ascii(const std::string& text)
{
    return std::all_of(
        text.begin(),
        text.end(),
        [](unsigned char character) {
            return character >= 0x20 &&
                   character <= 0x7e;
        }
    );
}

std::mt19937& engine()
{
    static std::mt19937 value(
        static_cast<unsigned>(
            std::chrono::high_resolution_clock::now()
                .time_since_epoch()
                .count()
        )
    );

    return value;
}

char random_output_character()
{
    return kOutputAlphabet[
        std::uniform_int_distribution<std::size_t>(
            0,
            kOutputAlphabet.size() - 1
        )(engine())
    ];
}

std::string encode(const std::string& password)
{
    if (
        password.size() > 20 ||
        !printable_ascii(password)
    ) {
        throw std::runtime_error(
            "Password must be 0..20 printable ASCII characters."
        );
    }

    const int max_key_length =
        std::min(
            19,
            30 - static_cast<int>(password.size())
        );

    if (max_key_length < 10) {
        throw std::runtime_error(
            "Password is too long."
        );
    }

    const int key_length =
        std::uniform_int_distribution<int>(
            10,
            max_key_length
        )(engine());

    std::string key;
    std::string ciphertext;

    // Generate key characters that keep output
    // inside the restricted legacy alphabet.
    for (
        int key_index = 0;
        key_index < key_length;
        ++key_index
    ) {
        std::string candidates;

        for (char key_char : kOutputAlphabet) {
            bool valid = true;

            for (
                std::size_t index =
                    static_cast<std::size_t>(key_index);
                index < password.size();
                index += key_length
            ) {
                const char cipher =
                    static_cast<char>(
                        (
                            static_cast<unsigned char>(
                                password[index]
                            )
                            - 0x20
                            - 0x3f
                            + static_cast<unsigned char>(
                                key_char
                            )
                            - 0x20
                            + 0x5f
                        ) % 0x5f
                    );

                if (
                    kOutputAlphabet.find(cipher) ==
                    std::string_view::npos
                ) {
                    valid = false;
                    break;
                }
            }

            if (valid)
                candidates += key_char;
        }

        if (candidates.empty()) {
            throw std::runtime_error(
                "Password cannot be encoded with the restricted output alphabet."
            );
        }

        key += candidates[
            std::uniform_int_distribution<std::size_t>(
                0,
                candidates.size() - 1
            )(engine())
        ];
    }

    for (
        std::size_t index = 0;
        index < password.size();
        ++index
    ) {
        ciphertext +=
            static_cast<char>(
                (
                    static_cast<unsigned char>(
                        password[index]
                    )
                    - 0x20
                    - 0x3f
                    + static_cast<unsigned char>(
                        key[index % key.size()]
                    )
                    - 0x20
                    + 0x5f
                ) % 0x5f
            );
    }

    std::string result(
        1,
        number_to_char(key_length)
    );

    result += key;

    result += number_to_char(
        static_cast<int>(
            password.size()
        )
    );

    result += ciphertext;

    while (result.size() < 32)
        result += random_output_character();

    swap_characters(result);

    return result;
}

std::string decode(std::string value)
{
    if (value.size() != 32) {
        throw std::runtime_error(
            "Encoded value must have exactly 32 characters."
        );
    }

    swap_characters(value);

    const int key_length =
        char_to_number(value[0]);

    if (
        key_length < 10 ||
        key_length > 30 ||
        key_length + 1 >= 32
    ) {
        throw std::runtime_error(
            "Invalid server password value."
        );
    }

    const int password_length =
        char_to_number(
            value[key_length + 1]
        );

    if (
        password_length > 20 ||
        key_length + 2 + password_length > 32
    ) {
        throw std::runtime_error(
            "Invalid server password value."
        );
    }

    std::string result;

    for (
        int index = 0;
        index < password_length;
        ++index
    ) {
        const int plain =
            (
                static_cast<unsigned char>(
                    value[
                        key_length +
                        2 +
                        index
                    ]
                )
                + 0x3f
                -
                (
                    static_cast<unsigned char>(
                        value[
                            1 +
                            index % key_length
                        ]
                    )
                    - 0x20
                )
            ) % 0x5f;

        result +=
            static_cast<char>(
                plain + 0x20
            );
    }

    return result;
}

// ============================================================
// Self-contained MD5
// ============================================================

inline uint32_t left_rotate(
    uint32_t value,
    uint32_t count)
{
    return (value << count) |
           (value >> (32 - count));
}

std::string md5_upper(const std::string& input)
{
    static constexpr uint32_t shifts[64] = {
        7, 12, 17, 22,
        7, 12, 17, 22,
        7, 12, 17, 22,
        7, 12, 17, 22,

        5, 9, 14, 20,
        5, 9, 14, 20,
        5, 9, 14, 20,
        5, 9, 14, 20,

        4, 11, 16, 23,
        4, 11, 16, 23,
        4, 11, 16, 23,
        4, 11, 16, 23,

        6, 10, 15, 21,
        6, 10, 15, 21,
        6, 10, 15, 21,
        6, 10, 15, 21
    };

    static constexpr uint32_t constants[64] = {
        0xd76aa478, 0xe8c7b756,
        0x242070db, 0xc1bdceee,
        0xf57c0faf, 0x4787c62a,
        0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af,
        0xffff5bb1, 0x895cd7be,
        0x6b901122, 0xfd987193,
        0xa679438e, 0x49b40821,

        0xf61e2562, 0xc040b340,
        0x265e5a51, 0xe9b6c7aa,
        0xd62f105d, 0x02441453,
        0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6,
        0xf4d50d87, 0x455a14ed,
        0xa9e3e905, 0xfcefa3f8,
        0x676f02d9, 0x8d2a4c8a,

        0xfffa3942, 0x8771f681,
        0x6d9d6122, 0xfde5380c,
        0xa4beea44, 0x4bdecfa9,
        0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa,
        0xd4ef3085, 0x04881d05,
        0xd9d4d039, 0xe6db99e5,
        0x1fa27cf8, 0xc4ac5665,

        0xf4292244, 0x432aff97,
        0xab9423a7, 0xfc93a039,
        0x655b59c3, 0x8f0ccc92,
        0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0,
        0xa3014314, 0x4e0811a1,
        0xf7537e82, 0xbd3af235,
        0x2ad7d2bb, 0xeb86d391
    };

    std::vector<uint8_t> message(
        input.begin(),
        input.end()
    );

    const uint64_t bit_length =
        static_cast<uint64_t>(
            message.size()
        ) * 8ULL;

    message.push_back(0x80);

    while (
        (message.size() % 64) != 56
    ) {
        message.push_back(0);
    }

    for (int i = 0; i < 8; ++i) {
        message.push_back(
            static_cast<uint8_t>(
                (bit_length >> (8 * i)) &
                0xff
            )
        );
    }

    uint32_t a0 = 0x67452301;
    uint32_t b0 = 0xefcdab89;
    uint32_t c0 = 0x98badcfe;
    uint32_t d0 = 0x10325476;

    for (
        std::size_t offset = 0;
        offset < message.size();
        offset += 64
    ) {
        uint32_t words[16]{};

        for (int i = 0; i < 16; ++i) {
            const std::size_t p =
                offset +
                static_cast<std::size_t>(i) * 4;

            words[i] =
                static_cast<uint32_t>(
                    message[p]
                )
                |
                (
                    static_cast<uint32_t>(
                        message[p + 1]
                    ) << 8
                )
                |
                (
                    static_cast<uint32_t>(
                        message[p + 2]
                    ) << 16
                )
                |
                (
                    static_cast<uint32_t>(
                        message[p + 3]
                    ) << 24
                );
        }

        uint32_t a = a0;
        uint32_t b = b0;
        uint32_t c = c0;
        uint32_t d = d0;

        for (
            uint32_t i = 0;
            i < 64;
            ++i
        ) {
            uint32_t f = 0;
            uint32_t g = 0;

            if (i < 16) {
                f = (b & c) |
                    ((~b) & d);

                g = i;
            }
            else if (i < 32) {
                f = (d & b) |
                    ((~d) & c);

                g = (5 * i + 1) % 16;
            }
            else if (i < 48) {
                f = b ^ c ^ d;

                g = (3 * i + 5) % 16;
            }
            else {
                f = c ^ (
                    b | (~d)
                );

                g = (7 * i) % 16;
            }

            const uint32_t temp = d;

            d = c;
            c = b;

            b =
                b +
                left_rotate(
                    a +
                    f +
                    constants[i] +
                    words[g],
                    shifts[i]
                );

            a = temp;
        }

        a0 += a;
        b0 += b;
        c0 += c;
        d0 += d;
    }

    const uint32_t digest[4] = {
        a0,
        b0,
        c0,
        d0
    };

    static constexpr char hex[] =
        "0123456789ABCDEF";

    std::string result;
    result.reserve(32);

    for (uint32_t word : digest) {
        for (int i = 0; i < 4; ++i) {
            const uint8_t byte =
                static_cast<uint8_t>(
                    (word >> (i * 8)) &
                    0xff
                );

            result.push_back(
                hex[
                    (byte >> 4) & 0x0f
                ]
            );

            result.push_back(
                hex[
                    byte & 0x0f
                ]
            );
        }
    }

    return result;
}

// ============================================================
// UI
// ============================================================

std::string read_text(HWND control)
{
    const int length =
        GetWindowTextLengthA(control);

    std::string result(
        static_cast<std::size_t>(
            length
        ) + 1,
        '\0'
    );

    GetWindowTextA(
        control,
        result.data(),
        length + 1
    );

    result.resize(
        static_cast<std::size_t>(
            length
        )
    );

    return result;
}

void run_action(bool should_encode)
{
    try {
        const std::string input =
            read_text(input_box);

        std::string output;
        std::string plain;

        if (should_encode) {
            // Input = plaintext
            plain = input;
            output = encode(input);
        }
        else {
            // Input = legacy protected value
            plain = decode(input);
            output = plain;
        }

        SetWindowTextA(
            output_box,
            output.c_str()
        );

        const std::string md5 =
            md5_upper(plain);

        SetWindowTextA(
            md5_box,
            md5.c_str()
        );
    }
    catch (
        const std::exception& error
    ) {
        SetWindowTextA(
            output_box,
            ""
        );

        SetWindowTextA(
            md5_box,
            ""
        );

        MessageBoxA(
            nullptr,
            error.what(),
            "Password Tool",
            MB_OK |
            MB_ICONWARNING
        );
    }
}

LRESULT CALLBACK window_proc(
    HWND window,
    UINT message,
    WPARAM wparam,
    LPARAM lparam)
{
    switch (message) {

    case WM_COMMAND:

        if (
            LOWORD(wparam) ==
            kEncode
        ) {
            run_action(true);
            return 0;
        }

        if (
            LOWORD(wparam) ==
            kDecode
        ) {
            run_action(false);
            return 0;
        }

        return 0;

    case WM_DESTROY:

        PostQuitMessage(0);
        return 0;

    default:

        return DefWindowProcA(
            window,
            message,
            wparam,
            lparam
        );
    }
}

HWND add_control(
    const char* type,
    const char* text,
    DWORD style,
    int x,
    int y,
    int width,
    int height,
    int id,
    HWND parent)
{
    return CreateWindowExA(
        0,
        type,
        text,
        WS_CHILD |
        WS_VISIBLE |
        style,
        x,
        y,
        width,
        height,
        parent,
        reinterpret_cast<HMENU>(
            static_cast<INT_PTR>(id)
        ),
        GetModuleHandleA(nullptr),
        nullptr
    );
}

} // namespace

int WINAPI WinMain(
    HINSTANCE instance,
    HINSTANCE,
    LPSTR,
    int show)
{
    const char* class_name =
        "BishopPasswordTool";

    WNDCLASSA window_class{};

    window_class.hInstance =
        instance;

    window_class.lpszClassName =
        class_name;

    window_class.lpfnWndProc =
        window_proc;

    window_class.hCursor =
        LoadCursor(
            nullptr,
            IDC_ARROW
        );

    window_class.hIcon =
        LoadIconA(
            instance,
            MAKEINTRESOURCEA(kAppIcon)
        );

    window_class.hbrBackground =
        reinterpret_cast<HBRUSH>(
            COLOR_BTNFACE + 1
        );

    if (!RegisterClassA(&window_class)) {
        return 1;
    }

    // Slightly taller than original
    // because MD5 has been added.
    HWND window =
        CreateWindowExA(
            0,
            class_name,
            "GM PassTool",
            WS_OVERLAPPED |
            WS_CAPTION |
            WS_SYSMENU,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            255,
            225,
            nullptr,
            nullptr,
            instance,
            nullptr
        );

    if (!window) {
        return 1;
    }

    HFONT font =
        static_cast<HFONT>(
            GetStockObject(
                DEFAULT_GUI_FONT
            )
        );

    // ---------------- INPUT ----------------

    HWND label1 =
        add_control(
            "STATIC",
            "Input",
            0,
            10, 8,
            45, 17,
            0,
            window
        );

    input_box =
        add_control(
            "EDIT",
            "",
            WS_BORDER |
            ES_AUTOHSCROLL,
            10, 25,
            220, 22,
            kInput,
            window
        );

    // ---------------- BUTTONS ----------------

    HWND encode_button =
        add_control(
            "BUTTON",
            "Encrypt",
            BS_PUSHBUTTON,
            56, 55,
            65, 24,
            kEncode,
            window
        );

    HWND decode_button =
        add_control(
            "BUTTON",
            "Decrypt",
            BS_PUSHBUTTON,
            128, 55,
            65, 24,
            kDecode,
            window
        );

    // ---------------- OUTPUT ----------------

    HWND label2 =
        add_control(
            "STATIC",
            "Output",
            0,
            10, 87,
            50, 17,
            0,
            window
        );

    output_box =
        add_control(
            "EDIT",
            "",
            WS_BORDER |
            ES_AUTOHSCROLL |
            ES_READONLY,
            10, 104,
            220, 22,
            kOutput,
            window
        );

    // ---------------- MD5 ----------------

    HWND label3 =
        add_control(
            "STATIC",
            "MD5",
            0,
            10, 135,
            45, 17,
            0,
            window
        );

    md5_box =
        add_control(
            "EDIT",
            "",
            WS_BORDER |
            ES_AUTOHSCROLL |
            ES_READONLY,
            10, 152,
            220, 22,
            kMd5,
            window
        );

    // Apply Windows default UI font.
    for (HWND control : {
        label1,
        input_box,
        encode_button,
        decode_button,
        label2,
        output_box,
        label3,
        md5_box
    }) {
        SendMessageA(
            control,
            WM_SETFONT,
            reinterpret_cast<WPARAM>(
                font
            ),
            TRUE
        );
    }

    ShowWindow(
        window,
        show
    );

    UpdateWindow(
        window
    );

    MSG message{};

    while (
        GetMessageA(
            &message,
            nullptr,
            0,
            0
        ) > 0
    ) {
        TranslateMessage(
            &message
        );

        DispatchMessageA(
            &message
        );
    }

    return 0;
}