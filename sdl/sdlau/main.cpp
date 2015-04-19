#include "SDL/SDL.h"
#include <process.h>
#include <stdio.h>

static Uint8 *audio_chunk;
static Uint32 audio_len;
static Uint8 *audio_pos;

/* The audio function callback takes the following parameters:
       stream:  A pointer to the audio buffer to be filled
       len:     The length (in bytes) of the audio buffer
*/
void fill_audio(void *udata, Uint8 *stream, int len)
{
    /* Only play if we have data left */
    if ( audio_len == 0 )
        return;

    /* Mix as much data as possible */
    len = ( len > audio_len ? audio_len : len );
    SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
    audio_pos += len;
    audio_len -= len;
}

int main(int argc, char *argv[])
{

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        printf( "Could not initialize SDL - %s\n", SDL_GetError());
        exit(1);
    }
    else
    {

        printf("Init SDL :OK \n");
        SDL_AudioSpec wanted;


        /* Set the audio format */
        wanted.freq = 22050;
        wanted.format = AUDIO_S16;
        wanted.channels = 2;    /* 1 = mono, 2 = stereo */
        wanted.samples = 1024;  /* Good low-latency value for callback */
        wanted.callback = fill_audio;
        wanted.userdata = NULL;

        /* Open the audio device, forcing the desired format */
        if ( SDL_OpenAudio(&wanted, NULL) < 0 ) {
            printf("Couldn't open audio: %s\n", SDL_GetError());
            return(-1);
        }
        printf("Open audio:OK \n");
        audio_pos = audio_chunk;

        /* Let the callback function play the audio chunk */
        SDL_PauseAudio(0);
       

        /* Wait for sound to complete */
        while ( audio_len > 0 ) {
            SDL_Delay(100);         /* Sleep 1/10 second */
        }

        SDL_CloseAudio();

    }


    //Quit SDL
    SDL_Quit();
}
