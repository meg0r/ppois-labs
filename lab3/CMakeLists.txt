cmake_minimum_required(VERSION 3.15)
project(FileManagementSystemLab3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(file_manager_app
    src/main.cpp
    src/StorageHardware.cpp
    src/FileHierarchy.cpp
    src/SecurityAccess.cpp
    src/IOOperations.cpp
    src/SearchAndMetadata.cpp
    src/LifecycleAndBackup.cpp
)

# Указываем компилятору, где искать заголовочные файлы (.hpp)
target_include_directories(file_manager_app PRIVATE include)

if(MSVC)
    target_compile_options(file_manager_app PRIVATE /W4 /EHsc)
else()
    target_compile_options(file_manager_app PRIVATE -Wall -Wextra)
endif()