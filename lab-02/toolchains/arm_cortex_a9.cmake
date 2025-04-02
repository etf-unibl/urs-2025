set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_SYSROOT /home/lab1204-0/x-tools/arm-etfbl-linux-gnueabihf/arm-etfbl-linux-gnueabihf/sysroot)
set(CMAKE_STAGING_PREFIX ./build)

set(tools /home/lab1204-0/x-tools/arm-etfbl-linux-gnueabihf)
set(CMAKE_C_COMPILER ${tools}/bin/arm-linux-gcc)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

