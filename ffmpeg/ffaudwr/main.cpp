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

/*SDL audio buffer size, in samples. */
#define SDL_AUDIO_BUFFER_SIZE 1024

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
    
    if(audio_len==0)		/*  Only  play  if  we  have  data  left  */
        return;
    len=(len>audio_len?audio_len:len);	/*  Mix  as  much  data  as  possible  */

    SDL_MixAudio(stream,audio_pos,len,SDL_MIX_MAXVOLUME);
    printf("audio_len : %d audio pos:%d :len : %d  \n",audio_len ,audio_pos,len);
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
    uint8_t			*audio_buffer;
    SDL_AudioSpec wanted_spec,spec;
    int index = 0;
    uint32_t len = 0;
    int data_size=0;

    //if( argc>1)
    {
        //redirect SDL output to console
        //freopen("CON", "w", stdout); // redirects stdout
        //freopen("CON", "w", stderr); // redirects stderr

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
		// MP3 decodes to S16P which we don't support, tell it to use S16 instead.
       if (aCodecCtx->sample_fmt == AV_SAMPLE_FMT_S16P)
	   {   
           printf ("Found planar audio S16P \n");
		   aCodecCtx->request_sample_fmt = AV_SAMPLE_FMT_S16;
	   }

        // Open codec
        if(avcodec_open2(aCodecCtx, aCodec, NULL)){
            printf("Could not open codec.\n");
            return -1;
        }
        packet=(AVPacket *)av_malloc(sizeof(AVPacket));
        av_init_packet(packet);
        pFrame=av_frame_alloc();

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



        /* Set the audio format */
        //SDL_AudioSpec
        wanted_spec.freq =  aCodecCtx->sample_rate;
        wanted_spec.format = AUDIO_S16SYS;
        wanted_spec.channels =aCodecCtx->channels;
        wanted_spec.silence = 0;
        wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE ;
        wanted_spec.callback = fill_audio;
        wanted_spec.userdata = NULL;

        /* Open the audio device, forcing the desired format */
        if ( SDL_OpenAudio(&wanted_spec,&spec) < 0 ) {
            printf("Couldn't open audio: %s\n", SDL_GetError());
            return(-1);
        }
        printf("Open audio:OK \n");

        //Setting up out_buffer
         audio_buffer=(uint8_t *)av_malloc(MAX_AUDIO_FRAME_SIZE*2);
		
		/*
		AV_SAMPLE_FMT_S16P is planar signed 16 bit audio, i.e. 2 bytes for each sample which is same for AV_SAMPLE_FMT_S16.
        The only difference is in AV_SAMPLE_FMT_S16 samples of each channel are interleaved i.e. if you have two channel 
	    audio then the samples buffer will look like
        c1 c1 c2 c2 c1 c1 c2 c2...
        where c1 is a sample for channel1 and c2 is sample for channel2.
        while for one frame of planar audio you will have something like
		c1 c1 c1 c1 .... c2 c2 c2 c2 .. 
		now how is it stored in AVFrame: for planar audio:
        data[i] will contain the data of channel i (assuming channel 0 is first channel).
        however if you have more channels then 8 then data for rest of the channels can be found in extended_data attribute of AVFrame.
        for non-planar audio data[0] will contain the data for all channels in an interleaved manner.	
	   */


        printf ("aCodecCtx->sample_fmt %d \n",aCodecCtx->sample_fmt);
		
        while(av_read_frame(pFormatCtx, packet)>=0){

            //Check packet for audio
            if(packet->stream_index==audioStream){

                //Decode it
                int ret = avcodec_decode_audio4( aCodecCtx, pFrame,&got_frame, packet);
                if ( ret < 0 ) {
                    printf("Error in decoding audio frame.\n");
                    continue;
                }

                if (got_frame){

                    //printf("index:%5d\t pts:%lld\t packet size:%d\n",index,packet->pts,packet->size);
                    data_size = av_samples_get_buffer_size(NULL,
                                               aCodecCtx->channels,
                                               pFrame->nb_samples,
                                               AV_SAMPLE_FMT_S16,
                                               1);
											   
											   

                       // assert(data_size <= buf_size);
                        memcpy(audio_buffer, pFrame->data[0], data_size);
                        printf ("data_size :%d nbsample:%d \n", data_size,pFrame->nb_samples);

                    if(wanted_spec.samples!=pFrame->nb_samples){
                        printf("FIX:FLAC,MP3,AAC Different number of samples");
                        SDL_CloseAudio();

                         data_size=av_samples_get_buffer_size(NULL,aCodecCtx->channels ,pFrame->nb_samples,AV_SAMPLE_FMT_S16, 1);
                        wanted_spec.samples=pFrame->nb_samples;
                        if (SDL_OpenAudio(&wanted_spec, &spec)<0){
                            printf("can't open audio.\n");
                            return -1;
                        }
                    }

                   index++;
                }

                //Play
                //Set audio buffer (PCM data)
                audio_chunk = (Uint8 *) audio_buffer;
                //Audio buffer length
                audio_len = data_size ;
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
        av_free(audio_buffer);
        av_frame_free(&pFrame);

        // Close the codec
        avcodec_close(aCodecCtxOrig);
        avcodec_close(aCodecCtx);

        // Close the video file
        avformat_close_input(&pFormatCtx);
    }

    return 0;
}
