/*****************************************************************************
*
* This example source code introduces a c library buffered I/O interface to
* URL reads it supports fopen(), fread(), fgets(), feof(), fclose().
* Supported functions have identical prototypes to their normal c
* lib namesakes and are preceaded by url_ .
*
* Using this code you can replace your program's fopen() with url_open()
* and fread() with url_read() and it become possible to read remote streams
* instead of (only) local files. Local files (ie those that can be directly
* fopened) will drop back to using the underlying clib implementations
*
* Coyright (c)2003 Simtec Electronics
*
* Re-implemented by Vincent Sanders <vince@kyllikki.org> with extensive
* reference to original curl example code
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
* 3. The name of the author may not be used to endorse or promote products
*    derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* This example requires libcurl 7.9.7 or later.
*/

#include <stdio.h>
#include <string.h>
#ifndef WIN32
#  include <sys/time.h>
#endif
#include <stdlib.h>
#include <errno.h>

#include <curl/curl.h>

enum fcurl_type_e { CFTYPE_NONE=0, CFTYPE_FILE=1, CFTYPE_CURL=2, CFTYPE_HTTPS=3 };

struct fcurl_data
{
    enum fcurl_type_e type;     /* type of handle */
    union {
        CURL *curl;
        FILE *file;
    } handle;                   /* handle */

    char *buffer;               /* buffer to store cached data*/
    size_t buffer_len;             /* currently allocated buffers length */
    size_t buffer_pos;             /* end of data in buffer*/
    int still_running;          /* Is background url fetch still in progress */
};

typedef struct fcurl_data URL_FILE;

/* exported functions */
URL_FILE *url_open(const char *url,const char *operation, const char* SSLClientCert);
int url_close(URL_FILE *file);
int url_eof(URL_FILE *file);

size_t url_read(void *ptr, size_t size, size_t nmemb, URL_FILE *file, int waittime);
char * url_gets(char *ptr, int size, URL_FILE *file, int waittime);

/* we use a global one for convenience */
CURLM *multi_handle;

/* curl calls this routine to get more data */
static size_t
write_callback(char *buffer,
               size_t size,
               size_t nitems,
               void *userp)
{
    char *newbuff;
    size_t rembuff;

    URL_FILE *url = (URL_FILE *)userp;
    size *= nitems;

    rembuff=url->buffer_len - url->buffer_pos; /* remaining space in buffer */

    if(size > rembuff)
    {
        /* not enough space in buffer */
        newbuff=realloc(url->buffer,url->buffer_len + (size - rembuff));
        if(newbuff==NULL)
        {
            fprintf(stderr,"callback buffer grow failed\n");
            size=rembuff;
        }
        else
        {
            /* realloc suceeded increase buffer size*/
            url->buffer_len+=size - rembuff;
            url->buffer=newbuff;
        }
    }

    memcpy(&url->buffer[url->buffer_pos], buffer, size);
    url->buffer_pos += size;

    return size;
}

typedef struct _MemoryBuffer
{
    int head;
    char *buffer;               /* buffer to store cached data*/
    size_t buffer_len;             /* currently allocated buffers length */
    size_t buffer_pos;             /* end of data in buffer*/

} MemoryBuffer, *PMemoryBuffer;

static size_t pipe_data_to_memory(char *buffer, size_t size, size_t nitems, void *userp)
{
    char *newbuff;
    size_t rembuff;

    PMemoryBuffer mem = (PMemoryBuffer)userp;
    size *= nitems;

    if(!buffer || !size || !userp) return 0;

    if(mem->head)
    {
        char* ptr = memchr(buffer, '\n', size);
        if(ptr)
        {
            ptr[0] = 0;
            if(strstr(buffer, "Transfer-Encoding: chunked\r"))
            {
                return size;
            }
            ptr[0] = '\n';
        }
    }

    rembuff=mem->buffer_len - mem->buffer_pos; /* remaining space in buffer */

    if(size > rembuff)
    {
        /* not enough space in buffer */
        newbuff=realloc(mem->buffer,mem->buffer_len + (size - rembuff));
        if(newbuff==NULL)
        {
            fprintf(stderr,"callback buffer grow failed\n");
            size=rembuff;
        }
        else
        {
            /* realloc suceeded increase buffer size*/
            mem->buffer_len+=size - rembuff;
            mem->buffer=newbuff;
        }
    }

    memcpy(&mem->buffer[mem->buffer_pos], buffer, size);
    mem->buffer_pos += size;

    return size;
}

