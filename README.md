# Tool mã hóa/giải mã Password VLTK Server Offline

## Bản app Windows C++ `.exe` (không cần Visual Studio)

File: `password_tool_ui.cpp`. Đây là app cửa sổ Windows thật (WinAPI),
có một ô **Input**, một ô **Output**, cùng hai nút **Encode** và **Decode**.
Không dùng Qt, .NET hay Visual Studio. Icon của app lấy từ `tools/favicon.ico`.

Trên Windows đã cài MinGW-w64 có `g++` và `windres`, mở PowerShell tại thư mục
gốc project rồi chạy:

```powershell
.\build_password_tool_ui.ps1
.\password_tool_ui.exe
```

Nếu PowerShell chặn file script ở lần đầu, chạy lệnh này chỉ cho cửa sổ PowerShell
hiện tại rồi build lại:

```powershell
Set-ExecutionPolicy -Scope Process Bypass
```

Hoặc chạy file `password_tool_ui.exe` bằng cách nhấp đúp. Script build
cũng tự nhúng icon `favicon.ico` vào file `.exe`.

```bash
g++ -std=c++17 -O2 -mwindows -o password_tool_ui.exe password_tool_ui.cpp -luser32 -lgdi32
```

## Phạm vi áp dụng

Tool tương thích với format chuỗi 32 ký tự của các trường sau:

| Thành phần | File | Trường dùng tool |
| --- | --- | --- |
| Bishop | `gw/bishop.cfg` | `[Setting] Password` |
| Goddess | `gw/goddess.cfg` | `[Database] Password` |
| PaySys | `Pays/database.ini` | `Server`, `DataBase`, `User`, `PassWord` trong `[card]` và `[account]` |
| S3Relay | `gw/s3relay/relay_config.ini` | `[Database] Password`, `[Setting] Password` nếu cùng format |
| GMPassGen | `volam/GMPassGen.exe` | Chuỗi password do tool sinh ra |

Không áp dụng tool này cho `[Setting] Password` dạng hex trong `gw/goddess.cfg` (ví dụ chuỗi bắt đầu `2DED...`); đó là cơ chế khác.

Đã đối chiếu thực tế: chuỗi do `GMPassGen.exe` tạo `r60D0z0BEfZI66YGkBF_MS0FoU_mPOIV` được tool giải mã thành `1234560123`; vì vậy GMPassGen dùng cùng format này.

## Cơ chế

Đây là **obfuscation có thể đảo ngược**, không phải hash hoặc mã hóa bảo mật hiện đại.

Binary Bishop/Goddess chứa hàm `SimplyDecryptPassword`, sử dụng các bước:

1. Chuỗi cấu hình luôn có 32 ký tự.
2. Hoán đổi cố định một số cặp ký tự.
3. Lấy độ dài key và độ dài plaintext từ hai ký tự đã mã hóa.
4. Dùng key lặp vòng để khôi phục plaintext trong dải ký tự ASCII in được.

Vì thuật toán và key đều nằm trong chuỗi/binary, người có quyền đọc file cấu hình và binary có thể giải mã. Không dùng cơ chế này để bảo vệ bí mật quan trọng.

## Cách dùng

Chạy tại thư mục gốc project:

```bash
# Mã hóa plaintext thành chuỗi để điền sau dấu = trong file .ini/.cfg
python3 tools/encode_server_password.py encode 'PassMoi123!'

# Giải mã một chuỗi đang có trong file cấu hình
python3 tools/encode_server_password.py decode 'CHUOI_32_KY_TU'

# Cách an toàn trong Git Bash khi chuỗi có dấu `, $, ;, ...:
python3 tools/encode_server_password.py decode
# Tool sẽ hỏi, sau đó dán nguyên chuỗi và nhấn Enter.
```

Lệnh `encode` in ra một chuỗi 32 ký tự. Sao chép nguyên chuỗi đó vào sau dấu `=`; không thêm dấu ngoặc kép hoặc khoảng trắng thừa.

Chuỗi mới chỉ dùng chữ, số và các ký tự `_`, `[`, `]`, `` ` ``, `!`, `$` (trường hợp mật khẩu dài đúng 20 ký tự có thể có thêm `>`). Các chuỗi cũ vẫn giải mã bình thường.

Khi đưa chuỗi vào lệnh Git Bash, hãy đặt nó trong dấu nháy đơn `'...'`. Nếu chuỗi có ký tự đặc biệt gây khó dán, dùng chế độ nhập tương tác `decode` không kèm tham số.

Ví dụ đổi IP database PaySys:

```bash
python3 tools/encode_server_password.py encode '192.168.1.201'
```

Thay output vào `Server=` ở cả mục `[card]` và `[account]` trong `Pays/database.ini` nếu hai mục cùng trỏ một server.

## Giới hạn và kiểm tra

- Plaintext dài từ 0 đến 20 ký tự ASCII in được.
- Mỗi lần `encode` cùng một plaintext có thể cho chuỗi khác nhau vì key/padding được tạo ngẫu nhiên. Đây là bình thường; tất cả các chuỗi đó đều giải mã về cùng plaintext.
- Sau khi sửa cấu hình, khởi động lại đúng dịch vụ liên quan và kiểm tra log kết nối.
- Khi đổi password MySQL, cần đổi password thật trên MySQL trước, rồi cập nhật mọi dịch vụ đang dùng cùng tài khoản.

## Khuyến nghị an toàn

- Không commit file cấu hình có thông tin đăng nhập lên Git hoặc chia sẻ công khai.
- Giới hạn quyền đọc thư mục server.
- Đổi các thông tin đăng nhập mặc định/cũ, vì cơ chế che chuỗi này không đủ an toàn.
