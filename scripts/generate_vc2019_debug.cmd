@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars32.bat"

REM Сборка
cd ..
if not exist bld mkdir bld
cd bld
cmake -G "Visual Studio 16 2019" -A "Win32" ../src
pause