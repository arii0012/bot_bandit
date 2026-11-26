cmake_minimum_required(VERSION 3.16)
project(BotBanditUnderworld VERSION 1.3.0 LANGUAGES C CXX)

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

set(GAME_SOURCES
  src/main.cpp
  src/Core/Application.cpp
  src/Core/FontUI.cpp
  src/Core/ResourceManager.cpp
  src/Graphics/AssetManager.cpp
  src/Graphics/ProceduralAssetBuilder.cpp
  src/Graphics/ExhaustParticles.cpp
  src/Graphics/Shaders3D.cpp
  src/Graphics/ModelStore.cpp
  src/Graphics/ArtworkRenderer.cpp
  src/UI/UISystem.cpp
  src/UI/InventoryUI.cpp
  src/Character/Character.cpp
  src/Character/CharacterCreator.cpp
  src/World/City.cpp
  src/World/World.cpp
  src/Vehicles/Vehicle.cpp
  src/Vehicles/Garage.cpp
  src/Vehicles/CarCustomizer.cpp
  src/Vehicles/DriftPhysics.cpp
  src/Vehicles/Auction.cpp
  src/Vehicles/TirePhysics.cpp
  src/Cutscenes/CutsceneSystem.cpp
  src/DevConsole/DevConsole.cpp
  src/SaveSystem/SaveSystem.cpp
  src/Systems/GameSystems.cpp
  src/Systems/Crafting.cpp
  src/Systems/Weather.cpp
  src/Systems/Business.cpp
  src/Systems/SmartHome.cpp
  src/Systems/Damage.cpp
  src/Systems/EngineAudio.cpp
  src/Systems/AudioEngine.cpp
  src/Smartphone/Smartphone.cpp
  src/Minigames/Fighting.cpp
  src/Minigames/Racing.cpp
  src/Minigames/DJ.cpp
)

add_executable(botbandit ${GAME_SOURCES})
if(WIN32 AND MSVC)
  target_sources(botbandit PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/resources/app.rc)
endif()

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
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Minigames
)

target_link_libraries(botbandit PRIVATE raylib nlohmann_json::nlohmann_json)
target_compile_features(botbandit PRIVATE cxx_std_20)

if(MSVC)
  add_compile_options(/Zc:char8_t-)
  target_compile_options(botbandit PRIVATE /W3 /permissive- /utf-8 /EHsc)
  target_compile_definitions(botbandit PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
  target_compile_options(botbandit PRIVATE -Wall -Wextra)
endif()

if(WIN32)
  target_link_libraries(botbandit PRIVATE winmm)
elseif(UNIX AND NOT APPLE)
  find_package(X11 REQUIRED)
  find_package(OpenGL REQUIRED)
  target_link_libraries(botbandit PRIVATE ${X11_LIBRARIES} OpenGL::GL pthread dl m)
endif()

add_custom_command(TARGET botbandit POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:botbandit>/assets
  COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:botbandit>/shaders
  COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:botbandit>/data
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:botbandit>/assets
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:botbandit>/shaders
  COMMENT "Copy assets and shaders"
)

if(WIN32)
  add_executable(LifeSim3D_Setup
    installer/setup_main.cpp
    installer/win_admin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/app.rc
  )
  target_include_directories(LifeSim3D_Setup PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/installer)
  set_target_properties(LifeSim3D_Setup PROPERTIES OUTPUT_NAME BotBandit_Setup)
  target_link_libraries(LifeSim3D_Setup PRIVATE raylib winmm shell32 ole32 uuid advapi32 winhttp)
  target_compile_features(LifeSim3D_Setup PRIVATE cxx_std_20)
  if(MSVC)
    target_compile_options(LifeSim3D_Setup PRIVATE /W3 /permissive- /utf-8 /EHsc)
    target_compile_definitions(LifeSim3D_Setup PRIVATE _CRT_SECURE_NO_WARNINGS)
  endif()
  add_custom_command(TARGET LifeSim3D_Setup POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:LifeSim3D_Setup>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:LifeSim3D_Setup>/assets
  )
endif()

# Optional character texture prep (Pillow)
find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_FOUND)
  add_custom_target(prep_character
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/prepare_character_assets.py
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Prepare character PNG textures")
endif()
