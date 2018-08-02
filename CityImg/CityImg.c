/*===============================================================
  File Name:     CityImg.c
  Purpose:       Main function
  PF Melamed    
  ==============================================================*/
/*===============================================================
  System Includes
  ==============================================================*/ 
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/*===============================================================
  User Includes
  ==============================================================*/
#include "../CityImg/Common.h"
#include "../RtspServer/FM_ImgGrabber_Thread.h"
#include "../RtspServer/RtspServer.h"

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
  main
  ==============================================================*/
int main(int argc, char *argv[]) 
{
  pthread_t ThPid; /* Thread pid */
  
  int RetCode;
  
  Init_RtspServer();              /* Init the Rtsp (live555) lib */

  RetCode = pthread_create(&ThPid, NULL,FM_ImgGrabber_Thread, NULL);
  if(RetCode != 0) {
    printf("[%s %d] pthread create error thread\n",__FILE__,__LINE__);
    exit(FAILEDCODE);
  }

  RetCode = pthread_join(ThPid, NULL);
  if (RetCode != 0) {
    printf("[%s,%d] pthread join error %d\n",__FUNCTION__,__LINE__,RetCode);
    exit(FAILEDCODE);
  }
   
    /* clen up */
  Deinit_RtspServer();
  
  exit(SUCCESSCODE);
}