typedef struct _SocketInfo
{
    int head;
    SOCKET socket;
    curl_write_callback onData;
    void* userData;

} SocketInfo, *PSocketInfo;

static size_t pipe_data_to_socket(char *buffer, size_t size, size_t nitems, void *userp)
{
    PSocketInfo pSock = (PSocketInfo)userp;

    int nRet = 0;
    int byteOffset = 0;
    int bytesLeft = (int)(size * nitems);
    char* bytes = buffer;

    if(!buffer || !bytes || !userp) return 0;

    if(pSock->head)
    {
        char* ptr = memchr(buffer, '\n', bytesLeft);
        if(ptr)
        {
            ptr[0] = 0;
            if(strstr(buffer, "Transfer-Encoding: chunked\r"))
            {
                return bytesLeft;
            }
            ptr[0] = '\n';
        }
    }

    if(pSock->onData)
    {
        (*pSock->onData)(buffer, size, nitems, pSock->userData);
    }

    while(bytesLeft)
    {
        nRet = send(pSock->socket, &bytes[byteOffset], bytesLeft, 0);
        if(nRet <= 0) break;
        byteOffset += nRet;
        bytesLeft -= nRet;
    }

    return byteOffset;
}

/* use to attempt to fill the read buffer up to requested number of bytes */
static int
fill_buffer(URL_FILE *file,size_t want,int waittime)
{
    fd_set fdread;
    fd_set fdwrite;
    fd_set fdexcep;
    int maxfd;
    struct timeval timeout;
    int rc;

    /* only attempt to fill buffer if transactions still running and buffer
    * doesnt exceed required size already
    */
    if((!file->still_running) || (file->buffer_pos > want))
        return 0;

    /* attempt to fill buffer */
    do
    {
        FD_ZERO(&fdread);
        FD_ZERO(&fdwrite);
        FD_ZERO(&fdexcep);

        /* set a suitable timeout to fail on */
        timeout.tv_sec = waittime;
        timeout.tv_usec = 0;

        /* get file descriptors from the transfers */
        curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);

        /* In a real-world program you OF COURSE check the return code of the
        function calls, *and* you make sure that maxfd is bigger than -1
        so that the call to select() below makes sense! */

        rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);

        switch(rc) {
    case -1:
        /* select error */
        break;

    case 0:
        break;

    default:
        /* timeout or readable/writable sockets */
        /* note we *could* be more efficient and not wait for
        * CURLM_CALL_MULTI_PERFORM to clear here and check it on re-entry
        * but that gets messy */
        while(curl_multi_perform(multi_handle, &file->still_running) ==
            CURLM_CALL_MULTI_PERFORM);

        break;
        }
    } while(file->still_running && (file->buffer_pos < want));
    return 1;
}

/* use to remove want bytes from the front of a files buffer */
static int
use_buffer(URL_FILE *file,size_t want)
{
    /* sort out buffer */
    if((file->buffer_pos - want) <=0)
    {
        /* ditch buffer - write will recreate */
        if(file->buffer)
            free(file->buffer);

        file->buffer=NULL;
        file->buffer_pos=0;
        file->buffer_len=0;
    }
    else
    {
        /* move rest down make it available for later */
        memmove(file->buffer,
            &file->buffer[want],
            (file->buffer_pos - want));

        file->buffer_pos -= want;
    }
    return 0;
}

