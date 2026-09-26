@echo off
setlocal
cd /d "%~dp0.."
if not exist build mkdir build
where cl >nul 2>nul
if errorlevel 1 call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b 1
cl /nologo /std:c11 /W4 /WX /I Core\Inc /I USB_DEVICE\App /I Middlewares\ST\STM32_USB_Device_Library\Core\Inc /I Middlewares\ST\STM32_USB_Device_Library\Class\CustomHID\Inc tests\controller_usb_test.c /Fo:build\controller_usb_test.obj /Fe:build\controller_usb_test.exe
if errorlevel 1 exit /b 1
build\controller_usb_test.exe
if errorlevel 1 exit /b 1
cl /nologo /std:c11 /W4 /WX /I tests\stubs /I Core\Inc tests\mpu_test.c /Fo:build\mpu_test.obj /Fe:build\mpu_test.exe
if errorlevel 1 exit /b 1
build\mpu_test.exe
if errorlevel 1 exit /b 1
cl /nologo /std:c11 /W4 /WX /I tests\stubs /I Core\Inc /I USB_DEVICE\App /I Middlewares\ST\STM32_USB_Device_Library\Core\Inc /I Middlewares\ST\STM32_USB_Device_Library\Class\CustomHID\Inc tests\input_test.c /Fo:build\input_test.obj /Fe:build\input_test.exe
if errorlevel 1 exit /b 1
build\input_test.exe
exit /b %errorlevel%
