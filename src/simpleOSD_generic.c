#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>


 
#include "simpleOSD_bs.h"
#include "simpleOSD_generic.h"

char logFile[]="/var/log/simpleOSD.log";
int  GlobalLogLevel=0;
char LOGTYPE[LOGTYPECOUNT][MAXLOGTYPELEN]={"LOG_DEBUG","LOG_INFO","LOG_CRITICAL","LOG_ERROR"};

void  BLOCKFRONT_LOG(int loglevel,char * fmt, ...) {
    if(loglevel>=GlobalLogLevel){
        char fmtString[MAX_MSG_LEN];
        memset(fmtString, 0, MAX_MSG_LEN);
        int close = 1;
        FILE *logFilePtr = NULL;
        int errsv;
        logFilePtr = fopen(logFile, "a+");
        errsv = errno;
        if(!logFilePtr) {
            fprintf(stderr, "Failed to open log file %s %s %d \n", logFile,
                    strerror(errsv), errsv);
            close = 0;
            logFilePtr = stderr;
        }
        char date[200];
        time_t tm = time(NULL);
        int ret = strftime(date, 200, "%a, %d %b %Y %H:%M:%S GMT", gmtime(&tm));
        if(!ret) {
            memset(date, 0, 200);
        }
        va_list fmtArgs;
        va_start(fmtArgs, fmt);
        snprintf(fmtString, sizeof(fmtString), "%s:%s :  :Error : %s\n",
                LOGTYPE[(loglevel>LOG_ERROR)?LOG_ERROR:loglevel],date, fmt);
            vfprintf(logFilePtr, fmtString, fmtArgs);
        va_end(fmtArgs);
        if(close) {
            fclose(logFilePtr);
        }	
    }
}


struct timeval 
time_diff(struct timeval time_from, struct timeval time_to)
{

  struct timeval result;

  if(time_to.tv_usec < time_from.tv_usec)
    {
      result.tv_sec = time_to.tv_sec - time_from.tv_sec - 1;
      result.tv_usec = 1000000 + time_to.tv_usec - time_from.tv_usec;
    }
  else
    {
      result.tv_sec = time_to.tv_sec - time_from.tv_sec;
      result.tv_usec = time_to.tv_usec - time_from.tv_usec;
    }

  return result;
}

