#include "GetPasswd.h"
#include "ConvertString.h"

#ifdef WIN32
#include <windows.h>
#elif VMS
#include <unistd.h>
#include <descrip.h>
#include <iodef.h>
#include <ssdef.h>
#include <starlet.h>
#include <string.h>
#include <stsdef.h>
#include <ttdef.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

#include <stdio.h>

#ifdef VMS

int readnoecho(int len,char *buffer)
{
    struct dsc$descriptor_s tt_desc;
    char *terminal = "TT";
    long fonction;
    int status;
    unsigned short channel;
    unsigned short iosb[4];
    tt_desc.dsc$w_length = strlen(terminal);
    tt_desc.dsc$a_pointer = terminal;
    tt_desc.dsc$b_class = DSC$K_CLASS_S;
    tt_desc.dsc$b_dtype = DSC$K_DTYPE_T;

    status = sys$assign(&tt_desc,&channel,0,0);
    if(!(status & STS$M_SUCCESS))
    {
        return(-1);
    }
    fonction = IO$_READVBLK | IO$M_NOECHO;
    status = sys$qiow(0,channel,fonction,iosb,0,0,buffer,len - 1,0,0,0,0);
    if(!(status & STS$M_SUCCESS))
    {
        return(-1);
    }
    sys$dassgn(channel);
    if(!(iosb[0] & STS$M_SUCCESS))
    {
        return(-1);
    }
    buffer[iosb[1]] = 0;
    return((int)iosb[1]);
}

int GetPasswd(char _message[], char _buffer[], int _size)
{
    int len = 0;
    int idx = 0;
    int inPos = 0;
    int outPos = 0;

    memset(_buffer, 0, _size);

    fputs(_message, stderr);
    readnoecho(100, _buffer);

    len = (int)strlen(_buffer);

    if((len > 0) && (_buffer[len-1] == '\n')) 
    {
        _buffer[len-1] = 0;
        len--;
    }

    while(inPos < len)
    {
        _buffer[outPos++] = DecodeCString(&_buffer[inPos], &idx);
        inPos += idx;
    }

    return len;
}

#elif WIN32

// prevoir un buffer raisonnable
int GetPasswd(char _message[], char _buffer[], int _size)
{
    int len = 0;
    int idx = 0;
    int inPos = 0;
    int outPos = 0;

    DWORD fdwMode, fdwOldMode;
    HANDLE hStdin;

    memset(_buffer, 0, _size);

    // Get STDIN win32 handle
    hStdin = GetStdHandle(STD_INPUT_HANDLE);

    // Turn off console echo mode
    GetConsoleMode(hStdin, &fdwOldMode);  
    fdwMode = fdwOldMode & ~ENABLE_ECHO_INPUT; 
    SetConsoleMode(hStdin, fdwMode);

    // Retreive unencrypted password
    if(_message[0] == '\n')
        fputs(&_message[1], stderr);
    else
        fputs(_message, stderr);
    fgets(_buffer, _size-1, stdin);

    // Restore original console mode
    SetConsoleMode(hStdin, fdwOldMode);

    len = (int)strlen(_buffer);

    if((len > 0) && (_buffer[len-1] == '\n')) 
    {
        _buffer[len-1] = 0;
        len--;
        fputs("\n", stderr);
    }

    while(inPos < len)
    {
        _buffer[outPos++] = DecodeCString(&_buffer[inPos], &idx);
        inPos += idx;
    }

    return len;
}

#else

int GetPasswd(char _message[], char _buffer[], int _size)
{
    int len = 0;
    int idx = 0;
    int inPos = 0;
    int outPos = 0;

    struct termios orig, changes;

    memset(_buffer, 0, _size);

    tcgetattr(STDIN_FILENO, &orig);

    /* turn tty echo mode off */
    changes = orig;
    changes.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSADRAIN, &changes);

    fputs(_message, stderr);
    fgets(_buffer, _size-1, stdin);

    /* restore old tty settings */
    tcsetattr(STDIN_FILENO, TCSADRAIN, &orig);

    len = (int)strlen(_buffer);

    if((len > 0) && (_buffer[len-1] == '\n')) 
    {
        _buffer[len-1] = 0;
        len--;
    }

    while(inPos < len)
    {
        _buffer[outPos++] = DecodeCString(&_buffer[inPos], &idx);
        inPos += idx;
    }

    return len;
}

#endif
