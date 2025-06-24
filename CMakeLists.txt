cmake_minimum_required(VERSION 3.5)
project("BooBoo")

# Options

option(GAME_LAUNCHER "Build the game launcher" ON)
option(LUA "Link with Lua for benchmarks" OFF)
option(DEBUG "Debug build" OFF)
option(USE_STATIC_LIBS "Link with static libraries" ON)
option(RASPBERRYPI "Build for Raspberry Pi" OFF)

set(USER_INCLUDE_PATH "" CACHE FILEPATH "Any extra include directories you want")
set(USER_LIBRARY_PATH "" CACHE FILEPATH "Any extra library directories you want")

set(CMAKE_RC_COMPILER "/mingw32/bin/windres.exe")

include_directories("include")
include_directories(${USER_INCLUDE_PATH})
link_directories(${USER_LIBRARY_PATH})
link_directories(${CMAKE_BINARY_DIR})

set(SHIM5_LIB shim5)
set(TGUI6_LIB tgui6)

# Inputs

set(LIB_SOURCES
	src/booboo.cpp
	src/standard_lib.cpp
)

set(GAME_LAUNCHER_SOURCES
	src/game_launcher.cpp
	src/game_lib.cpp
)

set(CORE_LAUNCHER_SOURCES
	src/launcher.cpp
)

if(MINGW)
	add_custom_command(OUTPUT "${PROJECT_SOURCE_DIR}/misc/versioninfo.rc.res" COMMAND "/mingw64/bin/windres" ARGS "-O" "coff" "${PROJECT_SOURCE_DIR}/misc/versioninfo.rc" "${PROJECT_SOURCE_DIR}/misc/versioninfo.rc.res")
	add_custom_command(OUTPUT "${PROJECT_SOURCE_DIR}/misc/windows_icons.rc.res" COMMAND "/mingw64/bin/windres" ARGS "-O" "coff" "${PROJECT_SOURCE_DIR}/misc/windows_icons.rc" "${PROJECT_SOURCE_DIR}/misc/windows_icons.rc.res")
	list(APPEND GAME_LAUNCHER_SOURCES misc/booboo.manifest)
endif()

# Outputs

add_library("corelibrary" STATIC ${LIB_SOURCES})
add_executable("corelauncher" ${CORE_LAUNCHER_SOURCES})

if(GAME_LAUNCHER)
	add_library("gamelibrary" STATIC ${LIB_SOURCES})
	add_executable("gamelauncher" ${GAME_LAUNCHER_SOURCES})
endif()

# Platform specifics

