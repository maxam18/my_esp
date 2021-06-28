# My ESP (me) collection
This is the collection of ESP-IDF components for various needs.
# Build
The build configurations based on Cmake ESP-IDF build system.
Just add this into a CMakeFile.txt of your project
_Example_
```
cmake_minimum_required(VERSION 3.5)

set(EXTRA_COMPONENT_DIRS
    .../my_esp/me_ads111x
    .../my_esp/me_mq4
    .../my_esp/me_i2c
    .../my_esp/me_mh741
    .../my_esp/me_bme280
    .../my_esp/me_mhuart
    .../my_esp/me_wifi
    .../my_esp/me_time
    .../my_esp/me_debug
    )
set(COMPONENTS freertos main esp32 esptool_py log me_ads111x me_mq4 i2c me_i2c me_mh741 me_bme280 mhuart me_wifi me_time)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(me_test)
```
# Components
## me_i2c
General purpose i2c communication functions.

