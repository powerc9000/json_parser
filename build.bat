@echo off

set CommonCompilerFlags=-MTd -nologo -Gm- -GR- -EHa- -Od -Oi  -W4 -wd4201 -wd4100 -wd4189 -wd4505 -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -DHANDMADE_WIN32=1 -FC -Z7 
set CommonLinkerFlags= -incremental:no -opt:ref 

REM TODO - can we just build both with one exe?

IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build


REM 64-bit build
del *.pdb > NUL 2> NUL
cl %CommonCompilerFlags% ..\json_parser\main.cpp -Fmparser.map /link %CommonLinkerFlags%
popd