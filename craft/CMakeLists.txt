# =====================================================================
# Craft - Puerto multiplataforma (Windows EXE / Android APK / iOS IPA)
# Basado en CMakeLists original de fogleman/Craft, extendido con:
#   - SDL2 en lugar de GLFW (ver src/glfw_sdl.c)
#   - Rama Android (libreria "main" + SDL2 compartida)
#   - Rama iOS (bundle .app)
# =====================================================================
cmake_minimum_required(VERSION 3.16)
project(craft C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release)
endif()

get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(HAS_PARENT)
    # Nos estan incluyendo via add_subdirectory (Android/iOS wrapper):
    # el padre define SDL2_SRC_DIR y CRAFT_* .
else()
    # Build directo (escritorio): rutas por defecto relativas a este repo
    if(NOT DEFINED SDL2_SRC_DIR)
        set(SDL2_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../sdl2/SDL2-2.30.11"
            CACHE PATH "Carpeta fuente de SDL2 (Android/iOS)")
    endif()
endif()

set(CRAFT_SOURCES
    src/main.c
    src/glfw_sdl.c
    src/auth.c
    src/client.c
    src/cube.c
    src/db.c
    src/item.c
    src/map.c
    src/matrix.c
    src/ring.c
    src/sign.c
    src/util.c
    src/world.c
    deps/lodepng/lodepng.c
    deps/noise/noise.c
    deps/sqlite/sqlite3.c
    deps/tinycthread/tinycthread.c)

set(CRAFT_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/deps
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/lodepng
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/noise
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/sqlite
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/tinycthread)

# =====================================================================
# ANDROID  -> libreria nativa "main" (cargada por SDLActivity) + SDL2
# =====================================================================
if(ANDROID)
    set(CRAFT_TARGET main)
    add_library(main SHARED
        ${CRAFT_SOURCES}
        ${SDL2_SRC_DIR}/src/main/android/SDL_android_main.c)

    # En movil el stub de glew/ES2 y el shim GLFW van PRIMERO
    target_include_directories(main PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/sdlshim
        ${CRAFT_INCLUDES})

    target_compile_definitions(main PRIVATE CRAFT_MOBILE=1)

    set(SDL_SHARED ON CACHE BOOL "" FORCE)
    set(SDL_STATIC OFF CACHE BOOL "" FORCE)
    set(SDL_TEST_ENABLED OFF CACHE BOOL "" FORCE)
    add_subdirectory(${SDL2_SRC_DIR} ${CMAKE_BINARY_DIR}/sdl2 EXCLUDE_FROM_ALL)

    target_link_libraries(main
        SDL2
        android
        log
        EGL
        GLESv2
        m)

# =====================================================================
# iOS  -> bundle .app (crear con ios-cmake toolchain, ver workflow CI)
# =====================================================================
elseif(IOS)
    set(CRAFT_TARGET craft)
    add_executable(craft MACOSX_BUNDLE ${CRAFT_SOURCES})

    target_include_directories(craft PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/sdlshim
        ${CRAFT_INCLUDES})

    target_compile_definitions(craft PRIVATE CRAFT_MOBILE=1)

    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
    set(SDL_TEST_ENABLED OFF CACHE BOOL "" FORCE)
    add_subdirectory(${SDL2_SRC_DIR} ${CMAKE_BINARY_DIR}/sdl2 EXCLUDE_FROM_ALL)

    target_link_libraries(craft SDL2-static SDL2main)

    # Frameworks de iOS (SDL2-static ya los expone; se repiten por seguridad)
    target_link_libraries(craft
        "-framework UIKit"
        "-framework Foundation"
        "-framework CoreGraphics"
        "-framework QuartzCore"
        "-framework OpenGLES"
        "-framework AudioToolbox"
        "-framework AVFoundation"
        "-framework CoreAudio"
        "-framework GameController"
        "-framework Metal"
        "-framework CoreVideo"
        "-framework Security")

    # Info.plist con nombre, identificador e iconos (craft/ios)
    set_target_properties(craft PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST
        "${CMAKE_CURRENT_SOURCE_DIR}/../ios/Info.plist")

    # Copia textures/, shaders/ e iconos dentro del bundle (raiz, para que
    # las rutas relativas "textures/x.png" funcionen via SDL_GetBasePath)
    add_custom_command(TARGET craft POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/textures
            $<TARGET_BUNDLE_CONTENT_DIR:craft>/textures
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/shaders
            $<TARGET_BUNDLE_CONTENT_DIR:craft>/shaders
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/../ios/icons
            $<TARGET_BUNDLE_CONTENT_DIR:craft>)

# =====================================================================
# ESCRITORIO (Windows / macOS / Linux) -> ejecutable con SDL2 precompilado
# =====================================================================
else()
    set(CRAFT_TARGET craft)
    add_executable(craft ${CRAFT_SOURCES} deps/glew/src/glew.c)

    # deps/glew/include PRIMERO para que resuelve el glew.h real (no el stub)
    target_include_directories(craft BEFORE PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/glew/include)
    target_include_directories(craft PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/sdlshim
        ${CRAFT_INCLUDES})

    # SDL2 precompilado (Windows) o del sistema (macOS/Linux)
    if(WIN32)
        set(SDL2_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../sdl2-devel/SDL2-2.30.11/cmake"
            CACHE PATH "Carpeta cmake de SDL2-devel (Windows)")
        find_package(SDL2 CONFIG REQUIRED)
        target_link_libraries(craft SDL2::SDL2main SDL2::SDL2)
        target_link_libraries(craft ws2_32 opengl32 winmm)
        # Aplicacion Windows sin ventana de consola
        set_target_properties(craft PROPERTIES WIN32_EXECUTABLE TRUE)
        # Copia textures/, shaders/ y SDL2.dll junto al .exe para que
        # funcione con doble clic desde cualquier carpeta
        add_custom_command(TARGET craft POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/textures
                $<TARGET_FILE_DIR:craft>/textures
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/shaders
                $<TARGET_FILE_DIR:craft>/shaders)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(SDL2_DLL_ARCH x64)
        else()
            set(SDL2_DLL_ARCH x86)
        endif()
        add_custom_command(TARGET craft POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${SDL2_DIR}/../lib/${SDL2_DLL_ARCH}/SDL2.dll
                $<TARGET_FILE_DIR:craft>/SDL2.dll)
    else()
        find_package(SDL2 REQUIRED)
        find_package(OpenGL REQUIRED)
        target_link_libraries(craft SDL2::SDL2 OpenGL::GL)
        if(APPLE)
            target_link_libraries(craft "-framework Cocoa")
        else()
            target_link_libraries(craft dl pthread m)
        endif()
    endif()

    # Icono de la app Windows (opcional: existe resources/craft.rc)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources/craft.rc")
        target_sources(craft PRIVATE resources/craft.rc)
    endif()
endif()

# Optimizacion en release (equivalente al -O3 original)
if(NOT MSVC)
    target_compile_options(${CRAFT_TARGET} PRIVATE -O3)
endif()
