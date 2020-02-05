#include <string.h> 
#include <signal.h> 
#include <fcntl.h>

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <io.h>
#include <windows.h>
typedef int socklen_t;
#else
#define SOCKET int
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#endif

#include <pipe.H> 

int disable_nagle(int fd)
{
    int ret;
    const int Delay = 1;
    ret = setsockopt( fd, IPPROTO_TCP , TCP_NODELAY, (const char *)&Delay, sizeof(Delay)); 
    return (ret);
}

int pipe_non_block(int fd)
{
    int current_mode = 0;

#ifdef PIPE_SOCKET
    disable_nagle(fd);
#endif

#ifdef WIN32
    unsigned long opt = 1;
    return ioctlsocket (fd, FIONBIO, &opt );
#else
    current_mode = fcntl ( fd, F_GETFL, 0 );
    return fcntl (fd, F_SETFL, O_NONBLOCK | current_mode);
#endif
}

int pipe_non_block(int handles[2], int count)
{
    for(int idx = 0; idx < count; idx++) 
    {
        if(pipe_non_block(handles[idx]) == -1) return -1;
    }
    return 0;
}

void pipe_close(int handles[2], int count)
{
    for(int idx = 0; idx < count; idx++) 
    {
        if(handles[idx] != -1)
        {
#ifdef WIN32
            shutdown(handles[idx], SD_BOTH);
            closesocket(handles[idx]);
#else
            shutdown(handles[idx], SHUT_RDWR);
            close(handles[idx]);
#endif
            handles[idx] = -1;
        }
    }
}

#ifdef PIPE_SOCKET
int pipe_create(int handles[2])
{
    static const int bufSize = 1024*1024;
    SOCKET s;
    struct sockaddr_in serv_addr;
    socklen_t len = sizeof(serv_addr);

    handles[0] = handles[1] = -1;

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        return -1;
    }

    if(bufSize > 0)
    {
        setsockopt(s, SOL_SOCKET, SO_SNDBUF, (char *)&bufSize, sizeof(bufSize));
        setsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *)&bufSize, sizeof(bufSize));
    }

    memset((void *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(0);
    serv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(s, (struct sockaddr*)&serv_addr, len) == -1)
    {
        pipe_close((int*)&s, 1);
        return -1;
    }
    if (listen(s, 1) == -1)
    {
        pipe_close((int*)&s, 1);
        return -1;
    }
    if (getsockname(s, (struct sockaddr*)&serv_addr, &len) == -1)
    {
        pipe_close((int*)&s, 1);
        return -1;
    }
    if ((handles[1] = (int)socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        pipe_close((int*)&s, 1);
        return -1;
    }

    if(bufSize > 0)
    {
        setsockopt(handles[1], SOL_SOCKET, SO_SNDBUF, (char *)&bufSize, sizeof(bufSize));
        setsockopt(handles[1], SOL_SOCKET, SO_RCVBUF, (char *)&bufSize, sizeof(bufSize));
    }

    if (connect(handles[1], (struct sockaddr*)& serv_addr, len) == -1)
    {
        pipe_close((int*)&s, 1);
        return -1;
    }
    if ((handles[0] = (int)accept(s, (struct sockaddr*)&serv_addr, &len)) == -1)
    {
        pipe_close(&handles[1], 1);
        handles[1] = -1;
        pipe_close((int*)&s, 1);
        return -1;
    }
    pipe_close((int*)&s, 1);

    return 0;
}

int pipe_write(int s, const char *buf, int len)
{
    return send(s,buf,len,0);
}

int pipe_read(int s, char *buf, int len)
{
    int ret = recv(s, buf, len, 0);

#ifdef WIN32
    if (ret < 0 && WSAGetLastError() == WSAECONNRESET)
    {
        /* EOF on the pipe! (win32 socket based implementation) */
        ret = 0;
    }
#endif

    return ret;
}
#else

int pipe_create(int handles[2])
{
    return pipe(handles);
}

int pipe_read(int s, char *buf, int len)
{
    return read(s,buf,len);
}

int pipe_write(int s, const char *buf, int len)
{
    return write(s,buf,len);
}

#endif
