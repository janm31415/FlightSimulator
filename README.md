# FlightSimulator

Heavily inspired by https://github.com/gue-ni/OpenGL_Flightsim.

Should work on Windows with OpenGL, and on Macos with Metal.

https://user-images.githubusercontent.com/60060546/221649370-893c1fd0-d354-4bbe-8d93-547aab42b0b1.mov

## Building

All dependencies are delivered with the code as submodule, so first call

    git submodule update --init
    
to download all the dependencies.

Next use CMake to create a solution file on Windows, or an XCode project on Macos. You have to set the CMAKE variables FLIGHTSIM_ARCHITECTURE and FLIGHTSIM_PLATFORM to the correct values. The defaults are correct for Windows, but if you have macos with an ARM processor, you have to change these values with CMake to `arm` and `macos` respectively.

