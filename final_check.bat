@echo off
chcp 936 >nul
echo ===============================
echo        最终检查脚本
echo ===============================
echo.

echo 1. 检查大文件（大于10MB）：
for /f "delims=" %%f in ('dir /s /b /a-d *.zip *.rar *.7z 2^>nul') do (
    for /f "tokens=3" %%s in ('dir "%%f" ^| find "个文件"') do (
        set "size=%%s"
        setlocal enabledelayedexpansion
        if !size! GTR 10240 (
            echo    发现大文件: %%f (!size! KB)
        )
        endlocal
    )
)

echo.
echo 2. 检查是否包含编译文件：
if exist "x64" echo   [警告] x64目录存在（建议忽略）
if exist ".vs" echo   [警告] .vs目录存在（建议忽略）
if exist "Debug" echo   [警告] Debug目录存在（建议忽略）
if exist "Release" echo   [警告] Release目录存在（建议忽略）
dir *.exe 2>nul >nul && echo   [警告] 发现.exe文件（建议忽略）

echo.
echo 3. 检查Git状态：
git status --short

echo.
set /p proceed=检查完成，是否继续推送？(y/N): 
if /i "%proceed%"=="y" (
    call push_now.bat
) else (
    echo 取消推送
    pause
)