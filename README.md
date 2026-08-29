# Tool Mã Hóa & Giải Mã Mật Khẩu VLTK Server Offline

## Giao diện Windows C++ (`password_tool_ui.exe`)

Công cụ chạy dạng cửa sổ WinAPI thuần, không phụ thuộc vào Visual Studio, .NET hay Qt framework. Giao diện tích hợp sẵn ô **Input**, **Output** cùng hai nút bấm **Encode** và **Decode**. Icon của app sử dụng trực tiếp từ `tools/favicon.ico`.

### Cách build từ PowerShell

Mở PowerShell tại thư mục gốc của project và thực hiện các bước sau:

```powershell
# Mở quyền chạy script nếu bị chặn ở lần đầu
Set-ExecutionPolicy -Scope Process Bypass

# Build ứng dụng
.\build_password_tool_ui.ps1

# Chạy ứng dụng
.\password_tool_ui.exe
```

*Lưu ý:* Có thể nhấp đúp trực tiếp vào file `password_tool_ui.exe` để sử dụng. Script build đã tự động nhúng icon `favicon.ico` vào file `.exe`.

Nếu muốn build thủ công bằng MinGW-w64 trong terminal:

```bash
g++ -std=c++17 -O2 -mwindows -o password_tool_ui.exe password_tool_ui.cpp -luser32 -lgdi32
```

---

## Phạm vi áp dụng

Tool tương thích với format chuỗi 32 ký tự của các trường sau:

| Thành phần | File | Trường dùng tool |
| --- | --- | --- |
| **Bishop** | `gw/bishop.cfg` | `[Setting] Password` |
| **Goddess** | `gw/goddess.cfg` | `[Database] Password` |
| **PaySys** | `Pays/database.ini` | `Server`, `DataBase`, `User`, `PassWord` trong `[card]` và `[account]` |
| **S3Relay** | `gw/s3relay/relay_config.ini` | `[Database] Password`, `[Setting] Password` nếu cùng format |
| **GMPassGen** | `volam/GMPassGen.exe` | Chuỗi password do tool sinh ra |

Không áp dụng tool này cho `[Setting] Password` dạng hex trong `gw/goddess.cfg` (ví dụ chuỗi bắt đầu `2DED...`); đó là cơ chế khác.

---

## Cơ chế

Đây là **obfuscation có thể đảo ngược**, không phải hash hoặc mã hóa bảo mật hiện đại.

Binary Bishop/Goddess chứa hàm `SimplyDecryptPassword`, sử dụng các bước:

1. Chuỗi cấu hình luôn có 32 ký tự.
2. Hoán đổi cố định một số cặp ký tự.
3. Lấy độ dài key và độ dài plaintext từ hai ký tự đã mã hóa.
4. Dùng key lặp vòng để khôi phục plaintext trong dải ký tự ASCII in được.

Vì thuật toán và key đều nằm trong chuỗi/binary, người có quyền đọc file cấu hình và binary có thể giải mã. Không dùng cơ chế này để bảo vệ bí mật quan trọng.

---

## Giới hạn và lưu ý sử dụng

* **Độ dài chuỗi gốc:** Plaintext dài từ 0 đến 20 ký tự ASCII in được.
* **Mã hóa ngẫu nhiên:** Mỗi lần `encode` cùng một plaintext có thể cho chuỗi khác nhau vì key/padding được tạo ngẫu nhiên. Đây là bình thường; tất cả các chuỗi đó đều giải mã về cùng plaintext.
* **Dải ký tự đầu ra:** Chuỗi mới chỉ dùng chữ, số và các ký tự `_`, `[`, `]`, `` ` ``, `!`, `$` (trường hợp mật khẩu dài đúng 20 ký tự có thể có thêm `>`). Các chuỗi cũ vẫn giải mã bình thường.
* **Cập nhật cấu hình:** Sau khi sửa cấu hình, khởi động lại đúng dịch vụ liên quan và kiểm tra log kết nối.
* **Quy trình đổi pass MySQL:** Khi đổi password MySQL, cần đổi password thật trên MySQL trước, rồi cập nhật mọi dịch vụ đang dùng cùng tài khoản.
