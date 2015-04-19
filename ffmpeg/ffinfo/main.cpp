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
int main(int argc, char *argv[])
{
	if( argc>1)
   {
	//Register
	av_register_all();
	
	AVFormatContext *pFormatCtx = NULL;
	
   // Open video file
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
 }
 return 0;
}