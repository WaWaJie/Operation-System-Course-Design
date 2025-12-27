@echo off
chcp 65001 >nul
echo ========================================
echo        GitHub 完整推送脚本
echo ========================================
echo.

REM 设置颜色
color 0A

:main_menu
echo 请选择操作:
echo.
echo  [1] 快速推送（添加、提交、推送）
echo  [2] 安全检查并推送（推荐）
echo  [3] 强制推送（解决冲突后）
echo  [4] 检查并修复问题
echo  [5] 查看Git状态
echo  [6] 查看远程仓库信息
echo  [7] 退出
echo.

set /p choice=请输入选择 (1-7): 

if "%choice%"=="1" goto quick_push
if "%choice%"=="2" goto safe_push
if "%choice%"=="3" goto force_push
if "%choice%"=="4" goto check_fix
if "%choice%"=="5" goto git_status
if "%choice%"=="6" goto remote_info
if "%choice%"=="7" goto exit_script
echo 无效选择，请重新输入
goto main_menu

:quick_push
echo.
echo ⚡ 快速推送模式
echo.
git add .
set /p commit_msg=请输入提交信息: 
git commit -m "%commit_msg%"
git push origin master
goto push_complete

:safe_push
echo.
echo 🔒 安全检查模式
echo.
call :check_git
if errorlevel 1 goto main_menu

echo.
echo 📝 查看将要提交的文件:
git status --porcelain

echo.
set /p confirm=确认要提交这些文件吗？(y/N): 
if /i not "%confirm%"=="y" (
    echo 取消提交
    goto main_menu
)

echo.
set /p commit_msg=请输入提交信息: 
if "%commit_msg%"=="" set commit_msg="自动提交 [%date% %time%]"

echo.
echo 🔄 正在提交...
git add .
git commit -m "%commit_msg%"

echo.
echo 🚀 正在推送到GitHub...
git push origin master
goto push_complete

:force_push
echo.
echo ⚠️  强制推送模式（谨慎使用）
echo 注意：这会覆盖远程仓库，请确保你知道在做什么！
echo.
set /p confirm=确定要强制推送吗？这可能会影响其他协作者。(y/N): 
if /i not "%confirm%"=="y" (
    echo 取消强制推送
    goto main_menu
)

echo.
echo 🔄 正在强制推送...
git push -f origin master
goto push_complete

:check_fix
echo.
echo 🔍 检查并修复问题
echo.
call :check_big_files
call :check_binary_files
call :clean_unnecessary

echo.
set /p fix=是否要自动修复上述问题？(y/N): 
if /i "%fix%"=="y" (
    call :auto_fix
)

goto main_menu

:git_status
echo.
echo 📊 Git状态:
echo.
git status
echo.
pause
goto main_menu

:remote_info
echo.
echo 🌐 远程仓库信息:
echo.
git remote -v
git branch -vv
echo.
pause
goto main_menu

:push_complete
echo.
echo ✅ 推送完成！
echo.
echo 你的项目已上传到:
echo   https://github.com/WaWaJie/Operation-System-Course-Design
echo.
echo 按任意键返回主菜单...
pause >nul
goto main_menu

:exit_script
echo 再见！
pause
exit /b 0

REM ========================================
REM 子程序
REM ========================================

:check_git
echo 检查Git环境...
where git >nul 2>nul
if %errorlevel% neq 0 (
    echo ❌ 错误：Git未安装或不在PATH中
    echo 请先安装Git：https://git-scm.com
    pause
    exit /b 1
)

if not exist ".git" (
    echo ❌ 错误：当前目录不是Git仓库
    pause
    exit /b 1
)

git remote get-url origin >nul 2>nul
if %errorlevel% neq 0 (
    echo ⚠️  警告：未设置远程仓库origin
    set /p repo_url=请输入GitHub仓库URL: 
    git remote add origin "%repo_url%"
)
exit /b 0

:check_big_files
echo.
echo 📦 检查大文件（>10MB）...
for /f "delims=" %%f in ('dir /s /b /a-d ^| findstr /v "\\\.git\\"') do (
    for /f %%s in ('powershell -Command "(Get-Item '%%f').Length / 1MB"') do (
        if %%s gtr 10 (
            echo ⚠️  大文件: %%f (%%s MB)
        )
    )
)
exit /b 0

:check_binary_files
echo.
echo 🔍 检查可能不该提交的二进制文件...
where python >nul 2>nul
if %errorlevel% equ 0 (
    python -c "
import os
import subprocess

binary_exts = {'.exe', '.dll', '.pdb', '.obj', '.lib', '.pch', '.ilk'}
for root, dirs, files in os.walk('.'):
    if '.git' in root:
        continue
    for file in files:
        ext = os.path.splitext(file)[1].lower()
        if ext in binary_exts:
            print(f'⚠️  二进制文件: {os.path.join(root, file)}')
"
) else (
    echo （需要Python来检查二进制文件）
)
exit /b 0

:clean_unnecessary
echo.
echo 🧹 检查不必要的文件...
where python >nul 2>nul
if %errorlevel% equ 0 (
    python -c "
import os
import glob

unnecessary_dirs = ['.vs', 'x64', 'Debug', 'Release', 'bin', 'obj']
for dir_name in unnecessary_dirs:
    if os.path.exists(dir_name):
        print(f'⚠️  发现目录: {dir_name} (通常应该忽略)')
"
) else (
    echo （需要Python来检查目录）
)
exit /b 0

:auto_fix
echo.
echo 🛠️  自动修复...
REM 1. 更新.gitignore
if not exist ".gitignore" (
    echo # 创建.gitignore文件
    echo # Visual Studio > .gitignore
    echo .vs/ >> .gitignore
    echo *.user >> .gitignore
    echo *.sln.docstates >> .gitignore
    echo # 编译输出 >> .gitignore
    echo x64/ >> .gitignore
    echo Debug/ >> .gitignore
    echo Release/ >> .gitignore
    echo bin/ >> .gitignore
    echo obj/ >> .gitignore
    echo *.exe >> .gitignore
    echo *.pdb >> .gitignore
    echo *.ilk >> .gitignore
    echo *.obj >> .gitignore
    echo *.pch >> .gitignore
    git add .gitignore
    echo ✅ 已创建.gitignore文件
)

REM 2. 删除已提交的大文件
echo 正在检查已提交的大文件...
git ls-tree -r HEAD --name-only | findstr /i "\.exe$ \.pdb$ \.obj$" >nul
if %errorlevel% equ 0 (
    echo 发现已提交的编译文件，正在清理...
    git rm --cached *.exe *.pdb *.obj *.ilk *.pch 2>nul
)

echo.
echo ✅ 自动修复完成！
exit /b 0