CURLcode url_to_memory(const char *url, char** pHeadBuffer, size_t* pHeadSize, char** pBodyBuffer, size_t* pBodySize)
{
    CURLcode retCode = CURLE_FAILED_INIT;
    CURL* curl = curl_easy_init();

    const char* SSL_CLIENT_CERT = getenv("SSL_CLIENT_CERT");
    if(!SSL_CLIENT_CERT) SSL_CLIENT_CERT = "cert.pem";

    if(curl) 
    {
        MemoryBuffer headBuff = {1, NULL, 0, 0};
        MemoryBuffer bodyBuff = {0, NULL, 0, 0};

        curl_easy_setopt(curl, CURLOPT_URL, url);

        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headBuff);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, pipe_data_to_memory);

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &bodyBuff);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, pipe_data_to_memory);

        curl_easy_setopt(curl,CURLOPT_SSLCERTTYPE,"PEM");
        curl_easy_setopt(curl,CURLOPT_SSLCERT,SSL_CLIENT_CERT);
        curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,FALSE);
        curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,1);

        retCode = curl_easy_perform(curl);

        (*pHeadSize) = headBuff.buffer_pos;
        (*pHeadBuffer) = headBuff.buffer;

        (*pBodySize) = bodyBuff.buffer_pos;
        (*pBodyBuffer) = bodyBuff.buffer;

        curl_easy_cleanup(curl);
    }

    return retCode;
}

CURLcode url_to_socket(const char *url, SOCKET pipe, curl_write_callback onData, void* userData)
{
    CURLcode retCode = CURLE_FAILED_INIT;
    CURL* curl = curl_easy_init();

    const char* SSL_CLIENT_CERT = getenv("SSL_CLIENT_CERT");
    if(!SSL_CLIENT_CERT) SSL_CLIENT_CERT = "cert.pem";

    if(curl) 
    {
        SocketInfo headInfo = {1, pipe, onData, userData};
        SocketInfo bodyInfo = {0, pipe, onData, userData};

        curl_easy_setopt(curl, CURLOPT_URL, url);

        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headInfo);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, pipe_data_to_socket);

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &bodyInfo);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, pipe_data_to_socket);

        curl_easy_setopt(curl,CURLOPT_SSLCERTTYPE,"PEM");
        curl_easy_setopt(curl,CURLOPT_SSLCERT,SSL_CLIENT_CERT);
        curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,FALSE);
        curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,1);

        retCode = curl_easy_perform(curl);

        curl_easy_cleanup(curl);
    }

    return retCode;
}

URL_FILE * url_open(const char *url,const char *operation, const char* SSLClientCert)
{
    /* this code could check for URLs or types in the 'url' and
    basicly use the real fopen() for standard files */

    URL_FILE *file;
    (void)operation;

    file = malloc(sizeof(URL_FILE));
    if(!file)
        return NULL;

    memset(file, 0, sizeof(URL_FILE));

    if((file->handle.file=fopen(url,operation)))
    {
        file->type = CFTYPE_FILE; /* marked as URL */
    }
    else
    {
        file->type = CFTYPE_CURL; /* marked as URL */
        file->handle.curl = curl_easy_init();

        if((url[0] == 'H' || url[0] == 'h') &&
            (url[1] == 'T' || url[1] == 't') &&
            (url[2] == 'T' || url[2] == 't') &&
            (url[3] == 'P' || url[3] == 'p') &&
            (url[4] == 'S' || url[4] == 's') &&
            (url[5] == ':'))
        {
            if(!SSLClientCert) SSLClientCert = getenv("SSL_CLIENT_CERT");
            if(!SSLClientCert) SSLClientCert = "cert.pem";

            file->type = CFTYPE_HTTPS;  /* marked as HTTPS */

            curl_easy_setopt(file->handle.curl,CURLOPT_SSLCERTTYPE,"PEM");
            curl_easy_setopt(file->handle.curl,CURLOPT_SSLCERT,SSLClientCert);
            curl_easy_setopt(file->handle.curl,CURLOPT_SSL_VERIFYPEER,FALSE);
            curl_easy_setopt(file->handle.curl,CURLOPT_SSL_VERIFYHOST,1);
        }

        curl_easy_setopt(file->handle.curl, CURLOPT_URL, url);
        curl_easy_setopt(file->handle.curl, CURLOPT_WRITEDATA, file);
        curl_easy_setopt(file->handle.curl, CURLOPT_VERBOSE, 0L);
        curl_easy_setopt(file->handle.curl, CURLOPT_WRITEFUNCTION, write_callback);

        if(!multi_handle)
            multi_handle = curl_multi_init();

        curl_multi_add_handle(multi_handle, file->handle.curl);

        /* lets start the fetch */
        while(curl_multi_perform(multi_handle, &file->still_running) ==
            CURLM_CALL_MULTI_PERFORM );

        if((file->buffer_pos == 0) && (!file->still_running))
        {
            /* if still_running is 0 now, we should return NULL */

            /* make sure the easy handle is not in the multi handle anymore */
            curl_multi_remove_handle(multi_handle, file->handle.curl);

            /* cleanup */
            curl_easy_cleanup(file->handle.curl);

            free(file);

            file = NULL;
        }
    }
    return file;
}

