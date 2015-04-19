#include "SDL/SDL.h"
#include <process.h>
int main(int argc, char *argv[])
{
	printf("Init SDL :OK");
if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
  fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
  exit(1);
}
else
{
printf("Init SDL :OK");
	
}
//Quit SDL
 SDL_Quit();
}