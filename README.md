# GM PassTool VLTK Offline

Công cụ mã hóa và giải mã password dùng trong VLTK Server Offline. Giao diện được viết bằng C++ MFC và chỉnh sửa trực tiếp bằng Resource View của Visual Studio.

## Tính năng

* Mã hóa password thành chuỗi 32 ký tự.
* Giải mã chuỗi 32 ký tự về password gốc.
* Tạo MD5 chữ hoa của password gốc.
* Output và MD5 ở chế độ chỉ đọc.
* Click vào Output hoặc MD5 để chọn và tự động copy.
* Kiểm tra dữ liệu đầu vào và thông báo khi sai định dạng.
* Tương thích với chuỗi được tạo bởi bản Win32 cũ.

## Cách dùng

### Mã hóa

1. Nhập password vào **Input**.
2. Nhấn **Encrypt**.
3. **Output** trả về chuỗi mã hóa 32 ký tự.
4. **MD5** trả về MD5 chữ hoa của password gốc.

### Giải mã

1. Nhập chuỗi mã hóa 32 ký tự vào **Input**.
2. Nhấn **Decrypt**.
3. **Output** trả về password gốc.
4. **MD5** trả về MD5 chữ hoa của password đó.

Click vào ô **Output** hoặc **MD5** để tự động copy kết quả.

## Phạm vi áp dụng

| Thành phần | File cấu hình                 | Trường sử dụng                                                         |
| ---------- | ----------------------------- | ---------------------------------------------------------------------- |
| Bishop     | `gw/bishop.cfg`               | `[Setting] Password`                                                   |
| Goddess    | `gw/goddess.cfg`              | `[Database] Password`                                                  |
| PaySys     | `Pays/database.ini`           | `Server`, `DataBase`, `User`, `PassWord` trong `[card]` và `[account]` |
| S3Relay    | `gw/s3relay/relay_config.ini` | `[Database] Password`, `[Setting] Password` nếu cùng định dạng         |
| GMPassGen  | `volam/GMPassGen.exe`         | Chuỗi password do chương trình sinh ra                                 |

> Không dùng tool cho `[Setting] Password` dạng hex trong `gw/goddess.cfg`, ví dụ chuỗi bắt đầu bằng `2DED...`. Đây là cơ chế khác.

## Lưu ý

* Password gốc dài tối đa 20 ký tự ASCII.
* Chuỗi mã hóa luôn dài 32 ký tự.
* Cùng một password có thể tạo ra các chuỗi mã hóa khác nhau. Các chuỗi này vẫn giải mã về cùng một kết quả.
* Đây chỉ là cơ chế làm rối có thể giải ngược, không phải mã hóa bảo mật hiện đại.
* Sau khi sửa file cấu hình, cần khởi động lại dịch vụ liên quan và kiểm tra log kết nối.
* Nếu đổi password MySQL, hãy đổi password trên MySQL trước rồi cập nhật tất cả dịch vụ đang dùng chung tài khoản.

## Build

Mở `gmpasstool.sln` bằng Visual Studio và chọn:

```text
Release | x64
```

Sau đó chọn **Build Solution**.

File chạy được tạo trong thư mục:

```text
x64\Release
```

## Yêu cầu trên máy client

Bản `Release | x64` sử dụng MFC Dynamic và toolset v143. Máy client cần cài:

[Microsoft Visual C++ Redistributable 2015–2022 (x64)](https://aka.ms/vc14/vc_redist.x64.exe)

Nếu build bản `Release | Win32`, cài bản x86:

[Microsoft Visual C++ Redistributable 2015–2022 (x86)](https://aka.ms/vc14/vc_redist.x86.exe)
