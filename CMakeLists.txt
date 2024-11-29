cmake_minimum_required(VERSION 3.23)
project(bl1_sdk_mods)

add_custom_target(${PROJECT_NAME})

add_subdirectory(./vendor/pyunrealsdk EXCLUDE_FROM_ALL)
