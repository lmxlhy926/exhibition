#ifndef MGCORE_LOGTOOL_H
#define MGCORE_LOGTOOL_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>  
#include <sys/ioctl.h>
#include <sys/wait.h>  
#include <arpa/inet.h>
//#include <linux/types.h>
//#include <linux/netlink.h>
//#include <linux/if_ether.h>
#include <pthread.h>
#include <signal.h>
#include <ctype.h>  
#include <errno.h>  
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <dirent.h>
#include <sys/time.h>
#include <semaphore.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_OPEN	1
#define _DEBUG      1
#define sleep_ms(x)                     usleep(x * 1000)
#define sleep_s(s)						sleep(s)
#define sleep_us(us)					usleep(us)

	/*log function define*/
#if LOG_OPEN

#ifdef __ANDROID_SYSTEM__
#include <android/log.h>
#define LOG_TAG "COSS_TAG"
#define ipp_LogV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#define ipp_LogD(...) __android_log_print(ANDROID_LOG_DEBUG,   LOG_TAG, __VA_ARGS__)
#define ipp_LogI(...) __android_log_print(ANDROID_LOG_INFO,    LOG_TAG, __VA_ARGS__)
#define ipp_LogW(...) __android_log_print(ANDROID_LOG_WARN,    LOG_TAG, __VA_ARGS__)
#define ipp_LogE(...) __android_log_print(ANDROID_LOG_ERROR,   LOG_TAG, __VA_ARGS__)

#define ipp_LogV_Prefix ipp_LogV
#define ipp_LogD_Prefix ipp_LogD
#define ipp_LogI_Prefix ipp_LogI
#define ipp_LogW_Prefix ipp_LogW
#define ipp_LogE_Prefix ipp_LogE

#else
#define ipp_LogV_Prefix(__fmt__, ...) do{printf("[IPP]:" __fmt__, ##__VA_ARGS__);}while(0)
#define ipp_LogD_Prefix(__fmt__, ...) do{printf("[IPP_DEBUG]:" __fmt__, ##__VA_ARGS__);}while(0)
#define ipp_LogI_Prefix(__fmt__, ...) do{printf("[IPP_INFO]:" __fmt__, ##__VA_ARGS__);}while(0)
#define ipp_LogW_Prefix(__fmt__, ...) do{printf("[IPP_WARNING]:" __fmt__, ##__VA_ARGS__);}while(0)
#define ipp_LogE_Prefix(__fmt__, ...) do{printf("[IPP_ERROR]:" __fmt__, ##__VA_ARGS__);}while(0)

#define ipp_LogV(__fmt__, ...) do{printf(__fmt__, ##__VA_ARGS__);}while(0)
#define ipp_LogD(__fmt__, ...) do{printf(__fmt__, ##__VA_ARGS__);}while(0)
#define ipp_LogI(__fmt__, ...) do{printf(__fmt__, ##__VA_ARGS__);}while(0)
#define ipp_LogW(__fmt__, ...) do{printf(__fmt__, ##__VA_ARGS__);}while(0)
#define ipp_LogE(__fmt__, ...) do{printf(__fmt__, ##__VA_ARGS__);}while(0)
#endif

#else
#define ipp_LogV_Prefix(__fmt__, ...)
#define ipp_LogD_Prefix(__fmt__, ...)
#define ipp_LogI_Prefix(__fmt__, ...)
#define ipp_LogW_Prefix(__fmt__, ...)
#define ipp_LogE_Prefix(__fmt__, ...)

#define ipp_LogV(__fmt__, ...)
#define ipp_LogD(__fmt__, ...)
#define ipp_LogI(__fmt__, ...)
#define ipp_LogW(__fmt__, ...)
#define ipp_LogE(__fmt__, ...)
#endif
/*log function define end*/


#ifdef __cplusplus
}
#endif

#endif/*#ifndef MGCORE_LOGTOOL_H*/

