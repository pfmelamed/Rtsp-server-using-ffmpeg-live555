/*===============================================================
  File : Common.h
  Purpose : General include
  PF Melamed     Mar 2014
  ==============================================================*/
#ifndef COMMON_2014_03_H
#define COMMON_2014_03_H

/*===============================================================
  System Includes
  ==============================================================*/
#include <stdio.h>
#include <syslog.h>

/*===============================================================
  Constants
  ==============================================================*/
#define ArtSysPrintf(fmtstr, ...)               \
  syslog(LOG_NOTICE,fmtstr, ##__VA_ARGS__);     \
  printf(fmtstr, ##__VA_ARGS__);                \
  printf("\n")

#define PI 3.14159265

#define L_Aaccolade {
#define R_Aaccolade }

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#define UNKNOWN -1

#define DOUBLEBUFFER 2
#define PIPONGCOUNT  2

#define SMALL_RETRYTIMES 5
#define LARGE_RETRYTIMES 10

#define CPlx 2  
  
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define ON  1
#define OFF 0

#define YES 1
#define NO  0

#define CONNECTED 1
#define DISCONNECTED 0

#define TINY_SIZE    16
#define MINI_SIZE    32
#define NORMAL_SIZE  64
#define LARGE_SIZE   128
#define EXLARGE_SIZE 256
#define HUGE_SIZE    512
#define EXHUGE_SIZE  1024

#define TIMEFORMAT_SIZE 11    /* time format HH:MM:SS:dd */

#define MACADDRFORMAT_SIZE 17  /* mac addr format AA.11.BB.22.CC.33 */

#define FOREVER        1
#define TINYWAITTIME   1  /* in secs */
#define SMALLWAITTIME  2  /* in secs */
#define MEDWAITTIME    5  /* in secs */
#define LARGEWAITTIME  10 /* in secs */

#define EMPTYLINECODE     -6
#define NODATACODE        -5
#define CPINOTALIGNEDCODE -4
#define BADDATACODE       -3
#define TIMEOUTCODE       -2
#define FAILEDCODE        -1
#define SUCCESSCODE        0
#define STOPACTCODE        1

#define MSECINSEC     1000000

#endif
