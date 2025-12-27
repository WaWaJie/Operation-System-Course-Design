@echo off
echo 清理项目目录...
echo.

REM 1. 删除空的git文件
if exist "git" (
    echo 删除 git 文件
    del git
)

REM 2. 重命名属性文件
if exist ".gitatributes" (
    echo 重命名 .gitatributes -> .gitattributes
    ren .gitatributes .gitattributes
)

REM 3. 添加必要的忽略规则到.gitignore
if exist ".gitignore" (
    echo 检查.gitignore...
    findstr /C:".vs/" .gitignore >nul || echo .vs/ >> .gitignore
    findstr /C:"x64/" .gitignore >nul || echo x64/ >> .gitignore
    findstr /C:"*.exe" .gitignore >nul || echo *.exe >> .gitignore
)

echo.
echo 清理完成！
pause