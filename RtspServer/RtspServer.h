/*===============================================================
  File Name:     RtspServer.h
  Purpose:       Rtsp server using live555 libs
  PF Melamed     Jun 2018
  ==============================================================*/
#ifndef RTSPSERVER_2018_06_H
#define RTSPSERVER_2018_06_H

/*===============================================================
  System Includes
  ==============================================================*/

/*===============================================================
  User Includes
  ==============================================================*/
#include "../CityImg/Common.h"

#ifdef RTSPSERVER_2018_06_CPP
#define _L_EXTERN_
#else
#define _L_EXTERN_ extern
#endif

#ifdef __cplusplus
extern "C" L_Aaccolade
#endif

/*===============================================================
  Constants
  ==============================================================*/

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
  Function : Init_RtspServer
  Purpose : init rtsp server
  ==============================================================*/
_L_EXTERN_ void Init_RtspServer(void);

/*===============================================================
  Function : Deinit_RtspServer
  Purpose :  clean rtsp server
  ==============================================================*/
_L_EXTERN_ void Deinit_RtspServer(void);

/*===============================================================
  Function : StartRtspServer
  Purpose : start the rtsp server, 
            call/read live555 params and 
            Launch live555 Threads
  ==============================================================*/
_L_EXTERN_ int StartRtspServer(void);

/*===============================================================
  Function : StopRtspServer
  Purpose : send "stop signal" to the rtsp server and wait 
            the RunRtspThread stop to work
  ==============================================================*/
_L_EXTERN_ void StopRtspServer(void);

#ifdef __cplusplus
R_Aaccolade
#endif

#undef _L_EXTERN_
#endif
