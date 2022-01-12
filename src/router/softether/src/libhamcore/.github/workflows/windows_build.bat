@echo on

mkdir %RUNNER_WORKSPACE%\build && cd %RUNNER_WORKSPACE%\build

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat"

cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static-md %GITHUB_WORKSPACE%
cmake --build .
