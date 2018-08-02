/*===============================================================
  File Name:     DeviceSource.cpp
  Purpose:       
  Modify by:
  PF Melamed     Jul 2018
  ==============================================================*/
/*===============================================================
  System Includes
  ==============================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <queue>
#include "GroupsockHelper.hh" /* for "gettimeofday()" */
#include "DeviceSource.hh"

/*===============================================================
  User Includes
  ==============================================================*/
#include "../CityImg/Common.h"

#define DeviceSource_2018_07_CPP
#include "DeviceSource.h"
#undef DeviceSource_2018_07_CPP

/*===============================================================
  Constants
  ==============================================================*/
#define QueueCapacity 30 /* the H264Frame Queue Capacity */

/*===============================================================
  Structures
  ==============================================================*/

/*===============================================================
  Class
  ==============================================================*/
class H264FrameCls {
public :
  uint8_t       *Frame;
  uint32_t       FrameSize;
  struct timeval FramePTime;
  
public :
  H264FrameCls() {
    Frame = NULL;
    FrameSize = 0;
    gettimeofday(&FramePTime, NULL);
  }
      
  ~H264FrameCls() {
    if(Frame != NULL) {
      delete Frame;
      Frame = NULL;
    }
  }
};
 
/*===============================================================
  Variables
  ==============================================================*/
EventTriggerId  DeviceSource::eventTriggerId = 0;
unsigned        DeviceSource::referenceCount = 0;
DeviceSource   *_L_H264FrameSource = NULL;

/* Frame Queue */
pthread_mutex_t            RWQueueMutex;
std::queue<H264FrameCls *> H264FrameQueue;

/*===============================================================
  forward Functions
  ==============================================================*/

/*===============================================================
  Functions
  ==============================================================*/
/*===============================================================
  Function : createNew
  Purpose : 
  ==============================================================*/
DeviceSource* DeviceSource::createNew(UsageEnvironment& env)
{
  return new DeviceSource(env);
}

/*===============================================================
  Function : DeviceSource constructor
  Purpose : 
  ==============================================================*/
DeviceSource::DeviceSource(UsageEnvironment& env): FramedSource(env)
{
    /* Any global initialization of the device would be done here: */
  if (referenceCount == 0) {
    _L_H264FrameSource = static_cast<DeviceSource *>(this);
    fPresentationTime.tv_sec  = 0;
    fPresentationTime.tv_usec = 0;

    pthread_mutex_init(&RWQueueMutex,NULL);
    
  }
  referenceCount = 1;
    
    /* Create an 'event trigger' for this device (if it hasn't already been done): */
  if (eventTriggerId == 0) {
    eventTriggerId = envir().taskScheduler().createEventTrigger(deliverFrame0);
  }
}

/*===============================================================
  Function : DeviceSource destructor
  Purpose : 
  ==============================================================*/
DeviceSource::~DeviceSource()
{
  --referenceCount;
  if (referenceCount == 0) {
      /* Any global 'destruction' (i.e., resetting) of the device would be done here: */
    pthread_mutex_destroy(&RWQueueMutex);

    while(!H264FrameQueue.empty()) {
      H264FrameCls *H264Frame = H264FrameQueue.front();
      delete H264Frame;
      H264FrameQueue.pop();
    }
    
      /* Reclaim our 'event trigger' */
    envir().taskScheduler().deleteEventTrigger(eventTriggerId);
    eventTriggerId = 0;
  }
}

/*===============================================================
  Function : doGetNextFrame
  Purpose : This function is called (by our 'downstream' object)
             when it asks for new data.
  ==============================================================*/
void DeviceSource::doGetNextFrame()
{
  deliverFrame();
}

/*===============================================================
  Function : deliverFrame0
  Purpose : 
  ==============================================================*/
void DeviceSource::deliverFrame0(void* clientData)
{
  ((DeviceSource*)clientData)->deliverFrame();
}

/*===============================================================
  Function : deliverFrame
  Purpose : This function is called when new frame data is available
             from the device.
  ==============================================================*/
void DeviceSource::deliverFrame()
{
    /*  we're not ready for the data yet */
  if (!isCurrentlyAwaitingData()) {
    return;
  }

    // Deliver the frame 
  pthread_mutex_lock(&RWQueueMutex);

  if(!H264FrameQueue.empty()) {
    H264FrameCls *H264Frame = H264FrameQueue.front();
    
    if (H264Frame->FrameSize > fMaxSize) {
      fFrameSize = fMaxSize;
      fNumTruncatedBytes = H264Frame->FrameSize - fMaxSize;
    }
    else {
      fFrameSize = H264Frame->FrameSize;
    }

    fPresentationTime.tv_sec = H264Frame->FramePTime.tv_sec;
    fPresentationTime.tv_usec = H264Frame->FramePTime.tv_usec;
    
    memmove(fTo, H264Frame->Frame, fFrameSize);
    delete H264Frame;
    H264FrameQueue.pop();
  }
  else {
    fFrameSize = 0;
  }
  
  pthread_mutex_unlock(&RWQueueMutex);  

  // After delivering the data, inform the reader that it is now available:
  if(fFrameSize > 0) {
    FramedSource::afterGetting(this);
  }
}

/*===============================================================
  Function : signalNewFrameDataCPP
  Purpose : The following code would be called to signal that a new frame of data 
  has become available. This (unlike other "LIVE555 Streaming Media" library code) 
  may be called from a separate thread. (Note, however, that "triggerEvent()" cannot 
  be called with the same 'event trigger id' from different threads.
  ==============================================================*/
void DeviceSource::signalNewFrameDataCPP()
{
  envir().taskScheduler().triggerEvent(eventTriggerId, _L_H264FrameSource);
}


extern "C" L_Aaccolade
/*===============================================================
  Function : signalNewFrameData for C prog
  Purpose :
  ==============================================================*/
void signalNewFrameData(unsigned char *Frame,unsigned int FrameSize)
{ 
  if(_L_H264FrameSource != NULL) { // I'm an anal retentive programmer
    
    pthread_mutex_lock(&RWQueueMutex);
    
    while(H264FrameQueue.size() >= QueueCapacity) {
      H264FrameCls *H264Frame = H264FrameQueue.front();
      delete H264Frame;
      H264FrameQueue.pop();
    }
    H264FrameCls *H264Frame = new H264FrameCls();
    
    H264Frame->FrameSize = FrameSize;
    H264Frame->Frame = (u_int8_t *)malloc(H264Frame->FrameSize);
    memcpy(H264Frame->Frame,Frame,H264Frame->FrameSize);

    H264FrameQueue.push(H264Frame);
    
    pthread_mutex_unlock(&RWQueueMutex);
    
    _L_H264FrameSource->signalNewFrameDataCPP();
  }
}
R_Aaccolade



