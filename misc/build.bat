@echo off
mkdir ..\..\build
pushd ..\..\build
cl -FC -Zi ..\handmade\code\handmade.cpp user32.lib Gdi32.lib
popd
