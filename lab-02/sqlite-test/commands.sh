mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchains/arm_cortex_a9.cmake
cmake --build .