if(MINGW)
	if(GAME_LAUNCHER)
		SET(FLAGS "${FLAGS} -DUSE_TTF -DUSE_PNG")
	endif()
	set(LFLAGS "${LFLAGS} -static -static-libgcc -static-libstdc++")
	set(LFLAGS_WIN32 "${LFLAGS} -Wl,--subsystem,windows")
	if(DEBUG)
		set(FLAGS "${FLAGS} -DDEBUG=1 -g")
		set(LFLAGS "${LFLAGS} -g")
	endif()
	if(USE_STATIC_LIBS AND GAME_LAUNCHER)
		set(FLAGS "${FLAGS} -DTGUI6_STATIC -DSHIM5_STATIC")
	endif()
	set_target_properties("corelibrary" PROPERTIES COMPILE_FLAGS "${FLAGS} -DCLI")
	set_target_properties("corelibrary" PROPERTIES OUTPUT_NAME "BooBooCore")
	set_target_properties("corelauncher" PROPERTIES COMPILE_FLAGS "${FLAGS} -DCLI")
	set_target_properties("corelauncher" PROPERTIES LINK_FLAGS "${LFLAGS}")
	set_target_properties("corelauncher" PROPERTIES OUTPUT_NAME "BooBooCLI")

	if(GAME_LAUNCHER)
		set_target_properties("gamelibrary" PROPERTIES COMPILE_FLAGS "${FLAGS}")
		set_target_properties("gamelibrary" PROPERTIES OUTPUT_NAME "BooBooGame")
		set_target_properties("gamelauncher" PROPERTIES COMPILE_FLAGS "${FLAGS}")
		set_target_properties("gamelauncher" PROPERTIES LINK_FLAGS "${LFLAGS_WIN32}" "${PROJECT_SOURCE_DIR}/misc/versioninfo.rc.res" "${PROJECT_SOURCE_DIR}/misc/windows_icons.rc.res")
		set_target_properties("gamelauncher" PROPERTIES OUTPUT_NAME "BooBoo")
	endif()
	if(LUA)
		set(CORE_LAUNCHER_LIBS lua5.4 "corelibrary" mingw32 imm32 setupapi version)
		set(GAME_LAUNCHER_LIBS lua5.4 "gamelibrary" twinkle ${SHIM5_LIB} util ${TGUI6_LIB} mingw32 SDL3 SDL3_ttf png16 vorbis FLAC ogg freetype zlibstatic d3d9 winmm dbghelp shcore msvcrt imm32 setupapi version)
	else()
		set(CORE_LAUNCHER_LIBS mingw32 "corelibrary" twinkle util zlibstatic shlwapi)
		set(GAME_LAUNCHER_LIBS "gamelibrary" twinkle ${SHIM5_LIB} util ${TGUI6_LIB} mingw32 SDL3 SDL3_ttf png16 vorbis FLAC ogg freetype zlibstatic d3d9 winmm dbghelp shcore msvcrt imm32 setupapi shlwapi version)
	endif()
elseif(WIN32)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /IGNORE:4075 /IGNORE:4099")
	SET(FLAGS "${FLAGS} /D_CRT_SECURE_NO_WARNINGS /D_USE_MATH_DEFINES /EHsc /MP /W3 /wd4244 /wd4800 /wd4101 /MT")
	SET(FLAGS "${FLAGS} /DNTDDI_VERSION=0x06010000 /D_WIN32_WINNT=0x601")
	SET(FLAGS "${FLAGS} /DUSE_TTF /DUSE_PNG")
	if(DEBUG)
		set(FLAGS "${FLAGS} /DDEBUG=1")
	endif()
	if(USE_STATIC_LIBS)
		set(FLAGS "${FLAGS} /DTGUI6_STATIC /DSHIM5_STATIC")
	endif()
	#set(FLAGS "${FLAGS} /DUSE_TTF")
	set_target_properties("gamelauncher" PROPERTIES COMPILE_FLAGS "${FLAGS}")
	set_target_properties("gamelauncher" PROPERTIES OUTPUT_NAME "BooBoo")
	set(LIBS ${LIBS} twinkle ${SHIM5_LIB} util ${TGUI6_LIB} SDL3 SDL3_ttf libpng16_static ogg libvorbis_static FLAC freetype zlibstatic d3d9 winmm dbghelp shcore msvcrt)

elseif(APPLE)
	SET(FLAGS "-Wall -Wno-extern-c-compat -Wno-c++11-narrowing -Wno-overloaded-virtual -DTGUI6_STATIC -DSHIM5_STATIC")
	set(FLAGS "${FLAGS} -stdlib=libc++ -std=c++11")
	if(DEBUG)
		set(FLAGS "${FLAGS} -DDEBUG=1 -g")
	endif()
	set_target_properties("gamelauncher" PROPERTIES BUILD_WITH_INSTALL_RPATH on)
	set_target_properties("gamelauncher" PROPERTIES INSTALL_RPATH "@executable_path/")
	set_target_properties("gamelauncher" PROPERTIES COMPILE_FLAGS "${FLAGS}")
	set_target_properties("gamelauncher" PROPERTIES LINK_FLAGS "-framework Cocoa -framework IOKit -framework OpenGL -framework CoreAudio -framework AudioUnit -framework ForceFeedback -framework Carbon")
	set_target_properties("gamelauncher" PROPERTIES OUTPUT_NAME "BooBoo")
	set(LIBS ${LIBS} twinkle ${SHIM5_LIB} util ${TGUI6_LIB} SDL3 SDL3_ttf z)
