@echo off
REM ============================================
REM �Զ����û������� PROJ_LIB Ϊ�ű�Ŀ¼�µ� proj ��Ŀ¼
REM ����ű��� C:\A��������� PROJ_LIB=C:\A\proj
REM ============================================

REM ��ȡ��ǰ�ű�����Ŀ¼��ȥ�����ķ�б�ܣ�
set "CURR_DIR=%~dp0"
set "CURR_DIR=%CURR_DIR:~0,-1%"

REM ƴ�� proj Ŀ¼·��
set "PROJ_PATH=%CURR_DIR%\proj"

REM �����û�����������
setx PROJ_LIB "%PROJ_PATH%"

echo [OK] ������ PROJ_LIB=%PROJ_PATH%
echo �����´������д�������Ч��
pause
