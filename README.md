# GM PassTool (MFC)

Ứng dụng mã hóa và giải mã password GM VLTK, chuyển từ bản giao diện Win32 thuần sang MFC.

## Cải tiến so với bản Win32 cũ

- Dùng giao diện **MFC Dialog** và resource của Visual Studio; dễ chỉnh sửa giao diện bằng Resource View.
- Gắn trực tiếp các nút **Encrypt** và **Decrypt** vào event handler MFC.
- Giữ nguyên thuật toán password codec của bản Win32, nên các chuỗi mã hóa dài 32 ký tự vẫn giải mã tương thích giữa hai bản.
- Vẫn tạo MD5 dạng chữ hoa cho password gốc sau khi Encrypt hoặc Decrypt.
- Ô **Output** và **MD5** là read-only để tránh sửa nhầm kết quả.
- Click vào Output hoặc MD5 sẽ chọn toàn bộ nội dung và tự copy vào clipboard.
- Kiểm tra dữ liệu đầu vào và báo lỗi rõ ràng khi password không hợp lệ hoặc chuỗi mã hóa không đúng định dạng.
- Build Release x64 để chạy tốt trên Windows 64-bit hiện nay.

## Cách dùng

1. Nhập password gốc vào ô **Input** rồi chọn **Encrypt**.
   - Output là chuỗi password đã mã hóa gồm 32 ký tự.
   - MD5 là mã MD5 chữ hoa của password gốc.
2. Hoặc nhập chuỗi mã hóa 32 ký tự vào **Input** rồi chọn **Decrypt**.
   - Output là password đã giải mã.
   - MD5 là mã MD5 chữ hoa của password vừa giải mã.
3. Click vào Output hoặc MD5 để tự copy giá trị đó.

## Build

Mở `gmpasstool.sln` bằng Visual Studio, chọn cấu hình:

```text
Release | x64
```

Sau đó chọn **Build Solution**. File thực thi nằm trong thư mục `x64\Release` của project.

## Yêu cầu khi chạy trên máy client

Bản `Release | x64` dùng MFC Dynamic và toolset v143. Máy client cần cài **Microsoft Visual C++ Redistributable for Visual Studio 2015–2022 (x64)**:

https://aka.ms/vc14/vc_redist.x64.exe

Nếu build bản `Release | Win32`, máy client cần runtime x86 tương ứng:

https://aka.ms/vc14/vc_redist.x86.exe

