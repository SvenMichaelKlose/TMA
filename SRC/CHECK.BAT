@echo off
rem sorry, doesn't recognize errors
mode con:lines=50

echo --- Building TMA...
del ..\bin\tma.com
call make
echo --- See if it works ...
..\bin\tma.com tma.asm
echo --- ... and the new one...
.\tma.com tma.asm
echo --- ...we don't want a surprise.
.\tma.com tma.asm
echo --- Ok, lets try the examples...
copy tma.com ..\bin\
del tma.com
cd ..\examples\3dtorus
call make
cd ..\dataland
call make
cd ..\relief
call make
cd ..\hc
call make
cd ..\truncate
call make
cd ..\fliplay
call make
cd ..\tsrclock
call make
cd ..\relief
call make
cd ..\mandel
call make
cd ..\genesis
call make
cd ..\..\src
