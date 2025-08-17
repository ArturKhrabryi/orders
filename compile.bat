mkdir build
cd build
cmake .. -DCMAKE_CXX_COMPILER="x86_64-w64-mingw32-g++.exe" -G "Ninja" -DCMAKE_BUILD_TYPE=Release
ninja
cpack -G ZIP
