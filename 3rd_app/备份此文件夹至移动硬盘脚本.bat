@echo off
rem ��ȡ��ǰ�ļ�����
setlocal enabledelayedexpansion
set "src=%cd%"
for %%a in ("%src%") do set "foldername=%%~nxa"

rem Ŀ��·��
set "dest=E:\Y.YingGuang\sun����3rd_����\!foldername!"

echo ���ڽ� "%src%" ������ "%dest%" ...
xcopy "%src%\*" "%dest%\" /E /I /Y

echo ������ɣ�
pause
