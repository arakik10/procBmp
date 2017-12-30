@echo off
set PATHC=C:\Program Files\mingw-w64\x86_64-6.2.0-posix-seh-rt_v5-rev1\mingw64\bin
set PATH=%PATHC%;%PATH%
.\AStyle\bin\AStyle.exe^
  --mode=c^
  --style=linux^
  --indent=spaces=2^
  --indent-switches^
  --pad-oper^
  --pad-header^
  %~n1.c
del a.exe
echo gcc build start
echo ------------------------------------------------------------
gcc %~n1.c
echo ------------------------------------------------------------
echo gcc build end
if exist a.exe (
echo program start
echo ------------------------------------------------------------
a.exe
echo;
echo ------------------------------------------------------------
echo program end
if exist %~n1*~ (del /q %~n1*~)
if exist %~n1*orig (del /q %~n1*orig)
)
Pause

