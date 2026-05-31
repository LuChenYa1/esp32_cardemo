# ESP32模块集成项目
# 项目根CMakeLists.txt配置文件

cmake_minimum_required(VERSION 3.16)

# 包含ESP-IDF项目配置
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# 定义项目名称
project(esp32-module-integration)
