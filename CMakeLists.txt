cmake_minimum_required(VERSION 2.8.12)
project("BooBoo")

# Options

option(GAME_LAUNCHER "Build the game launcher" ON)
option(LUA "Link with Lua for benchmarks" OFF)
option(DEBUG "Debug build" OFF)
option(USE_STATIC_LIBS "Link with static libraries" ON)
option(RASPBERRYPI "Build for Raspberry Pi" OFF)

set(USER_INCLUDE_PATH "" CACHE FILEPATH "Any extra include directories you want")
set(USER_LIBRARY_PATH "" CACHE FILEPATH "Any extra library directories you want")

include_directories("include")
include_directories(${USER_INCLUDE_PATH})
link_directories(${USER_LIBRARY_PATH})
link_directories(${CMAKE_BINARY_DIR})

set(SHIM4_LIB shim4)
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

if(WIN32)
	list(APPEND GAME_LAUNCHER_SOURCES misc/booboo.manifest misc/versioninfo.rc misc/windows_icons.rc)
endif()

# Outputs

add_library("library" STATIC ${LIB_SOURCES})
add_executable("corelauncher" ${CORE_LAUNCHER_SOURCES})

if(GAME_LAUNCHER)
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
		set(FLAGS "${FLAGS} -DTGUI6_STATIC -DSHIM4_STATIC")
	endif()
	set_target_properties("library" PROPERTIES COMPILE_FLAGS "${FLAGS}")
	set_target_properties("library" PROPERTIES OUTPUT_NAME "BooBoo")
	set_target_properties("corelauncher" PROPERTIES COMPILE_FLAGS "${FLAGS}")
	set_target_properties("corelauncher" PROPERTIES LINK_FLAGS "${LFLAGS}")
	set_target_properties("corelauncher" PROPERTIES OUTPUT_NAME "BooBooCMD")

	if(GAME_LAUNCHER)
		set_target_properties("gamelauncher" PROPERTIES COMPILE_FLAGS "${FLAGS}")
		set_target_properties("gamelauncher" PROPERTIES LINK_FLAGS "${LFLAGS_WIN32}")
		set_target_properties("gamelauncher" PROPERTIES OUTPUT_NAME "BooBoo")
	endif()
	if(LUA)
		set(CORE_LAUNCHER_LIBS lua5.4 "library" mingw32 imm32 setupapi version)
		set(GAME_LAUNCHER_LIBS lua5.4 "library" ${SHIM4_LIB} util ${TGUI6_LIB} mingw32 SDL2main SDL2 png16 vorbis FLAC ogg freetype zlibstatic d3d9 winmm dbghelp shcore msvcrt imm32 setupapi version)
	else()
		set(CORE_LAUNCHER_LIBS "library" ${SHIM4_LIB} util ${TGUI6_LIB} mingw32 SDL2main SDL2 png16 vorbis FLAC ogg freetype zlibstatic d3d9 winmm dbghelp shcore msvcrt imm32 setupapi shlwapi version)
		set(GAME_LAUNCHER_LIBS "library" ${SHIM4_LIB} util ${TGUI6_LIB} mingw32 SDL2main SDL2 png16 vorbis FLAC ogg freetype zlibstatic d3d9 winmm dbghelp shcore msvcrt imm32 setupapi shlwapi version)
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
		set(FLAGS "${FLAGS} /DTGUI6_STATIC /DSHIM4_STATIC")
	endif()
	#set(FLAGS "${FLAGS} /DUSE_TTF")
	set_target_properties("gamelauncher" PROPERTIES COMPILE_FLAGS "${FLAGS}")
	set_target_properties("gamelauncher" PROPERTIES OUTPUT_NAME "BooBoo")
	set(LIBS ${LIBS} ${SHIM4_LIB} util ${TGUI6_LIB} SDL2 SDL2main libpng16_static ogg libvorbis_static FLAC freetype zlibstatic d3d9 winmm dbghelp shcore msvcrt)# lua54)
elseif(APPLE)
	SET(FLAGS "-Wall -Wno-extern-c-compat -Wno-c++11-narrowing -Wno-overloaded-virtual -DTGUI6_STATIC -DSHIM4_STATIC")
	set(FLAGS "${FLAGS} -stdlib=libc++ -std=c++11")
	if(DEBUG)
		set(FLAGS "${FLAGS} -DDEBUG=1 -g")
	endif()
	set_target_properties("gamelauncher" PROPERTIES BUILD_WITH_INSTALL_RPATH on)
	set_target_properties("gamelauncher" PROPERTIES INSTALL_RPATH "@executable_path/")
	set_target_properties("gamelauncher" PROPERTIES COMPILE_FLAGS "${FLAGS}")
	set_target_properties("gamelauncher" PROPERTIES LINK_FLAGS "-framework Cocoa -framework IOKit -framework OpenGL -framework CoreAudio -framework AudioUnit -framework ForceFeedback -framework Carbon")
	set_target_properties("gamelauncher" PROPERTIES OUTPUT_NAME "BooBoo")
	set(LIBS ${LIBS} ${SHIM4_LIB} util ${TGUI6_LIB} SDL2 z)
else() # Linux
	SET(FLAGS "-Wall -Wno-comment -Wno-strict-aliasing -std=c++0x")
	SET(FLAGS "${FLAGS} -DUSE_TTF -DUSE_PNG")
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
	if (NOT DEBUG AND RASPBERRYPI)
		set(CMAKE_C_FLAGS_RELEASE "-Os -march=armv8-a")
		set(CMAKE_CXX_FLAGS_RELEASE "-Os -march=armv8-a")
	elseif(NOT RASPBERRYPI)
		set(CMAKE_C_FLAGS_RELEASE "-O2")
		set(CMAKE_CXX_FLAGS_RELEASE "-O2")
	endif()
	set_target_properties("library" PROPERTIES COMPILE_FLAGS "${FLAGS}")
	set_target_properties("library" PROPERTIES OUTPUT_NAME "BooBoo")
	set_target_properties("corelauncher" PROPERTIES COMPILE_FLAGS "${FLAGS}")
	set_target_properties("corelauncher" PROPERTIES LINK_FLAGS "${LFLAGS}")
	set_target_properties("corelauncher" PROPERTIES OUTPUT_NAME "BooBooCMD")
	if(GAME_LAUNCHER)
		set_target_properties("gamelauncher" PROPERTIES COMPILE_FLAGS "${FLAGS}")
		set_target_properties("gamelauncher" PROPERTIES LINK_FLAGS "${LFLAGS}")
		set_target_properties("gamelauncher" PROPERTIES OUTPUT_NAME "BooBoo")
	endif()
	if(LUA)
		set(CORE_LAUNCHER_LIBS "library" lua5.4)
		set(GAME_LAUNCHER_LIBS "library" ${SHIM4_LIB} util ${TGUI6_LIB} SDL2 GL Xcursor X11 FLAC vorbis ogg freetype png z lua5.4)
	else()
		set(CORE_LAUNCHER_LIBS "library" ${SHIM4_LIB} util ${TGUI6_LIB} SDL2 GL Xcursor X11 FLAC vorbis ogg freetype png z)
		set(GAME_LAUNCHER_LIBS "library" ${SHIM4_LIB} util ${TGUI6_LIB} SDL2 GL Xcursor X11 FLAC vorbis ogg freetype png z)
	endif()
endif()

target_link_libraries("corelauncher" ${CORE_LAUNCHER_LIBS})

if(GAME_LAUNCHER)
	target_link_libraries("gamelauncher" ${GAME_LAUNCHER_LIBS})
endif()