int
url_close(URL_FILE *file)
{
    int ret=0;/* default is good return */

    switch(file->type)
    {
    case CFTYPE_FILE:
        ret=fclose(file->handle.file); /* passthrough */
        break;

    case CFTYPE_CURL:
    case CFTYPE_HTTPS:
        /* make sure the easy handle is not in the multi handle anymore */
        curl_multi_remove_handle(multi_handle, file->handle.curl);

        /* cleanup */
        curl_easy_cleanup(file->handle.curl);
        break;

    default: /* unknown or supported type - oh dear */
        ret=EOF;
        errno=EBADF;
        break;

    }

    if(file->buffer)
        free(file->buffer);/* free any allocated buffer space */

    free(file);

    return ret;
}

int
url_eof(URL_FILE *file)
{
    int ret=0;

    switch(file->type)
    {
    case CFTYPE_FILE:
        ret=feof(file->handle.file);
        break;

    case CFTYPE_CURL:
    case CFTYPE_HTTPS:
        if((file->buffer_pos == 0) && (!file->still_running))
            ret = 1;
        break;
    default: /* unknown or supported type - oh dear */
        ret=-1;
        errno=EBADF;
        break;
    }
    return ret;
}

size_t
url_read(void *ptr, size_t size, size_t nmemb, URL_FILE *file, int waittime)
{
    size_t want;

    switch(file->type)
    {
    case CFTYPE_FILE:
        want=fread(ptr,size,nmemb,file->handle.file);
        break;

    case CFTYPE_CURL:
    case CFTYPE_HTTPS:
        want = nmemb * size;

        fill_buffer(file,want,waittime);

        /* check if theres data in the buffer - if not fill_buffer()
        * either errored or EOF */
        if(!file->buffer_pos)
            return 0;

        /* ensure only available data is considered */
        if(file->buffer_pos < want)
            want = file->buffer_pos;

        /* xfer data to caller */
        memcpy(ptr, file->buffer, want);

        use_buffer(file,want);

        want = want / size;     /* number of items - nb correct op - checked
                                * with glibc code*/
        break;

    default: /* unknown or supported type - oh dear */
        want=0;
        errno=EBADF;
        break;

    }
    return want;
}

char *
url_gets(char *ptr, int size, URL_FILE *file, int waittime)
{
    size_t want = size - 1;/* always need to leave room for zero termination */
    size_t loop;

    switch(file->type)
    {
    case CFTYPE_FILE:
        ptr = fgets(ptr,size,file->handle.file);
        break;

    case CFTYPE_CURL:
    case CFTYPE_HTTPS:
        fill_buffer(file,want,waittime);

        /* check if theres data in the buffer - if not fill either errored or
        * EOF */
        if(!file->buffer_pos)
            return NULL;

        /* ensure only available data is considered */
        if(file->buffer_pos < want)
            want = file->buffer_pos;

        /*buffer contains data */
        /* look for newline or eof */
        for(loop=0;loop < want;loop++)
        {
            if(file->buffer[loop] == '\n')
            {
                want=loop+1;/* include newline */
                break;
            }
        }

        /* xfer data to caller */
        memcpy(ptr, file->buffer, want);
        ptr[want]=0;/* allways null terminate */

        use_buffer(file,want);

        /*printf("(fgets) return %d bytes %d left\n", want,file->buffer_pos);*/
        break;

    default: /* unknown or supported type - oh dear */
        ptr=NULL;
        errno=EBADF;
        break;
    }

    return ptr;/*success */
}
