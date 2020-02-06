#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <dlfcn.h>

#ifndef RTLD_NEXT
#define RTLD_NEXT ((void *) -1L)
#endif

#define REAL_LIBC RTLD_NEXT

int (*real_sigaction)(int, const struct sigaction *, struct sigaction *) = 0;

static void nop(int signum __attribute__((unused))) {}

static void init (void) __attribute__ ((constructor));
static void init (void)
{
    if(!real_sigaction)
    {
        real_sigaction = dlsym(REAL_LIBC, "sigaction");
        if(!real_sigaction)
        {
            fprintf(stderr, "missing symbol: sigaction\n");
            exit(1);
        }

        struct sigaction new_action, old_action;

       new_action.sa_handler = nop;
       sigemptyset (&new_action.sa_mask);
       new_action.sa_flags = 0;

        real_sigaction (SIGINT, NULL, &old_action);
        if (old_action.sa_handler != SIG_IGN)
            real_sigaction (SIGINT, &new_action, NULL);

        real_sigaction (SIGTSTP, NULL, &old_action);
        if (old_action.sa_handler != SIG_IGN)
            real_sigaction (SIGTSTP, &new_action, NULL);
    }
}
static int do_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact, int (*sigaction_proc)(int, const struct sigaction *, struct sigaction *))
{
    if(signum == SIGINT || signum == SIGTSTP)
        return 0;
    return sigaction_proc(signum, act, oldact);
}
__attribute__ ((visibility("default"))) int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact)
{
    init(); // we must always call init as constructor may not be called in some cases (such as loading 32bit pthread library)

    return do_sigaction(signum, act, oldact, real_sigaction);
}
