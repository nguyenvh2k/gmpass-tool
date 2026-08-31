
// gmpasstoolDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "gmpasstool.h"
#include "gmpasstoolDlg.h"
#include "afxdialogex.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
const UINT kSelectAllTextMessage = WM_APP + 1;

struct SwapPosition
{
	int left;
	int right;
};

const SwapPosition kSwaps[] = {
	{ 0, 13 }, { 31, 25 }, { 12, 30 }, { 7, 19 },
	{ 3, 21 }, { 9, 20 }, { 15, 18 },
};

const std::string kOutputAlphabet =
	"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_[]>";

int CharToNumber(char character)
{
	const unsigned value = static_cast<unsigned char>(character) ^ 0x97U;
	return static_cast<int>(((value << 7U) | (value >> 1U)) & 0x1FU);
}

char NumberToChar(int number)
{
	for (std::string::const_iterator it = kOutputAlphabet.begin();
		it != kOutputAlphabet.end(); ++it)
		if (CharToNumber(*it) == number)
			return *it;
	throw std::runtime_error("Cannot encode number.");
}

void SwapCharacters(std::string& value)
{
	for (int index = 0; index < 7; ++index)
		std::swap(value[kSwaps[index].left], value[kSwaps[index].right]);
}

bool IsPrintableAscii(const std::string& text)
{
	for (std::string::const_iterator it = text.begin(); it != text.end(); ++it) {
		const unsigned char character = static_cast<unsigned char>(*it);
		if (character < 0x20 || character > 0x7e)
			return false;
	}
	return true;
}

std::mt19937& Engine()
{
	static std::mt19937 value(static_cast<unsigned>(
		std::chrono::high_resolution_clock::now().time_since_epoch().count()));
	return value;
}

char RandomOutputCharacter()
{
	return kOutputAlphabet[std::uniform_int_distribution<std::size_t>(
		0, kOutputAlphabet.size() - 1)(Engine())];
}

std::string Encode(const std::string& password)
{
	if (password.size() > 20 || !IsPrintableAscii(password))
		throw std::runtime_error("Password must be 0..20 printable ASCII characters.");

	const int keyLengthLimit = 30 - static_cast<int>(password.size());
	const int maxKeyLength = keyLengthLimit < 19 ? keyLengthLimit : 19;
	if (maxKeyLength < 10)
		throw std::runtime_error("Password is too long.");

	const int keyLength = std::uniform_int_distribution<int>(10, maxKeyLength)(Engine());
	std::string key;
	for (int keyIndex = 0; keyIndex < keyLength; ++keyIndex) {
		std::string candidates;
		for (std::string::const_iterator alphabetIt = kOutputAlphabet.begin();
			alphabetIt != kOutputAlphabet.end(); ++alphabetIt) {
			const char keyChar = *alphabetIt;
			bool valid = true;
			for (std::size_t index = static_cast<std::size_t>(keyIndex);
				index < password.size(); index += keyLength) {
				const char cipher = static_cast<char>(
					(static_cast<unsigned char>(password[index]) - 0x20 - 0x3f +
						static_cast<unsigned char>(keyChar) - 0x20 + 0x5f) % 0x5f);
				if (kOutputAlphabet.find(cipher) == std::string::npos) {
					valid = false;
					break;
				}
			}
			if (valid)
				candidates += keyChar;
		}
		if (candidates.empty())
			throw std::runtime_error("Password cannot be encoded with the restricted output alphabet.");
		key += candidates[std::uniform_int_distribution<std::size_t>(
			0, candidates.size() - 1)(Engine())];
	}

	std::string ciphertext;
	for (std::size_t index = 0; index < password.size(); ++index) {
		ciphertext += static_cast<char>(
			(static_cast<unsigned char>(password[index]) - 0x20 - 0x3f +
				static_cast<unsigned char>(key[index % key.size()]) - 0x20 + 0x5f) % 0x5f);
	}

	std::string result(1, NumberToChar(keyLength));
	result += key;
	result += NumberToChar(static_cast<int>(password.size()));
	result += ciphertext;
	while (result.size() < 32)
		result += RandomOutputCharacter();
	SwapCharacters(result);
	return result;
}

