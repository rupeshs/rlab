#include "SDL/SDL.h"
#include <process.h>
#include <stdio.h>
#include <conio.h>
#include <math.h>

#define FREQ 200
static Uint8 audio_pos; /* which sample we are up to */
int audio_len; /* how many samples left to play, stops when <= 0 */
float audio_frequency; //audio frequency in cycles per sample 
float audio_volume; /* audio volume, 0 - ~32000 */
	
/* The audio function callback takes the following parameters:
       stream:  A pointer to the audio buffer to be filled
       len:     The length (in bytes) of the audio buffer
*/
void fill_audio(void *udata, Uint8 *stream, int len)
{
    /* Only play if we have data left */
	printf("fill_audio %d:\n",len);

    len /= 2; /* 16 bit */
  int i;
  short* buf = (short*)stream;
  for(i = 0; i < len; i++) { 
  
    buf[i] = audio_volume * sin(2 * 3.14 * audio_pos * audio_frequency);
    printf("i: %d \t%d \t %d\n",i,len,buf[i]);
	audio_pos++;
	
  }
 // SDL_MixAudio(stream, buf , len, SDL_MIX_MAXVOLUME);
  printf("pos:%d audio_len %d \n",audio_pos,audio_len);
  audio_len -= len;
  return;
}

int main(int argc, char *argv[])
{    
	
    char abuffer[1024];
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        printf( "Could not initialize SDL - %s\n", SDL_GetError());
        exit(1);
    }
    else
    {

        printf("Init SDL :OK \n");
        SDL_AudioSpec wanted;


        /* Set the audio format */
        wanted.freq = 44100;
        wanted.format = AUDIO_S16;
        wanted.channels = 1;    /* 1 = mono, 2 = stereo */
        wanted.samples = 1024;  /* Good low-latency value for callback */
        wanted.callback = fill_audio;
        //wanted.userdata = abuffer;

        /* Open the audio device, forcing the desired format */
        if ( SDL_OpenAudio(&wanted, NULL) < 0 ) {
            printf("Couldn't open audio: %s\n", SDL_GetError());
            return(-1);
        }
        printf("Open audio:OK \n");
       audio_len =  10; /* 5 seconds */
  audio_pos = 0;
  audio_frequency = 1.0 * FREQ ; /* 1.0 to make it a float */
  audio_volume = 6000; /* ~1/5 max volume */
  
  SDL_PauseAudio( 0); /* play! */
  
  while(audio_len > 0) {
   printf("main\n");
    SDL_Delay(1000);
  }
        SDL_CloseAudio();

    }


    //Quit SDL
    SDL_Quit();
}
