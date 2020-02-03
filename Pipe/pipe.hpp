#ifndef _PIPE_HPP_
#define _PIPE_HPP_

#define PIPE_SOCKET 1

#ifdef PIPE_SOCKET

int pipe_create(int handles[2]);
int pipe_read(int s, char *buf, int len);
int pipe_write(int s, const char *buf, int len);

#else 

int pipe_create(int handles[2]);
int pipe_read(int s, char *buf, int len);
int pipe_write(int s, const char *buf, int len);

#endif

int pipe_non_block(int handles[2], int count = 2);
void pipe_close(int handles[2], int count = 2);

#endif //_PIPE_HPP_
