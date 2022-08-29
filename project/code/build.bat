@echo off

set CommonCompilerFlags=-MTd -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4456 -wd4505 -wd4127 -wd4996 -wd4312 -wd4661 -DQLIB_INTERNAL=1 -DQLIB_SLOW=1 -DSAVE_IMAGES=1 -DQLIB_WIN32=1 -DVSYNC=0 -FC -Z7

set CommonLinkerFlags= -incremental:no -opt:ref Dxgi.lib %CD%\qlib\sdl-vc\lib\x64\SDL2.lib %CD%\qlib\sdl-vc\lib\x64\SDL2main.lib

IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build

cl %CommonCompilerFlags% C:\Stuff\sound_visualizer\project\code\audio_visualizer.cpp /link %CommonLinkerFlags%

popd