else() # Linux
	SET(FLAGS "-Wall -Wno-comment -Wno-strict-aliasing -std=c++0x")
	SET(FLAGS "${FLAGS} -DUSE_TTF -DUSE_PNG")
	if (RASPBERRYPI)
		SET(FLAGS "${FLAGS} -DRASPBERRYPI")
	endif()
	set(LFLAGS "-Wl,-rpath,\$ORIGIN")
	if(DEBUG)
		#set(FLAGS "${FLAGS} -DDEBUG=1 -fsanitize=address -fsanitize=leak")
		#set(LFLAGS "${LFLAGS} -fsanitize=address -fsanitize=leak")
		#SET(LIBS asan ${LIBS})
		set(FLAGS "${FLAGS} -DDEBUG=1 -g -pg")
		set(LFLAGS "${LFLAGS} -g -pg")
		#set(FLAGS "${FLAGS} -DDEBUG=1 -g")
		#set(LFLAGS "${LFLAGS} -g")
	else()
		set(FLAGS "${FLAGS}")
	endif()
	set(CMAKE_C_FLAGS_RELEASE "-O3")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
	set_target_properties("corelibrary" PROPERTIES COMPILE_FLAGS "${FLAGS} -DCLID")
	set_target_properties("corelibrary" PROPERTIES OUTPUT_NAME "BooBooCore")
	set_target_properties("corelauncher" PROPERTIES COMPILE_FLAGS "${FLAGS} -DCLI")
	set_target_properties("corelauncher" PROPERTIES LINK_FLAGS "${LFLAGS}")
	set_target_properties("corelauncher" PROPERTIES OUTPUT_NAME "BooBooCLI")
	if(GAME_LAUNCHER)
		set_target_properties("gamelibrary" PROPERTIES COMPILE_FLAGS "${FLAGS}")
		set_target_properties("gamelibrary" PROPERTIES OUTPUT_NAME "BooBooGame")
		set_target_properties("gamelauncher" PROPERTIES COMPILE_FLAGS "${FLAGS}")
		set_target_properties("gamelauncher" PROPERTIES LINK_FLAGS "${LFLAGS}")
		set_target_properties("gamelauncher" PROPERTIES OUTPUT_NAME "BooBoo")
	endif()
	if(LUA)
		set(CORE_LAUNCHER_LIBS "corelibrary" lua5.4)
		set(GAME_LAUNCHER_LIBS "gamelibrary" twinkle ${SHIM5_LIB} util ${TGUI6_LIB} SDL3 SDL3_ttf GL Xcursor X11 FLAC vorbis ogg freetype png z lua5.4 GL)
	else()
		set(CORE_LAUNCHER_LIBS "corelibrary" twinkle ${SHIM5_LIB} util ${TGUI6_LIB} SDL3 SDL3_ttf GL Xcursor X11 FLAC vorbis ogg freetype png z)
		set(GAME_LAUNCHER_LIBS "gamelibrary" twinkle ${SHIM5_LIB} util ${TGUI6_LIB} SDL3 SDL3_ttf GL Xcursor X11 FLAC vorbis ogg freetype png z)
	endif()
endif()

target_link_libraries("corelauncher" ${CORE_LAUNCHER_LIBS})

if(GAME_LAUNCHER)
	if(MINGW)
		target_link_libraries("gamelauncher" ${GAME_LAUNCHER_LIBS} "${PROJECT_SOURCE_DIR}/misc/versioninfo.rc.res" "${PROJECT_SOURCE_DIR}/misc/windows_icons.rc.res")
	else()
		target_link_libraries("gamelauncher" ${GAME_LAUNCHER_LIBS})
	endif()
endif()
