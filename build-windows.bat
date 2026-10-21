@echo off
setlocal
if "%JUCE_DIR%"=="" (
  echo Set JUCE_DIR to your JUCE folder first.
  echo Example: set JUCE_DIR=C:\JUCE
  exit /b 1
)
cmake -S . -B build -DJUCE_DIR="%JUCE_DIR%" -G "Visual Studio 17 2022" -A x64
if errorlevel 1 exit /b 1
cmake --build build --config Release --target OrbitTexture_VST3
if errorlevel 1 exit /b 1
echo.
echo Build complete. Look in build\OrbitTexture_artefacts\Release\VST3\
pause
