@echo off
echo Making TMA...
rem DON'T CHANGE TMABCKUP! You can just compile it with itself.
tmabckup tma.asm
if errorlevel 0 	echo Ok.
md ..\bin
copy tma.com ..\bin
del tma.com
if errorlevel 0         echo Executable: ..\bin\tma.com

