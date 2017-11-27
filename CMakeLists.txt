cmake_minimum_required(VERSION 3.2)
project(esp8266_projects_root)

execute_process(COMMAND ${CMAKE_HOME_DIRECTORY}/configure ${CMAKE_HOME_DIRECTORY})
set(CMAKE_CXX_STANDARD 11)

add_subdirectory(IOTbase)
add_subdirectory(Ambi-Light-LE46B650)
add_subdirectory(HID-Emulator)
add_subdirectory(HUE-Bridge)
add_subdirectory(Decorative-Lights)