std::string Decode(std::string value)
{
	if (value.size() != 32)
		throw std::runtime_error("Encoded value must have exactly 32 characters.");
	SwapCharacters(value);

	const int keyLength = CharToNumber(value[0]);
	if (keyLength < 10 || keyLength > 30 || keyLength + 1 >= 32)
		throw std::runtime_error("Invalid server password value.");

	const int passwordLength = CharToNumber(value[keyLength + 1]);
	if (passwordLength > 20 || keyLength + 2 + passwordLength > 32)
		throw std::runtime_error("Invalid server password value.");

	std::string result;
	for (int index = 0; index < passwordLength; ++index) {
		const int plain = (static_cast<unsigned char>(value[keyLength + 2 + index]) + 0x3f -
			(static_cast<unsigned char>(value[1 + index % keyLength]) - 0x20)) % 0x5f;
		result += static_cast<char>(plain + 0x20);
	}
	return result;
}

inline uint32_t LeftRotate(uint32_t value, uint32_t count)
{
	return (value << count) | (value >> (32 - count));
}

std::string Md5Upper(const std::string& input)
{
	static const uint32_t shifts[64] = {
		7,12,17,22, 7,12,17,22, 7,12,17,22, 7,12,17,22,
		5,9,14,20, 5,9,14,20, 5,9,14,20, 5,9,14,20,
		4,11,16,23, 4,11,16,23, 4,11,16,23, 4,11,16,23,
		6,10,15,21, 6,10,15,21, 6,10,15,21, 6,10,15,21 };
	static const uint32_t constants[64] = {
		0xd76aa478,0xe8c7b756,0x242070db,0xc1bdceee,0xf57c0faf,0x4787c62a,0xa8304613,0xfd469501,0x698098d8,0x8b44f7af,0xffff5bb1,0x895cd7be,0x6b901122,0xfd987193,0xa679438e,0x49b40821,
		0xf61e2562,0xc040b340,0x265e5a51,0xe9b6c7aa,0xd62f105d,0x02441453,0xd8a1e681,0xe7d3fbc8,0x21e1cde6,0xc33707d6,0xf4d50d87,0x455a14ed,0xa9e3e905,0xfcefa3f8,0x676f02d9,0x8d2a4c8a,
		0xfffa3942,0x8771f681,0x6d9d6122,0xfde5380c,0xa4beea44,0x4bdecfa9,0xf6bb4b60,0xbebfbc70,0x289b7ec6,0xeaa127fa,0xd4ef3085,0x04881d05,0xd9d4d039,0xe6db99e5,0x1fa27cf8,0xc4ac5665,
		0xf4292244,0x432aff97,0xab9423a7,0xfc93a039,0x655b59c3,0x8f0ccc92,0xffeff47d,0x85845dd1,0x6fa87e4f,0xfe2ce6e0,0xa3014314,0x4e0811a1,0xf7537e82,0xbd3af235,0x2ad7d2bb,0xeb86d391 };

	std::vector<uint8_t> message(input.begin(), input.end());
	const uint64_t bitLength = static_cast<uint64_t>(message.size()) * 8ULL;
	message.push_back(0x80);
	while (message.size() % 64 != 56)
		message.push_back(0);
	for (int i = 0; i < 8; ++i)
		message.push_back(static_cast<uint8_t>((bitLength >> (8 * i)) & 0xff));

	uint32_t a0 = 0x67452301, b0 = 0xefcdab89, c0 = 0x98badcfe, d0 = 0x10325476;
	for (std::size_t offset = 0; offset < message.size(); offset += 64) {
		uint32_t words[16]{};
		for (int i = 0; i < 16; ++i) {
			const std::size_t p = offset + static_cast<std::size_t>(i) * 4;
			words[i] = static_cast<uint32_t>(message[p]) |
				(static_cast<uint32_t>(message[p + 1]) << 8) |
				(static_cast<uint32_t>(message[p + 2]) << 16) |
				(static_cast<uint32_t>(message[p + 3]) << 24);
		}
		uint32_t a = a0, b = b0, c = c0, d = d0;
		for (uint32_t i = 0; i < 64; ++i) {
			uint32_t f, g;
			if (i < 16) { f = (b & c) | ((~b) & d); g = i; }
			else if (i < 32) { f = (d & b) | ((~d) & c); g = (5 * i + 1) % 16; }
			else if (i < 48) { f = b ^ c ^ d; g = (3 * i + 5) % 16; }
			else { f = c ^ (b | (~d)); g = (7 * i) % 16; }
			const uint32_t temp = d;
			d = c; c = b;
			b = b + LeftRotate(a + f + constants[i] + words[g], shifts[i]);
			a = temp;
		}
		a0 += a; b0 += b; c0 += c; d0 += d;
	}

	static const char hex[] = "0123456789ABCDEF";
	std::string result;
	result.reserve(32);
	const uint32_t digest[4] = { a0, b0, c0, d0 };
	for (int wordIndex = 0; wordIndex < 4; ++wordIndex) {
		const uint32_t word = digest[wordIndex];
		for (int i = 0; i < 4; ++i) {
			const uint8_t byte = static_cast<uint8_t>((word >> (i * 8)) & 0xff);
			result.push_back(hex[(byte >> 4) & 0x0f]);
			result.push_back(hex[byte & 0x0f]);
		}
	}
	return result;
}
}


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CgmpasstoolDlg dialog



