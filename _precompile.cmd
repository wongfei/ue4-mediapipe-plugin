set "UE_DIR=C:\dev\UE_4.26"

set "WORK_DIR=%cd%"
set "BATCH_UAT=%UE_DIR%\Engine\Build\BatchFiles\RunUAT.bat"
set "BATCH_BUILD=%UE_DIR%\Engine\Build\BatchFiles\Build.bat"

"%BATCH_UAT%" BuildPlugin -Plugin="%WORK_DIR%\Plugins\MediaPipe\MediaPipe.uplugin" -Package="%WORK_DIR%\_Precompiled" -TargetPlatforms=Win64 -Rocket -precompile -VS2019

:: call "%BATCH_BUILD%" MediaPipeDemoEditor Win64 Development "%WORK_DIR%\MediaPipeDemo.uproject"
:: call "%BATCH_BUILD%" MediaPipeDemo Win64 Development "%WORK_DIR%\MediaPipeDemo.uproject"
:: call "%BATCH_BUILD%" MediaPipeDemo Win64 Shipping "%WORK_DIR%\MediaPipeDemo.uproject"
