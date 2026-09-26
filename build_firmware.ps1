param(
    [string]$Compiler = '',
    [ValidateSet('O0', 'Og', 'O2', 'Os')][string]$Optimization = 'O2'
)
$ErrorActionPreference = 'Stop'
if (!$Compiler) {
    $command = Get-Command arm-none-eabi-gcc -ErrorAction SilentlyContinue
    if ($command) { $Compiler = $command.Source }
    else {
        $Compiler = (Get-ChildItem 'C:/ST/STM32CubeIDE_2.2.0/STM32CubeIDE/plugins' -Filter arm-none-eabi-gcc.exe -Recurse |
            Select-Object -First 1).FullName
    }
}
if (!$Compiler -or !(Test-Path -LiteralPath $Compiler)) { throw 'Pass -Compiler with the path to arm-none-eabi-gcc.exe.' }
$output = Join-Path $PSScriptRoot "build/firmware-$Optimization"
New-Item -ItemType Directory -Force -Path $output | Out-Null
$includeDirs = @('Core/Inc', 'Drivers/STM32F4xx_HAL_Driver/Inc', 'Drivers/STM32F4xx_HAL_Driver/Inc/Legacy',
    'Drivers/CMSIS/Device/ST/STM32F4xx/Include', 'Drivers/CMSIS/Include', 'USB_DEVICE/App', 'USB_DEVICE/Target',
    'Middlewares/ST/STM32_USB_Device_Library/Core/Inc', 'Middlewares/ST/STM32_USB_Device_Library/Class/CustomHID/Inc')
$flags = @('-mcpu=cortex-m4', '-mthumb', '-mfpu=fpv4-sp-d16', '-mfloat-abi=hard', "-$Optimization", '-g3',
    '-DUSE_HAL_DRIVER', '-DSTM32F446xx', '-ffunction-sections', '-fdata-sections', '--specs=nano.specs')
$includes = @($includeDirs | ForEach-Object { '-I' + (Join-Path $PSScriptRoot $_) })
$sourceDirs = @('Core/Src', 'USB_DEVICE/App', 'USB_DEVICE/Target', 'Drivers/STM32F4xx_HAL_Driver/Src',
    'Middlewares/ST/STM32_USB_Device_Library/Core/Src', 'Middlewares/ST/STM32_USB_Device_Library/Class/CustomHID/Src')
$objects = @()
foreach ($dir in $sourceDirs) {
    foreach ($source in Get-ChildItem (Join-Path $PSScriptRoot $dir) -Filter '*.c') {
        $object = Join-Path $output (($dir -replace '/', '_') + '_' + $source.BaseName + '.o')
        & $Compiler @flags @includes '-std=gnu11' '-Wall' '-Werror' '-c' $source.FullName '-o' $object
        if ($LASTEXITCODE) { throw "Compilation failed: $($source.Name)" }
        $objects += $object
    }
}
$startup = Join-Path $output 'startup.o'
& $Compiler @flags '-x' 'assembler-with-cpp' '-c' (Join-Path $PSScriptRoot 'Core/Startup/startup_stm32f446retx.s') '-o' $startup
if ($LASTEXITCODE) { throw 'Startup assembly failed.' }
$objects += $startup
$elf = Join-Path $output 'HID_controller.elf'
& $Compiler @flags @objects '-o' $elf ('-T' + (Join-Path $PSScriptRoot 'STM32F446RETX_FLASH.ld')) '--specs=nosys.specs' `
    '-Wl,--gc-sections' ('-Wl,-Map=' + (Join-Path $output 'HID_controller.map')) '-static' '-Wl,--start-group' '-lc' '-lm' '-Wl,--end-group'
if ($LASTEXITCODE) { throw 'Link failed.' }
$binDir = Split-Path $Compiler
& (Join-Path $binDir 'arm-none-eabi-objcopy.exe') '-O' 'binary' $elf (Join-Path $output 'HID_controller.bin')
if ($LASTEXITCODE) { throw 'Binary conversion failed.' }
& (Join-Path $binDir 'arm-none-eabi-objcopy.exe') '-O' 'ihex' $elf (Join-Path $output 'HID_controller.hex')
if ($LASTEXITCODE) { throw 'HEX conversion failed.' }
& (Join-Path $binDir 'arm-none-eabi-size.exe') $elf
if ($LASTEXITCODE) { throw 'Size check failed.' }
Write-Output "Built $elf"
