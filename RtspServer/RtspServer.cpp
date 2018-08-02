/*===============================================================
  File Name:     RtspServer.cpp
  Purpose:       Rtsp server using live555 libs
  PF Melamed     Jun 2018
  ==============================================================*/
/*===============================================================
  System Includes
  ==============================================================*/
#include <stdio.h>
#include <pthread.h>
#include <UsageEnvironment.hh>
#include <BasicUsageEnvironment.hh>
#include <DeviceSource.hh>
#include <GroupsockHelper.hh>
#include <liveMedia.hh>

/*===============================================================
  User Includes
  ==============================================================*/
#include "../CityImg/Common.h"

#define RTSPSERVER_2018_06_CPP
#include "RtspServer.h"
#undef RTSPSERVER_2018_06_CPP

#include "DeviceSource.h"
#include "LiveServerMediaSubsession.hh"

/*===============================================================
  Constants
  ==============================================================*/

/*===============================================================
  Structures
  ==============================================================*/

/*===============================================================
  Variables
  ==============================================================*/
UsageEnvironment *env;
RTSPServer       *rtspServer;
DeviceSource     *H264FrameSource;
Boolean           RtspThreadStarted;

/* To make the second and subsequent client for each stream reuse the same
 input stream as the first client (rather than playing the file from the
 start for each client), change the following "False" to "True": */
Boolean reuseFirstSource = True;

/* return from doEventLoop routine when watchVariable != 0 */
char watchVariable;

/* Running params */

pthread_t RunRtspThreadId;

/*===============================================================
  forward Functions
  ==============================================================*/

/*===============================================================
  Functions
  ==============================================================*/
/*===============================================================
  Function : RunRtspThread
  Purpose : This thread call the live555 doEventLoop
            doEventLoop is a blocking function
  ==============================================================*/
static void *RunRtspThread(void *Arg)
{
  RtspThreadStarted = TRUE;
  
    /* does not return */
  env->taskScheduler().doEventLoop(&watchVariable);

  pthread_exit(NULL);
}

/*===============================================================
  Function : Init_RtspServer
  Purpose : init rtsp server
  ==============================================================*/
void Init_RtspServer(void)
{
  TaskScheduler      *scheduler;
  
    /* Begin by setting up our usage environment: */
  scheduler = BasicTaskScheduler::createNew();
  env       = BasicUsageEnvironment::createNew(*scheduler);

  OutPacketBuffer::maxSize = 128 * 1024 * 1024; 
}

/*===============================================================
  Function : Deinit_RtspServer
  Purpose :  clean rtsp server
  ==============================================================*/
void Deinit_RtspServer(void)
{
}

/*===============================================================
  Function : StartRtspServer
  Purpose : start the rtsp server, 
            call/read live555 params and 
            Launch live555 Threads
  ==============================================================*/
int StartRtspServer(void) 
{ 
  int RetCode;

  RtspThreadStarted = FALSE;
  watchVariable = OFF;
  
    /* Create the RTSP server: */
  int RtspPort = 8554;
  rtspServer = RTSPServer::createNew(*env, RtspPort, NULL);
  if (rtspServer == NULL) {
    printf("[%s %d] Failed to create RTSP server: %s \n",__FILE__,__LINE__,env->getResultMsg());
    return(FAILEDCODE);
  }

  int HttpTunnelingPort = -1;
  if(HttpTunnelingPort > 0) {
    rtspServer->setUpTunnelingOverHTTP(HttpTunnelingPort);
  }

  char *RtspRoute = (char *)"live";
  H264FrameSource = DeviceSource::createNew(*env);
  StreamReplicator *inputDevice = StreamReplicator::createNew(*env, H264FrameSource, false);
  ServerMediaSession *sms = ServerMediaSession::createNew(*env, RtspRoute);
  sms->addSubsession(LiveServerMediaSubsession::createNew(*env, inputDevice));
  rtspServer->addServerMediaSession(sms);
  
    /* now start the thread */
  RetCode = pthread_create(&RunRtspThreadId, NULL,RunRtspThread, NULL);
  if(RetCode != 0) {
    printf("[%s %d] Rtsp pthread create error thread \n",__FILE__,__LINE__);
    return(FAILEDCODE);
  }
  
  return(SUCCESSCODE);
}

/*===============================================================
  Function : StopRtspServer
  Purpose : send "stop signal" to the rtsp server and wait 
            the RunRtspThread stop to work
  ==============================================================*/
void StopRtspServer(void) 
{
  int RetCode;

  if(!RtspThreadStarted) {
    return;
  }
    
  watchVariable = ON;
  
  RetCode = pthread_join(RunRtspThreadId, NULL);
  if (RetCode != 0) {
    printf("[%s,%d] pthread join error %d\n",__FUNCTION__,__LINE__,RetCode);
    exit(FAILEDCODE);
  }

  Medium::close(rtspServer);
  Medium::close(H264FrameSource);
    //Medium::close(inputDevice);
}
