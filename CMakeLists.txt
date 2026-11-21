cmake_minimum_required(VERSION 3.16)
project(BotBanditUnderworld VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include(FetchContent)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG 5.5
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(raylib)

FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(json)

set(SOURCES
  src/main.cpp
  src/Core/Application.cpp
  src/Graphics/AssetManager.cpp
  src/Graphics/ProceduralAssetBuilder.cpp
  src/UI/UISystem.cpp
  src/Character/Character.cpp
  src/Character/CharacterCreator.cpp
  src/World/City.cpp
  src/World/World.cpp
  src/Vehicles/Vehicle.cpp
  src/Cutscenes/CutsceneSystem.cpp
  src/DevConsole/DevConsole.cpp
  src/SaveSystem/SaveSystem.cpp
  src/Systems/GameSystems.cpp
)

add_executable(botbandit ${SOURCES})

target_include_directories(botbandit PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Core
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Graphics
  ${CMAKE_CURRENT_SOURCE_DIR}/src/UI
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Character
  ${CMAKE_CURRENT_SOURCE_DIR}/src/World
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Vehicles
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Smartphone
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Cutscenes
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Systems
  ${CMAKE_CURRENT_SOURCE_DIR}/src/DevConsole
  ${CMAKE_CURRENT_SOURCE_DIR}/src/SaveSystem
)

target_link_libraries(botbandit PRIVATE raylib nlohmann_json::nlohmann_json)

if(MSVC)
  target_compile_options(botbandit PRIVATE /W3 /permissive-)
  target_compile_definitions(botbandit PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
  target_compile_options(botbandit PRIVATE -Wall -Wextra)
endif()

if(WIN32)
  target_link_libraries(botbandit PRIVATE winmm)
endif()

add_custom_command(TARGET botbandit POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:botbandit>/assets
  COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:botbandit>/data
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:botbandit>/assets
  COMMENT "Copy assets"
)

install(TARGETS botbandit RUNTIME DESTINATION bin)
install(DIRECTORY assets/ DESTINATION bin/assets)
