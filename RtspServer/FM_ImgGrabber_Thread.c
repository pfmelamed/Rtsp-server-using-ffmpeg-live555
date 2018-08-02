/*===============================================================
  File Name:     FM_ImgGrabber_Thread.c
  Purpose:       grab image thread using the ffmpeg libs 
  PF Melamed     Jun 2014
  ==============================================================*/
/*===============================================================
  System Includes
  ==============================================================*/
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libavutil/opt.h>

/*===============================================================
  User Includes
  ==============================================================*/
#include "../CityImg/Common.h"

#define FM_IMGGRABBER_THREAD_2014_06_C
#include "FM_ImgGrabber_Thread.h"
#undef FM_IMGGRABBER_THREAD_2014_06_C

#include "../RtspServer/DeviceSource.h"
#include "../RtspServer/RtspServer.h"

/*===============================================================
  Constants
  ==============================================================*/
char *IpCamServerURL = "http://quiquepl51.dyndns.org:51515/mjpg/video.mjpg";  /* Camera URL */

/*===============================================================
  Structures
  ==============================================================*/

/*===============================================================
  Variables
  ==============================================================*/

/*===============================================================
  Functions
  ==============================================================*/
/*===============================================================
  Function Name: FM_ImgGrabber_Thread
  Purpose:       Image grabber thread using the ffmpeg
  ==============================================================*/
