#ifndef __LOGGING_H__
#define __LOGGING_H__

#include <stdio.h>
#include <stdlib.h>

#include "ConvertString.h"

#define SUCCESS     "SUCCESS"
#define FAIL        "FAIL"

typedef struct _server_flags
{
    int HTTP;               //  -h  proxy a HTTP connection
    int HTTPS;              //  -x  proxy a HTTPS connection
    int RawLog;             //  -l  write raw log file
    int GLLog;              //  -g  write GL feed log
    int replaceHostName;    //  -r  replace hostname in datastream
    int simpleTime;         //  -s  use simple time when logging
    int noFlush;            //  -n  don't flush log files

} SERVER_FLAGS, *PSERVER_FLAGS;

typedef struct _server_args
{
    short nServerPort;
    char* sHostName;
    short nHostPort;
    int nSndBuf;
    int nRcvBuf;

    SERVER_FLAGS flags;

} SERVER_ARGS, *PSERVER_ARGS;

enum 
{
    DataDefault = -1,
    DataIn,
    DataOut,
};

typedef union ULONG_LONG 
{
    struct {
        unsigned short A;
        unsigned short B;
        unsigned short C;
        unsigned short D;
    } us;

    struct {
        unsigned long LowPart;
        unsigned long HighPart;
    } ul;

    long long QuadPart;

} ULONG_LONG;

typedef struct _LogInfo
{
    PSERVER_FLAGS flags;
    FILE * file;
    int direction;
    char* szBuffer;

} LogInfo, *PLogInfo;

long long GetSimplePrecisionTime();
void GetPrecisionTime(int* pDate, int* pTime, int* pMilli, int* pMicro);
void ChangeDateTime(int* iDate, int* iTime, int* isDST, int mday, int mon, int year, int wday, int hour, int min, int sec);
void CalcTime(int SimpleTime, ULONG_LONG* pSimpleTime, char* timeStr, int* pDate, int* pTime, int* pMilli, int* pMicro);

void GetLocalTimeAsString(char* szBuffer);
void DisplayUsage(FILE * file);
void DisplayServerStartUp(PSERVER_ARGS serverArgs, FILE * file);
size_t LogRawDataCallback(char *buffer, size_t size, size_t nitems, void *userp);
void LogRawData(PSERVER_FLAGS flags, char* szData, int nCount, FILE * file, int direction, char* szBuffer);
void LogConnection(PSERVER_FLAGS flags, int IsOpen, char* szRemoteName, char* type, FILE * file);
void WriteGLFeed(int LgMess, char* Mess, FILE * file);

#endif /* __LOGGING_H__ */
