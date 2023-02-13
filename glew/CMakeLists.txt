if (WIN32)
  set(OPENGL_LIBRARIES opengl32.lib CACHE FILEPATH "opengl lib file")
  set(GLX_LIBRARIES "")
endif (WIN32)

if (UNIX)
  if (APPLE)
    set(OPENGL_LIBRARIES "/System/Library/Frameworks/OpenGL.framework/OpenGL" CACHE FILEPATH "opengl lib file")
    set(GLX_LIBRARIES "")
    
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework Cocoa -framework QuartzCore -framework Metal")
    set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -framework Cocoa -framework QuartzCore -framework Metal")
  else()
    set(OPENGL_LIBRARIES "/usr/lib/x86_64-linux-gnu/libOpenGL.so.0.0.0" CACHE FILEPATH "opengl lib file")
    set(GLX_LIBRARIES "/usr/lib/x86_64-linux-gnu/libGLX.so" CACHE FILEPATH "glx lib files")
  endif(APPLE)
endif (UNIX)

add_library(libglew SHARED glew.cpp)

 target_include_directories(libglew
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../RenderDoos/glew/include
    )	
	
target_link_libraries(libglew             
    ${OPENGL_LIBRARIES}
    ${GLX_LIBRARIES}
    )	       