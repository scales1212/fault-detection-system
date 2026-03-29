@echo off
REM Build and test the Fault Detection System.
REM Requires: VS 2022 Build Tools (includes cmake/ctest)
REM
REM Usage:
REM   build.bat            — configure (first run), build Release, run all tests
REM   build.bat clean      — delete build/ and rebuild from scratch

SET VS_CMAKE=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe
SET VS_CTEST=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe

IF "%1"=="clean" (
    echo Cleaning build directory...
    rmdir /s /q build
)

IF NOT EXIST build (
    echo Configuring...
    "%VS_CMAKE%" -B build -G "Visual Studio 17 2022" -Wno-dev
    IF ERRORLEVEL 1 ( echo CMake configure failed & exit /b 1 )
)

echo Building Release...
"%VS_CMAKE%" --build build --config Release
IF ERRORLEVEL 1 ( echo Build failed & exit /b 1 )

echo Running C++ tests...
"%VS_CTEST%" --test-dir build -C Release --output-on-failure
IF ERRORLEVEL 1 ( echo C++ tests failed & exit /b 1 )

echo Running Python tests...
python -m pytest gui\tests\ -v
IF ERRORLEVEL 1 ( echo Python tests failed & exit /b 1 )

echo.
echo All done. Run: python launch.py
