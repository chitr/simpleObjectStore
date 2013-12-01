#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef ERROR
#define ERROR -1
#endif


#define COMPONENT_FSAL LOG_CRITICAL
#define LogCrit        BLOCKFRONT_LOG
#define LogDebug        BLOCKFRONT_LOG
#define LogInfo        BLOCKFRONT_LOG



#ifndef COMPONENT_SOCKET
#define COMPONENT_SOCKET COMPONENT_FSAL
#endif
typedef enum SOCKET_STATUS {
SOC_SUCCESS=0,
SOC_TIMEOUT,
SOC_ERROR
}SOCKET_STATUS_t;
#ifndef RESERVED_FD_COUNT 
#define RESERVED_FD_COUNT 3
#endif
/******* EPOLL  data **********************************************************/
#ifndef EPOLL_ARG1
#define EPOLL_ARG1 1
#endif
#ifndef MAX_EPOLL_EVENT
#define MAX_EPOLL_EVENT 1
#endif
#ifndef SEC2MSEC
#define SEC2MSEC 1000 /*convert from sec to msec*/
#endif
#ifndef ERR_RESET
#define ERR_RESET  \
	errno=0;
#endif

