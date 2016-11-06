@echo off
cd art
python ..\tools\img2ardu.py sprites.png
move sprites_bitmap.cpp ..\code\frogwizard\sprites_bitmap.cpp
move sprites_bitmap.h ..\code\frogwizard\sprites_bitmap.h
cd ..
pause