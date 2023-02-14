#ifndef _SDL_main_h
#define _SDL_main_h
#endif

#include "SDL.h"
#include "debug.h"
#include "view.h"

#define JTK_QBVH_IMPLEMENTATION
#include "jtk/qbvh.h"

int main(int argc, char** argv)
  {
  init_debug();
  SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE);

  if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
    {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Initilizated SDL Failed: %s", SDL_GetError());
    return -1;
    }
  {
  view my_view(argc, argv);
  my_view.loop();
  }
  SDL_Quit();
  close_debug();
  return 0;
  }
