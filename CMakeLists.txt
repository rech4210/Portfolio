cmake_minimum_required(VERSION 3.20)
project(MyGame LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)

# 모듈별 소스 수집
file(GLOB_RECURSE CLIENT_SOURCES "Source/Client/*.cpp")
file(GLOB_RECURSE SERVER_SOURCES "Source/Server/*.cpp")
file(GLOB_RECURSE MYGAME_SOURCES "Source/MyGame/*.cpp")

# 가상 target (빌드 안 함, LSP용)
add_library(Client STATIC ${CLIENT_SOURCES})
add_library(Server STATIC ${SERVER_SOURCES})
add_library(MyGame STATIC ${MYGAME_SOURCES})

# Include 경로는 UnrealEngine 기준
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    "C:/Users/QWER/Git_indie/UnrealEngine/Engine/Source"
    "C:/Users/QWER/Git_indie/UnrealEngine/Engine/Source/Runtime"
)

# 공통 매크로 정의
add_definitions(-DUNICODE -D_UNICODE)

# clangd 탐색을 위한 설정 파일 생성 목적이라면
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
