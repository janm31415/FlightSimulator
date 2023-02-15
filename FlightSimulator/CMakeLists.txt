if (WIN32)
  set(OPENGL_LIBRARIES opengl32.lib CACHE FILEPATH "opengl lib file")
  set(GLX_LIBRARIES "")
endif (WIN32)

if (UNIX)
  if (APPLE)
    set(OPENGL_LIBRARIES "" CACHE FILEPATH "opengl lib file")
    set(GLX_LIBRARIES "")
    
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework Cocoa -framework QuartzCore -framework Metal")
    set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -framework Cocoa -framework QuartzCore -framework Metal")
  else()
    set(OPENGL_LIBRARIES "/usr/lib/x86_64-linux-gnu/libOpenGL.so.0.0.0" CACHE FILEPATH "opengl lib file")
    set(GLX_LIBRARIES "/usr/lib/x86_64-linux-gnu/libGLX.so" CACHE FILEPATH "glx lib files")
  endif(APPLE)
endif (UNIX)

set(HDRS
data.h
debug.h
gl_shaders.h
flightmodel.h
physics.h
view.h
)
	
set(SRCS
debug.cpp
gl_shaders.cpp
flightmodel.cpp
main.cpp
physics.cpp
view.cpp
)

set(STB
${CMAKE_CURRENT_SOURCE_DIR}/../stb/stb_image.h
${CMAKE_CURRENT_SOURCE_DIR}/../stb/stb_image_write.h
)

set(SHADERS
)

if (APPLE)
list(APPEND HDRS ../SDL-metal/SDL_metal.h)
list(APPEND SRCS ../SDL-metal/SDL_metal.mm)
list(APPEND SHADERS ../RenderDoos/RenderDoos/shaders.metal)
endif (APPLE)

if (WIN32)
set(CMAKE_C_FLAGS_DEBUG "/W4 /MP /GF /RTCu /Od /MDd /Zi")
set(CMAKE_CXX_FLAGS_DEBUG "/W4 /MP /GF /RTCu /Od /MDd /Zi")
set(CMAKE_C_FLAGS_RELEASE "/W4 /MP /GF /O2 /Ob2 /Oi /Ot /MD /Zi")
set(CMAKE_CXX_FLAGS_RELEASE "/W4 /MP /GF /O2 /Ob2 /Oi /Ot /MD /Zi")
endif(WIN32)

include ("../jtk/jtk/jtk.cmake")

# general build definitions
add_definitions(-DNOMINMAX)
add_definitions(-D_UNICODE)
add_definitions(-DUNICODE)
add_definitions(-D_SCL_SECURE_NO_WARNINGS)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-DMEMORY_LEAK_TRACKING)

if (WIN32)
add_executable(FlightSimulator WIN32 ${HDRS} ${SRCS} ${SHADERS})
else()
add_executable(FlightSimulator ${HDRS} ${SRCS} ${STB} ${SHADERS})
endif(WIN32)
source_group("Header Files" FILES ${hdrs})
source_group("Source Files" FILES ${srcs})
source_group("ThirdParty/stb" FILES ${STB})
if (APPLE)
set_source_files_properties(${SHADERS} PROPERTIES LANGUAGE METAL)
endif (APPLE)

target_include_directories(FlightSimulator
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../jtk/
    ${CMAKE_CURRENT_SOURCE_DIR}/../stb/ 
    ${CMAKE_CURRENT_SOURCE_DIR}/../RenderDoos
    ${CMAKE_CURRENT_SOURCE_DIR}/../SDL2/include/
    )	
	
if (${FLIGHTSIM_PLATFORM} STREQUAL "win32")	
target_link_libraries(FlightSimulator    
    ${OPENGL_LIBRARIES}
    ${GLX_LIBRARIES}
    RenderDoos
    libglew
    SDL2
    SDL2main
    )
else(${FLIGHTSIM_PLATFORM} STREQUAL "win32")
target_link_libraries(FlightSimulator             
    RenderDoos
    SDL2
    SDL2main
    )
endif(${FLIGHTSIM_PLATFORM} STREQUAL "win32")       