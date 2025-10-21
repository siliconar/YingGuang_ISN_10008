@echo off
rem 获取当前文件夹名
setlocal enabledelayedexpansion
set "src=%cd%"
for %%a in ("%src%") do set "foldername=%%~nxa"

rem 目标路径
set "dest=E:\Y.YingGuang\sun代码3rd_备份\!foldername!"

echo 正在将 "%src%" 拷贝到 "%dest%" ...
xcopy "%src%\*" "%dest%\" /E /I /Y

echo 拷贝完成！
pause
