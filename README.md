# UE4 MediaPipe plugin

Platforms: Win64

2D features: Face, Iris, Hands, Pose, Holistic

3D features: Face Mesh, World Pose

Demo video: https://www.youtube.com/watch?v=_gRGjGn6FQE

More details: https://github.com/google/mediapipe

## Custom build howto

### Clone plugin

`git clone https://github.com/wongfei/ue4-mediapipe-plugin.git`

### Clone wrapper

`git clone -b unreal https://github.com/wongfei/mediapipe.git ue4-mediapipe-wrapper`

[Setup workspace](https://google.github.io/mediapipe/getting_started/install.html)

### Build wrapper

`cd ue4-mediapipe-wrapper`

`bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 --action_env PYTHON_BIN_PATH="C:\\Python39\\python.exe" mediapipe/unreal:ump_shared`

More info: `ue4-mediapipe-wrapper\mediapipe\unreal\scripts\build_shared.cmd`

Copy ump_shared.dll to `ue4-mediapipe-plugin\Plugins\MediaPipe\ThirdParty\mediapipe\Binaries\Win64\`

More info: `ue4-mediapipe-wrapper\mediapipe\unreal\scripts\deploy.cmd`

### Other deps

Protobuf 3.11.4

Copy libprotobuf.lib to `ue4-mediapipe-plugin\Plugins\MediaPipe\ThirdParty\protobuf\Lib\Win64\`

OpenCV 3.4.10 with fixed CAP_MSMF (optional)

`git clone -b msmf_fix_3410 https://github.com/wongfei/opencv.git`

Copy opencv_world3410.dll to `ue4-mediapipe-plugin\Plugins\MediaPipe\ThirdParty\mediapipe\Binaries\Win64\`
