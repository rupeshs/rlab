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
#define __STDC_CONSTANT_MACROS

extern "C"
{
#include <libavcodec/avcodec.h>
#include "libavformat/avformat.h"
};
#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio

int main(int argc, char *argv[])
{
    FILE		    *pFile=NULL;
    AVFormatContext *pFormatCtx = NULL;
    AVPacket		*packet;
    int 			got_frame;
    AVFrame			*pFrame;

    if( argc>1)
    {
        //Register
        av_register_all();
        avformat_network_init();

        // Open url
        if(avformat_open_input(&pFormatCtx, argv[1], NULL, NULL)!=0)
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



        pFrame=av_frame_alloc();

        packet=(AVPacket *)av_malloc(sizeof(AVPacket));
        av_init_packet(packet);

        pFile=fopen("output.pcm", "wb");

        while(av_read_frame(pFormatCtx, packet)>=0){

            if(packet->stream_index==audioStream){

                int ret = avcodec_decode_audio4( aCodecCtx, pFrame,&got_frame, packet);
                if ( ret < 0 ) {
                    printf("Error in decoding audio frame.\n");
                    return -1;
                }
                if (got_frame){

                    //Write PCM
                    fwrite((const uint8_t **)pFrame->data, 1, pFrame->nb_samples, pFile);

                    printf("%d\t", pFrame->nb_samples);

                }

            }
            av_free_packet(packet);

        }

        //Clean up stuff

        fclose(pFile);
        av_frame_free(&pFrame);

        // Close the codec
        avcodec_close(aCodecCtxOrig);
        avcodec_close(aCodecCtx);

        // Close the video file
        avformat_close_input(&pFormatCtx);
    }

    return 0;
}
