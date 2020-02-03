#ifndef __LOGGING_H__
#define __LOGGING_H__

#include <stdio.h>
#include <stdlib.h>

#include "ConvertString.h"

#define SUCCESS     "SUCCESS"
#define FAIL        "FAIL"

typedef struct _server_flags
{
    int RawLog;         // --rawLog
    int ScreenLog;      // --screenLog
    int Base64;         // --base64
    int ClientMode;     // --client

} SERVER_FLAGS, *PSERVER_FLAGS;

void GetLocalTimeAsString(char* szBuffer);
void DisplayUsage(FILE * file);
void DisplayDetailedUsage(FILE * file);
void DisplayServerStartUp(short nServerPort, PSERVER_FLAGS flags, FILE * file);
void LogRawData(char* szData, int nCount, FILE * file, char* szBuffer);
void LogToScreen(char* szData, int nCount, FILE * file, char* szBuffer);
void LogToScreenBase64(char* szData, int nCount, FILE * file, char* szBuffer);
void LogConnection(int IsOpen, char* szRemoteName, char* type, int id, int total, FILE * file);
void LogProxy(char* szRemoteName, char* message, char* hostName, int hostPort, int id, FILE * file);
void WriteGLFeed(int LgMess, char* Mess, FILE * file);

#endif /* __LOGGING_H__ */
