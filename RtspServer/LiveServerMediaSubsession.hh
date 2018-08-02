/*===============================================================
  File Name:     LiveServerMediaSubsession.hh
  Purpose:       
  PF Melamed     Jul 2018
  ==============================================================*/
#ifndef LIVESERVERMEDIASUBSESSION_2018_07_HH
#define LIVESERVERMEDIASUBSESSION_2018_07_HH

/*===============================================================
  System Includes
  ==============================================================*/
#include <OnDemandServerMediaSubsession.hh>
#include <StreamReplicator.hh>
#include <H264VideoRTPSink.hh>
#include <H264VideoStreamFramer.hh>
#include <H264VideoStreamDiscreteFramer.hh>
#include <UsageEnvironment.hh>
#include <Groupsock.hh>

/*===============================================================
  User Includes
  ==============================================================*/
#include "../CityImg/Common.h"

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
  forward Functions
  ==============================================================*/

/*===============================================================
  Class
  ==============================================================*/
class LiveServerMediaSubsession: public OnDemandServerMediaSubsession
{
public:
  static LiveServerMediaSubsession* createNew(UsageEnvironment& env, StreamReplicator* replicator);
    
protected:
  StreamReplicator * m_replicator;  

  LiveServerMediaSubsession(UsageEnvironment& env, StreamReplicator* replicator)
      : OnDemandServerMediaSubsession(env, False), m_replicator(replicator) {};
  
  virtual FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate);
  virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,  unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource);
};

#endif
