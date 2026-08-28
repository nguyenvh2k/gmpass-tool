$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot

& windres password_tool_ui.rc -O coff -o password_tool_ui.res
if ($LASTEXITCODE -ne 0) { throw 'Failed to create icon resource.' }

& g++ -std=c++17 -O2 -mwindows -static -static-libgcc -static-libstdc++ -Wall -Wextra -pedantic `
    password_tool_ui.cpp password_tool_ui.res -o password_tool_ui.exe -luser32 -lgdi32
if ($LASTEXITCODE -ne 0) { throw 'Build password_tool_ui.exe failed.' }

Remove-Item password_tool_ui.res -ErrorAction SilentlyContinue
Write-Host 'Built: tools/password_tool_ui.exe'
