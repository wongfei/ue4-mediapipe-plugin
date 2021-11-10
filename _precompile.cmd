set "UE_DIR=C:\dev\UE_4.26"

set "WORK_DIR=%cd%"
set "BATCH_UAT=%UE_DIR%\Engine\Build\BatchFiles\RunUAT.bat"
set "BATCH_BUILD=%UE_DIR%\Engine\Build\BatchFiles\Build.bat"

rmdir /S /Q "%WORK_DIR%\Build\Plugins\MediaPipe"

call "%BATCH_UAT%" BuildPlugin -Plugin="%WORK_DIR%\Plugins\MediaPipe\MediaPipe.uplugin" -Package="%WORK_DIR%\Build\Plugins\MediaPipe" -TargetPlatforms=Win64 -Rocket -precompile -VS2019 -StrictIncludes

xcopy /E /H /Q /Y "%WORK_DIR%\Plugins\MediaPipe\ThirdParty" "%WORK_DIR%\Build\Plugins\MediaPipe\ThirdParty\"

:: call "%BATCH_BUILD%" MediaPipeDemoEditor Win64 Development "%WORK_DIR%\MediaPipeDemo.uproject"
:: call "%BATCH_BUILD%" MediaPipeDemo Win64 Development "%WORK_DIR%\MediaPipeDemo.uproject"
:: call "%BATCH_BUILD%" MediaPipeDemo Win64 Shipping "%WORK_DIR%\MediaPipeDemo.uproject"
