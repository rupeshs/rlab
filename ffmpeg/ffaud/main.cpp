/*  This file is part of rlab
    Copyright (C) 2015 Rupesh Sreeraman
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define __STDC_CONSTANT_MACROS

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "SDL/SDL.h"
};
#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio

static Uint8 *audio_chunk;
static Uint32 audio_len;
static Uint8 *audio_pos;

/* The audio function callback takes the following parameters:
       stream:  A pointer to the audio buffer to be filled
       len:     The length (in bytes) of the audio buffer
*/
void fill_audio(void *udata, Uint8 *stream, int len)
{
	
    //SDL 2.0
	SDL_memset(stream, 0, len);
	//printf("fill_audio: %d \n",stream[10]);
	if(audio_len==0)		/*  Only  play  if  we  have  data  left  */ 
			return; 
	len=(len>audio_len?audio_len:len);	/*  Mix  as  much  data  as  possible  */ 
  
	SDL_MixAudio(stream,audio_pos,len,SDL_MIX_MAXVOLUME);
	//printf("fill_audio: %d \n",stream[10]);
	audio_pos += len; 
	audio_len -= len; 
}

int main(int argc, char *argv[])
{
   
    AVFormatContext *pFormatCtx = NULL;
    AVPacket		*packet;
    int 			got_frame;
    AVFrame			*pFrame;
	int64_t 		in_channel_layout;
	struct SwrContext *au_convert_ctx;
	uint8_t			*out_buffer;
   SDL_AudioSpec wanted_spec;
   int index = 0;
   	uint32_t len = 0;
 
    //if( argc>1)
    {
       //redirect SDL output to console 
        freopen("CON", "w", stdout); // redirects stdout
        freopen("CON", "w", stderr); // redirects stderr
		
        //Register
        av_register_all();
        avformat_network_init();
char url[]="e:/adele.mp3";

        // Open url
        if(avformat_open_input(&pFormatCtx, url, NULL, NULL)!=0)
            return -1; // Couldn't open file

        // Retrieve stream information
        if(avformat_find_stream_info(pFormatCtx, NULL)<0)
        {
            printf ("Failed to retrieve stream info!");
            return -1; // Couldn't find stream information
        }
        // Dump information about file onto standard error
        av_dump_format(pFormatCtx, 0, argv[1], 0);

        int i;
        for(i=0; i<pFormatCtx->nb_streams; i++)
        {
            if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
                printf("Has Video\n");
            if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO)
                printf("Has Audio\n");
            if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_SUBTITLE )
                printf("Has Subtitle\n");
            if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_ATTACHMENT  )
                printf("Has Attachment\n");
            if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_DATA   )
                printf("Has Data\n");
        }

        int audioStream=-1;
        for(i=0; i < pFormatCtx->nb_streams; i++) {
            if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO &&
                    audioStream < 0) {
                audioStream=i;
            }
        }

        printf("Audio Stream id:%d \n",audioStream);


        AVCodecContext *aCodecCtxOrig;
        AVCodecContext *aCodecCtx;

        aCodecCtxOrig=pFormatCtx->streams[audioStream]->codec;

        AVCodec         *aCodec;
        aCodec = avcodec_find_decoder(aCodecCtxOrig->codec_id);
        if(!aCodec) {
            printf("Unsupported codec!\n");
            return -1;
        }

        // Copy context
        aCodecCtx = avcodec_alloc_context3(aCodec);
        if(avcodec_copy_context(aCodecCtx, aCodecCtxOrig) != 0) {
            printf( "Couldn't copy codec context\n");
            return -1; // Error copying codec context
        }

        // Open codec
        if(avcodec_open2(aCodecCtx, aCodec, NULL)){
            printf("Could not open codec.\n");
            return -1;
        }

        printf("Bitrate:\t %3d\n", pFormatCtx->bit_rate);
        printf("Decoder Name:\t %s\n", aCodecCtx->codec->long_name);
        printf("Channels:\t %d\n", aCodecCtx->channels);
        printf("Sample per Second\t %d \n", aCodecCtx->sample_rate);
		//***************************************************************
		//SDL
		 
		 if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
          printf( "Could not initialize SDL - %s\n", SDL_GetError());
         return -1;
        }
		
		
		printf("Init SDL :OK \n");
        


		  packet=(AVPacket *)av_malloc(sizeof(AVPacket));
        av_init_packet(packet);
		//***************************************************************

      //Out Audio Param
	   uint64_t out_channel_layout=AV_CH_LAYOUT_STEREO;
	   int out_nb_samples=1024;
	   
	   AVSampleFormat out_sample_fmt=AV_SAMPLE_FMT_S16;
	  int out_sample_rate=44100;
	  int out_channels=av_get_channel_layout_nb_channels(out_channel_layout);
 
 
     //Out Buffer Size
	  int out_buffer_size=av_samples_get_buffer_size(NULL,out_channels ,out_nb_samples,out_sample_fmt, 1);

	  out_buffer=(uint8_t *)av_malloc(MAX_AUDIO_FRAME_SIZE*2);
	     
		 /* Set the audio format */
      //SDL_AudioSpec
	wanted_spec.freq = out_sample_rate; 
	wanted_spec.format = AUDIO_S16SYS; 
	wanted_spec.channels = out_channels; 
	wanted_spec.silence = 0; 
	wanted_spec.samples = out_nb_samples; 
	wanted_spec.callback = fill_audio; 
	wanted_spec.userdata = aCodecCtx; 

        /* Open the audio device, forcing the desired format */
        if ( SDL_OpenAudio(&wanted_spec, NULL) < 0 ) {
            printf("Couldn't open audio: %s\n", SDL_GetError());
            return(-1);
        }
        printf("Open audio:OK \n");
	  //FIX:Some Codec's Context Information is missing
	in_channel_layout=av_get_default_channel_layout(aCodecCtx->channels);
	//Swr

	au_convert_ctx = swr_alloc();
	au_convert_ctx=swr_alloc_set_opts(au_convert_ctx,out_channel_layout, out_sample_fmt, out_sample_rate,
		in_channel_layout,aCodecCtx->sample_fmt , aCodecCtx->sample_rate,0, NULL);
	swr_init(au_convert_ctx);
	
	
	
   
	
        pFrame=av_frame_alloc();

      

       

        while(av_read_frame(pFormatCtx, packet)>=0){

            if(packet->stream_index==audioStream){

                int ret = avcodec_decode_audio4( aCodecCtx, pFrame,&got_frame, packet);
                if ( ret < 0 ) {
                    printf("Error in decoding audio frame.\n");
                    return -1;
                }
                if (got_frame){
				printf("index:%5d\t pts:%lld\t packet size:%d\n",index,packet->pts,packet->size);

                 int ret=swr_convert(au_convert_ctx,&out_buffer, MAX_AUDIO_FRAME_SIZE,(const uint8_t **)pFrame->data , pFrame->nb_samples);

				if (ret < 0)  
        {  
            printf("swr_convert error \n");  
            return -1;  
        }  
		if(wanted_spec.samples!=pFrame->nb_samples){
				printf("FIX:FLAC,MP3,AAC Different number of samples"); 
					SDL_CloseAudio();
					out_nb_samples=pFrame->nb_samples;
					out_buffer_size=av_samples_get_buffer_size(NULL,out_channels ,out_nb_samples,out_sample_fmt, 1);
					wanted_spec.samples=out_nb_samples;
					if (SDL_OpenAudio(&wanted_spec, NULL)<0){ 
		printf("can't open audio.\n"); 
		return -1; 
	} 
				}
                 

                }
              
                  
					//Play
			       
				index++;
				
				//Set audio buffer (PCM data)
			audio_chunk = (Uint8 *) out_buffer; 
			//Audio buffer length
			audio_len =out_buffer_size;

			audio_pos = audio_chunk;
			//Play
			SDL_PauseAudio(0);
			while(audio_len>0)//Wait until finish
			{ 
				SDL_Delay(1); 
			}
            }
            av_free_packet(packet);

        }

        //Clean up stuff
		swr_free(&au_convert_ctx);
		av_free(out_buffer);
        av_frame_free(&pFrame);

        // Close the codec
        avcodec_close(aCodecCtxOrig);
        avcodec_close(aCodecCtx);

        // Close the video file
        avformat_close_input(&pFormatCtx);
    }

    return 0;
}