void *FM_ImgGrabber_Thread(void *Arg)
{
  int            ImgWidth;
  int            ImgHeight;
  unsigned char *YUVImage;
  int            BitRate;
  int            StreamFrameRate;
  int            GopSize;

    /* ffmpeg */
  AVFormatContext   *InFormatCtxPtr;
  int                VideoStremPos;
  AVStream          *InVideoStPtr;
  AVCodec           *InCodecPtr;
  AVCodecContext    *InCodecCtxPtr;
  AVFrame           *RawFramePtr;
  AVFrame           *YUVFramePtr;
  AVPacket           InPacketAv;
  int                EndOfInFrame;
  struct SwsContext *ImgConvertYUVCtxPtr;
  AVCodec           *OutCodecPtr;
  AVCodecContext    *OutCodecCtxPtr;
  AVPacket           OutPacket;
  unsigned int       FrameIdx;
  int                GotFrame;
  
  int RetCode;
  
  /* Start to work */
  ImgWidth  = 640;
  ImgHeight = 480;
  YUVImage  = (unsigned char *)malloc((ImgWidth * ImgHeight / 2) * 3);

    /* Init ffmpeg params is donne in ffmpeg_Utils */
  InFormatCtxPtr      = NULL;
  InCodecCtxPtr       = NULL;
  RawFramePtr         = NULL;
  YUVFramePtr         = NULL;
  ImgConvertYUVCtxPtr = NULL;
  OutCodecCtxPtr      = NULL;


  RetCode = StartRtspServer();
  if(RetCode == FAILEDCODE) {
    printf("[%s %d] Couldn't start the RTSPServer thread\n",  __FILE__, __LINE__);
    exit(0);
  }

  avformat_network_init();          /* The network function */
  av_log_set_level(AV_LOG_FATAL);   /* Something went wrong and recovery is not possible. AV_LOG_DEBUG */
    
    /* Open video file */
  if(avformat_open_input(&InFormatCtxPtr, IpCamServerURL, NULL, NULL) != 0) {
    printf("[%s %d] Connection to %s failed.\n", __FILE__, __LINE__, IpCamServerURL);
    exit(0);
  }
    
    /* Retrieve stream information */
  if(avformat_find_stream_info(InFormatCtxPtr, NULL) < 0) {
    printf("[%s %d] Couldn't find stream (%s) information.\n",  __FILE__, __LINE__, IpCamServerURL);
    exit(0);
  }
    
    /* Find the first video stream */
  RetCode = av_find_best_stream(InFormatCtxPtr, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
  if(RetCode < 0) {
    printf("[%s %d]  Didn't find a video stream (%s).\n",  __FILE__, __LINE__, IpCamServerURL);
    exit(0);
  }
  
  VideoStremPos = RetCode;
  InVideoStPtr =  InFormatCtxPtr->streams[VideoStremPos];
  
    /* Find the decoder for the video stream - Open codec */
  InCodecPtr = avcodec_find_decoder(InVideoStPtr->codecpar->codec_id);
  InCodecCtxPtr = avcodec_alloc_context3(InCodecPtr);
  if(InCodecCtxPtr == NULL) {
    printf("[%s %d] Can't alloc cotext!\n",  __FILE__, __LINE__);
    exit(0);
  }
    /* Open the codec */
  RetCode = avcodec_open2(InCodecCtxPtr, InCodecPtr, NULL);
  if(RetCode < 0){
    printf("[%s %d] Could not open codec! (%d)\n",  __FILE__, __LINE__,RetCode);
    exit(0);
  }
    
    /* Allocate video frame (Raw buffer)*/
  RawFramePtr = av_frame_alloc();
  if(RawFramePtr == NULL) {
    printf("[%s %d] Could not Allocate video frame!\n", __FILE__, __LINE__);
    exit(0);
  }

    /* Malloc the YUV buffer */
  YUVFramePtr = av_frame_alloc();
  if(YUVFramePtr == NULL) {
    printf("[%s %d] Could not Allocate video frame!\n", __FILE__, __LINE__);
    exit(0);
  }
  
  av_image_fill_arrays(YUVFramePtr->data,YUVFramePtr->linesize,YUVImage,AV_PIX_FMT_YUV420P,ImgWidth,ImgHeight,1);
  
    /* init the Packet */
  av_init_packet(&InPacketAv);
  InPacketAv.data = NULL;
  InPacketAv.size = 0;
  
    /* Read the first frame */
  while(FOREVER) {
    if(av_read_frame(InFormatCtxPtr, &InPacketAv) >= 0) {
        
        /* Is this a packet from the video stream? */
      if(InPacketAv.stream_index == VideoStremPos) {
        
          /* Decode video frame */
        avcodec_send_packet(InCodecCtxPtr, &InPacketAv);
        EndOfInFrame =  avcodec_receive_frame(InCodecCtxPtr,RawFramePtr);
        if(EndOfInFrame == 0) {            
          ImgConvertYUVCtxPtr = sws_getContext(InVideoStPtr->codecpar->width,InVideoStPtr->codecpar->height,InVideoStPtr->codecpar->format,
                                               ImgWidth,ImgHeight,AV_PIX_FMT_YUV420P,
                                               SWS_LANCZOS, NULL, NULL, NULL);
          if(ImgConvertYUVCtxPtr == NULL) {
            printf("[%s %d] Can't alloc Convert cotext!\n",  __FILE__, __LINE__);
            exit(0);
          }
          av_packet_unref(&InPacketAv);
          
          break;
        }
      }
    }
    av_packet_unref(&InPacketAv);
  }

     /* Non blocking read */
  InFormatCtxPtr->flags = InFormatCtxPtr->flags | AVFMT_FLAG_NONBLOCK; 
  InFormatCtxPtr->flags = InFormatCtxPtr->flags | AVIO_FLAG_NONBLOCK;

    /* H264 Params */
  BitRate         = 400000;
  StreamFrameRate = 25;
  GopSize         = 12;

    /* find the H264 video encoder */
  OutCodecPtr = avcodec_find_encoder(AV_CODEC_ID_H264);
  if (!OutCodecPtr) {
    printf("[%s %d] Codec not found\n",__FILE__,__LINE__);
    exit(FAILEDCODE);
  }
    
    OutCodecCtxPtr = avcodec_alloc_context3(OutCodecPtr);
    if (OutCodecCtxPtr == NULL) {
      printf("[%s %d] Could not allocate video codec context",__FILE__,__LINE__);
      exit(FAILEDCODE);
    }
    
      /* put sample parameters */
    OutCodecCtxPtr->bit_rate     = BitRate;
    OutCodecCtxPtr->width        = ImgWidth;
    OutCodecCtxPtr->height       = ImgHeight;
    OutCodecCtxPtr->time_base    = (AVRational){1,StreamFrameRate};
    OutCodecCtxPtr->gop_size     = GopSize; /* emit one intra frame every ten frames */
    OutCodecCtxPtr->max_b_frames = 1;
    OutCodecCtxPtr->pix_fmt      = AV_PIX_FMT_YUV420P;
    av_opt_set(OutCodecCtxPtr->priv_data, "preset", "ultrafast", 0);
    av_opt_set(OutCodecCtxPtr->priv_data, "crf", "0", 0);
    av_opt_set(OutCodecCtxPtr->priv_data, "tune", "film", 0);
  
     /* open it */
    if (avcodec_open2(OutCodecCtxPtr, OutCodecPtr, NULL) < 0) {
      printf("[%s %d] Could not open codec\n",__FILE__,__LINE__);
      exit(FAILEDCODE);
    }

    FrameIdx = 0;

    /* The Main loop */
  while(FOREVER) {
  
    if(av_read_frame(InFormatCtxPtr, &InPacketAv) < 0) {
      av_packet_unref(&InPacketAv);
      continue;
    }
          
      /* Is this a packet from the video stream? */
    if(InPacketAv.stream_index == VideoStremPos) {
            
        /* Decode video frame */
      avcodec_send_packet(InCodecCtxPtr, &InPacketAv);
      EndOfInFrame =  avcodec_receive_frame(InCodecCtxPtr,RawFramePtr);
      if(EndOfInFrame == 0) {
        sws_scale(ImgConvertYUVCtxPtr, (const uint8_t * const*)RawFramePtr->data,
                  RawFramePtr->linesize, 0, InVideoStPtr->codecpar->height,
                  YUVFramePtr->data, YUVFramePtr->linesize);


        YUVFramePtr->pts = FrameIdx;
        
        av_init_packet(&OutPacket);
        OutPacket.data = NULL;    /* packet data will be allocated by the encoder */
        OutPacket.size = 0;

          /* encode the image */
        avcodec_send_frame(OutCodecCtxPtr,YUVFramePtr);
        GotFrame = avcodec_receive_packet(OutCodecCtxPtr, &OutPacket);
        if (GotFrame == 0) {

          signalNewFrameData(OutPacket.data + 4,OutPacket.size - 4);
          ++FrameIdx;
        }
      
        av_packet_unref(&OutPacket);
      }
    } 
    av_packet_unref(&InPacketAv);
  }

   /* close staff from last setion (ADD it in the clenup func) */
  StopRtspServer();
  
  free(YUVImage);

  if(YUVFramePtr != NULL) {
    av_free(YUVFramePtr);
    YUVFramePtr = NULL;
  }
      
  if(RawFramePtr != NULL) {
    av_free(RawFramePtr);
    RawFramePtr = NULL;
  }
      
  if(InCodecCtxPtr != NULL) {
    avcodec_close(InCodecCtxPtr);
    av_free(InCodecCtxPtr);
    InCodecCtxPtr = NULL;
  }
  
  if(InFormatCtxPtr != NULL) {
    avformat_close_input(&InFormatCtxPtr);
    InFormatCtxPtr = NULL;
  }
  if(ImgConvertYUVCtxPtr != NULL) {
    sws_freeContext(ImgConvertYUVCtxPtr);
    ImgConvertYUVCtxPtr = NULL;
  }

  if(OutCodecCtxPtr != NULL) {
    avcodec_close(OutCodecCtxPtr);
    av_free(OutCodecCtxPtr);
    OutCodecCtxPtr = NULL;
  }

  avformat_network_deinit();
   
  pthread_exit(NULL);
}

