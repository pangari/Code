#ifndef __LOGGING_H__
#define __LOGGING_H__

#include <stdio.h>
#include <stdlib.h>

#include "ConvertString.h"
#include "crypt.h"

#define SUCCESS		"SUCCESS"
#define FAIL		"FAIL"

typedef struct _server_flags
{
    int RawLog;			// --rawLog
    int GLLog;			// --GLLog
    int ProxyServer;	// --ProxyServer
    int ProxyClient;	// --ProxyClient
    int LZO;			// --lzo
    int FISH;			// --fish
    int ICE;			// --ice
    int TEA;			// --tea
    int Statistics;		// --statistics

} SERVER_FLAGS, *PSERVER_FLAGS;

typedef struct _server_stats
{
    long long StTime;

    long long LzoIn;
    long long LzoOut;

    long long FishIn;
    long long FishOut;

    long long IceIn;
    long long IceOut;

    long long TeaIn;
    long long TeaOut;

    unsigned int RecvBytes;
    unsigned int SendBytes;

} SERVER_STATS, *PSERVER_STATS;

typedef struct _encrypt_key
{
    int                 flag;
    char                key[1024+1];
    INITIAL_VECTOR      iv;
    void*               ctx;
} ENCRYPT_KEY, *PENCRYPT_KEY;

typedef struct _server_keys
{
    char ServerKey [1024+1];

    ENCRYPT_KEY Fish;
    ENCRYPT_KEY Ice;
    ENCRYPT_KEY Tea;

} SERVER_KEYS, *PSERVER_KEYS;

void GetLocalTimeAsString(char* szBuffer);
void DisplayUsage(FILE * file);
void DisplayDetailedUsage(FILE * file);
void DisplayServerStartUp(short nServerPort, char* nHostName, short nHostPort, char* nProxyName, short nProxyPort, PSERVER_FLAGS flags, PSERVER_KEYS keys, FILE * file);
void LogRawData(char* szData, int nCount, FILE * file, char* szBuffer);
void LogConnection(int IsOpen, char* szRemoteName, char* type, int id, int total, FILE * file);
void LogProxy(char* szRemoteName, char* message, char* hostName, int hostPort, int id, FILE * file);
void WriteGLFeed(int LgMess, char* Mess, FILE * file);

void LogSocketStatistics(char* szRemoteName, PSERVER_STATS stats, int id, FILE * file);
void LogLzoStatistics(char* szRemoteName, PSERVER_STATS stats, int id, FILE * file);
void LogFishStatistics(char* szRemoteName, PSERVER_STATS stats, int id, FILE * file);
void LogIceStatistics(char* szRemoteName, PSERVER_STATS stats, int id, FILE * file);
void LogTeaStatistics(char* szRemoteName, PSERVER_STATS stats, int id, FILE * file);

long long GetSimplePrecisionTime();
long GetElapsedTime();

#endif /* __LOGGING_H__ */
