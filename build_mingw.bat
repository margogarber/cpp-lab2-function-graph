@echo off
rem Збірка за допомогою MinGW-w64 (g++ має бути у PATH)
if not exist build mkdir build
g++ -std=c++17 -O2 -Wall -municode -mwindows -static -DUNICODE -D_UNICODE src\main.cpp -o build\Lab02.exe
if %errorlevel% neq 0 (
    echo Build FAILED
    exit /b 1
)
echo Build OK: build\Lab02.exe