CgmpasstoolDlg::CgmpasstoolDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GMPASSTOOL_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CgmpasstoolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CgmpasstoolDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CgmpasstoolDlg::OnBnClickedEncrypt)
	ON_BN_CLICKED(IDC_BUTTON2, &CgmpasstoolDlg::OnBnClickedDecrypt)
	ON_BN_CLICKED(IDC_BUTTON3, &CgmpasstoolDlg::OnBnClickedClear)
	ON_EN_SETFOCUS(IDC_EDIT3, &CgmpasstoolDlg::OnEnSetfocusOutput)
	ON_EN_SETFOCUS(IDC_EDIT2, &CgmpasstoolDlg::OnEnSetfocusMd5)
	ON_MESSAGE(kSelectAllTextMessage, &CgmpasstoolDlg::OnSelectAllText)
END_MESSAGE_MAP()


// CgmpasstoolDlg message handlers

BOOL CgmpasstoolDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	GetDlgItem(IDC_EDIT2)->ModifyStyle(0, ES_READONLY);
	GetDlgItem(IDC_EDIT3)->ModifyStyle(0, ES_READONLY);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CgmpasstoolDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CgmpasstoolDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CgmpasstoolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CgmpasstoolDlg::OnBnClickedEncrypt()
{
	RunPasswordAction(true);
}

void CgmpasstoolDlg::OnBnClickedDecrypt()
{
	RunPasswordAction(false);
}

void CgmpasstoolDlg::OnBnClickedClear()
{
	SetDlgItemText(IDC_EDIT1, _T(""));
	SetDlgItemText(IDC_EDIT2, _T(""));
	SetDlgItemText(IDC_EDIT3, _T(""));
	GetDlgItem(IDC_EDIT1)->SetFocus();
}

void CgmpasstoolDlg::OnEnSetfocusOutput()
{
	PostMessage(kSelectAllTextMessage, IDC_EDIT3, 0);
}

void CgmpasstoolDlg::OnEnSetfocusMd5()
{
	PostMessage(kSelectAllTextMessage, IDC_EDIT2, 0);
}

LRESULT CgmpasstoolDlg::OnSelectAllText(WPARAM wParam, LPARAM)
{
	CWnd* edit = GetDlgItem(static_cast<int>(wParam));
	if (edit != NULL) {
		edit->SendMessage(EM_SETSEL, 0, -1);
		edit->SendMessage(WM_COPY);
	}
	return 0;
}

void CgmpasstoolDlg::RunPasswordAction(bool encode)
{
	try {
		CString inputText;
		GetDlgItemText(IDC_EDIT1, inputText);
		const CStringA ansiInput(inputText);
		const std::string input(ansiInput.GetString());

		const std::string plain = encode ? input : Decode(input);
		const std::string output = encode ? Encode(input) : plain;

		SetDlgItemText(IDC_EDIT3, CString(output.c_str()));
		SetDlgItemText(IDC_EDIT2, CString(Md5Upper(plain).c_str()));
	}
	catch (const std::exception& error) {
		SetDlgItemText(IDC_EDIT2, _T(""));
		SetDlgItemText(IDC_EDIT3, _T(""));
		AfxMessageBox(CString(error.what()), MB_OK | MB_ICONWARNING);
	}
}

