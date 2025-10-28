@echo off
REM ============================================
REM 自动设置环境变量 PROJ_LIB 为脚本目录下的 proj 子目录
REM 例如脚本在 C:\A，则会设置 PROJ_LIB=C:\A\proj
REM ============================================

REM 获取当前脚本所在目录（去掉最后的反斜杠）
set "CURR_DIR=%~dp0"
set "CURR_DIR=%CURR_DIR:~0,-1%"

REM 拼接 proj 目录路径
set "PROJ_PATH=%CURR_DIR%\proj"

REM 设置用户级环境变量
setx PROJ_LIB "%PROJ_PATH%"

echo [OK] 已设置 PROJ_LIB=%PROJ_PATH%
echo 请重新打开命令行窗口以生效。
pause
