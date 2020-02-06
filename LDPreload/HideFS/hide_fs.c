#define _GNU_SOURCE

#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <limits.h>
#include <netinet/in.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/types.h>

#ifndef RTLD_NEXT
#define RTLD_NEXT ((void *) -1L)
#endif

#define REAL_LIBC RTLD_NEXT

#define AT_FDCWD -100

extern int close(int fd);
extern  char *get_current_dir_name(void);
extern ssize_t readlink(const char *path, char *buf, size_t bufsiz);

static void init (void) __attribute__ ((constructor));

static int (*old_syscall)(int callnum, int* arg1, int* arg2, int* arg3, int* arg4, int* arg5, int* arg6, int* arg7, int* arg8, int* arg9, int* arg10);
static int (*old_access) (const char *path, int amode);
static int (*old_fxstat) (int ver, int fildes, struct stat * buf);
static int (*old_fxstat64) (int ver, int fildes, struct stat64 * buf);
static int (*old_lxstat) (int ver, const char *file, struct stat * buf);
static int (*old_lxstat64) (int ver, const char *file, struct stat64 * buf);
static int (*old_open) (const char *pathname, int flags, mode_t mode);
static int (*old_open64) (const char *pathname, int flags, mode_t mode);
static int (*old_openat) (int dirfd, const char *pathname, int flags, mode_t mode);
static int (*old_openat64) (int dirfd, const char *pathname, int flags, mode_t mode);
static int (*old_chdir)(const char *pathname);
static int (*old_fchdir)(int fd);
static int (*old_rmdir) (const char *pathname);
static int (*old_unlink) (const char *pathname);
static int (*old_unlinkat) (int dirfd, const char *pathname, int flags);
static int (*old_xstat) (int ver, const char *path, struct stat * buf);
static int (*old_xstat64) (int ver, const char *path, struct stat64 * buf);
static int (*old_statfs)(const char *path, struct statfs *buf);

static ssize_t (*old_write) (int fildes, const void *buf, size_t nbyte);

static FILE *(*old_fopen) (const char *filename, const char *mode);
static FILE *(*old_fopen64) (const char *filename, const char *mode);

static DIR *(*old_fdopendir) (int fd);
static DIR *(*old_opendir) (const char *name);

static void (*old_rewinddir) (DIR *__dirp);
static void (*old_seekdir) (DIR *dirp, long int pos);
static long int (*old_telldir) (DIR *dirp);
static int (*old_dirfd) (DIR *dirp);

static int (*old_closedir) (DIR *dirp);

static int (*old_readdir_r) (DIR *__restrict __dirp,
struct dirent *__restrict __entry,
struct dirent **__restrict __result);
static int (*old_readdir64_r) (DIR *__restrict __dirp,
struct dirent64 *__restrict __entry,
struct dirent64 **__restrict __result);

static struct dirent *(*old_readdir) (DIR * dir);
static struct dirent64 *(*old_readdir64) (DIR * dir);

static const char* HIDEFS_DEBUG = NULL;

static char* HIDEFS_GID = NULL;
static char* HIDEFS_UID = NULL;

static size_t HIDEFS_GID_COUNT = 0;
static size_t HIDEFS_UID_COUNT = 0;

static long* HIDEFS_GID_LIST = NULL;
static long* HIDEFS_UID_LIST = NULL;

static char* HIDEFS_STRING = NULL;
static regex_t HIDEFS_RE;

size_t ParseRange(const char* range, long* result)
{
    char* sep = "-,";
    char* rangeCpy = strdup(range);
    char *pch, *prevPch = 0;
    size_t count = 0;
    int prevSep = -1;
    long startRange, endRange, value;

    pch = strtok (rangeCpy, sep);
    while (pch != NULL)
    {
        size_t len = strlen(pch);
        size_t index = (size_t)((pch+len) - rangeCpy);

        if(prevSep == '-' && prevPch)
        {
            startRange = strtol(prevPch, NULL, 10);
            endRange = strtol(pch, NULL, 10);
            if(result)
            {
                long idx;
                for(idx = startRange; idx < endRange; ++idx)
                {
                    result[count++] = idx;
                }
            }
            else
            {
                count += (size_t)(endRange - startRange);
            }
        }

        prevSep = (unsigned char)range[index];

        if((prevSep == ',') || (prevSep == 0))
        {
            value = strtol(pch, NULL, 10);
            if(result)
                result[count] = value;
            ++count;
        }

        prevPch = pch;
        pch = strtok ((pch+len+1), sep);
    }

    free(rangeCpy);
    return count;
}

static __attribute ((constructor)) void init (void)
{
    if(getenv("HIDEFS_GID"))
    {
        HIDEFS_GID = getenv("HIDEFS_GID");

        HIDEFS_GID_COUNT = ParseRange(HIDEFS_GID, NULL);
        if(HIDEFS_GID_COUNT)
        {
            HIDEFS_GID_LIST = malloc(HIDEFS_GID_COUNT * sizeof(long));
            ParseRange(HIDEFS_GID, HIDEFS_GID_LIST);
        }
    }

    if(getenv("HIDEFS_UID"))
    {
        HIDEFS_UID = getenv("HIDEFS_UID");

        HIDEFS_UID_COUNT = ParseRange(HIDEFS_UID, NULL);
        if(HIDEFS_UID_COUNT)
        {
            HIDEFS_UID_LIST = malloc(HIDEFS_UID_COUNT * sizeof(long));
            ParseRange(HIDEFS_UID, HIDEFS_UID_LIST);
        }
    }

    if(getenv("HIDEFS_STRING"))
    {
        HIDEFS_STRING = getenv("HIDEFS_STRING");
        if (regcomp(&HIDEFS_RE, HIDEFS_STRING, REG_EXTENDED|REG_NOSUB) != 0)
            HIDEFS_STRING = NULL;
    }

    HIDEFS_DEBUG = getenv("HIDEFS_DEBUG");

    if(0 && HIDEFS_DEBUG)
    {
        fprintf (stderr, "[-] hide_fs.so loaded: MagicString[%s]"
        , (HIDEFS_STRING ? HIDEFS_STRING : "empty"));

        fprintf (stderr, " MagicGID[");
        if(HIDEFS_GID_LIST && HIDEFS_GID_COUNT)
        {
            size_t idx;
            for(idx = 0; idx < HIDEFS_GID_COUNT; ++idx)
            {
                fprintf (stderr, "%ld", HIDEFS_GID_LIST[idx]);
            }
        }
        else
        {
            fprintf (stderr, "empty");
        }
        fprintf (stderr, "]");

        fprintf (stderr, " MagicUID[");
        if(HIDEFS_UID_LIST && HIDEFS_UID_COUNT)
        {
            size_t idx;
            for(idx = 0; idx < HIDEFS_UID_COUNT; ++idx)
            {
                fprintf (stderr, "%ld;", HIDEFS_UID_LIST[idx]);
            }
        }
        else
        {
            fprintf (stderr, "empty");
        }
        fprintf (stderr, "]");

        fprintf (stderr, "\n");
    }
}

int CheckForMagicID(__gid_t st_gid, __uid_t st_uid)
{
    if(HIDEFS_GID_LIST && HIDEFS_GID_COUNT)
    {
        size_t idx;
        for(idx = 0; idx < HIDEFS_GID_COUNT; ++idx)
        {
            if(st_uid == HIDEFS_GID_LIST[idx])
            {
                if(HIDEFS_DEBUG)
                    fprintf (stderr, "[-] CheckForMagicID(st_gid=[%lu], st_uid=[%lu]) => rc[%d]\n", (unsigned long)st_gid, (unsigned long)st_uid, 1);
                return 1;
            }
        }
    }

    if(HIDEFS_UID_LIST && HIDEFS_UID_COUNT)
    {
        size_t idx;
        for(idx = 0; idx < HIDEFS_UID_COUNT; ++idx)
        {
            if(st_uid == HIDEFS_UID_LIST[idx])
            {
                if(HIDEFS_DEBUG)
                    fprintf (stderr, "[-] CheckForMagicID(st_gid=[%lu], st_uid=[%lu]) => rc[%d]\n", (unsigned long)st_gid, (unsigned long)st_uid, 1);
                return 1;
            }
        }
    }

    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] CheckForMagicID(st_gid=[%lu], st_uid=[%lu]) => rc[%d]\n", (unsigned long)st_gid, (unsigned long)st_uid, 0);
    return 0;
}
int CheckForMagicName(const char* name)
{
    char absPath[PATH_MAX];

    if(HIDEFS_STRING && realpath(name, absPath))
    {
        if(!regexec(&HIDEFS_RE, absPath, (size_t) 0, NULL, 0))
        {
            if(HIDEFS_DEBUG)
                fprintf (stderr, "[-] CheckForMagicName(name=[%s]) => rc[%d]\n", name, 1);
            return 1;
        }
    }

    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] CheckForMagicName(name=[%s]) => rc[%d]\n", name, 0);
    return 0;
}

int CheckForMagic(struct stat* s_fstat, const char* name)
{
    if(s_fstat)
    {
        if(CheckForMagicID(s_fstat->st_gid, s_fstat->st_uid))
            return 1;
    }
    if(name)
    {
        if(CheckForMagicName(name))
            return 1;
    }
    return 0;
}
int CheckForMagic64(struct stat64* s_fstat, const char* name)
{
    if(s_fstat)
    {
        if(CheckForMagicID(s_fstat->st_gid, s_fstat->st_uid))
            return 1;
    }
    if(name)
    {
        if(CheckForMagicName(name))
            return 1;
    }
    return 0;
}

__attribute__ ((visibility("default"))) int syscall(int callnum
                                                    , int* arg1
                                                    , int* arg2
                                                    , int* arg3
                                                    , int* arg4
                                                    , int* arg5
                                                    , int* arg6
                                                    , int* arg7
                                                    , int* arg8
                                                    , int* arg9
                                                    , int* arg10)
{
    int rc;
    if (!old_syscall)
        old_syscall = dlsym (REAL_LIBC, "syscall");

    rc = old_syscall(callnum, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] syscall(callnum=[%d]) => rc[%d]\n", callnum, rc);
    return rc;
}

__attribute__ ((visibility("default"))) int access (const char *path
                                                    , int amode)
{
    int rc;
    struct stat s_fstat;

    if (!old_access)
        old_access = dlsym (REAL_LIBC, "access");

    if (old_xstat == NULL)
        old_xstat = dlsym (REAL_LIBC, "__xstat");

    memset (&s_fstat, 0, sizeof (struct stat));

    old_xstat(_STAT_VER, path, &s_fstat);

    if(CheckForMagic(&s_fstat, path))
    {
        errno = ENOENT;
        return -1;
    }

    rc = old_access (path, amode);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] access(path=[%s], amode=[%d]) => rc[%d]\n", path, amode, rc);
    return rc;
}

__attribute__ ((visibility("default"))) FILE * fopen (const char *filename
                                                      , const char *mode)
{
    FILE* rc;
    struct stat s_fstat;

    if (!old_fopen)
        old_fopen = dlsym (REAL_LIBC, "fopen");

    if (!old_xstat)
        old_xstat = dlsym(REAL_LIBC, "__xstat");

    memset (&s_fstat, 0, sizeof (struct stat));

    old_xstat (_STAT_VER, filename, &s_fstat);

    if(CheckForMagic(&s_fstat, filename))
    {
        errno = ENOENT;
        return NULL;
    }

    rc = old_fopen (filename, mode);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] fopen(filename=[%s], mode=[%s]) => rc[%p]\n", filename, mode, rc);
    return rc;
}

__attribute__ ((visibility("default"))) FILE * fopen64 (const char *filename
                                                        , const char *mode)
{
    FILE* rc;
    struct stat s_fstat;

    if (!old_fopen64)
        old_fopen64 = dlsym (REAL_LIBC, "fopen64");

    if (!old_xstat)
        old_xstat = dlsym(REAL_LIBC, "__xstat");

    memset (&s_fstat, 0, sizeof (struct stat));

    old_xstat (_STAT_VER, filename, &s_fstat);

    if(CheckForMagic(&s_fstat, filename))
    {
        errno = ENOENT;
        return NULL;
    }

    return old_fopen64 (filename, mode);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] fopen64(filename=[%s], mode=[%s]) => rc[%p]\n", filename, mode, rc);
    return rc;
}

__attribute__ ((visibility("default"))) int fstat (int fd
                                                   , struct stat *statBuf)
{
    int rc;
    struct stat s_fstat;

    if (old_fxstat == NULL)
        old_fxstat = dlsym (REAL_LIBC, "__fxstat");

    memset (&s_fstat, 0, sizeof (struct stat));

    old_fxstat (_STAT_VER, fd, &s_fstat);

    if(CheckForMagic(&s_fstat, 0))
    {
        errno = ENOENT;
        return -1;
    }

    rc = old_fxstat (_STAT_VER, fd, statBuf);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] fxstat(fd=[%d], statBuf=[%p]) => rc[%d]\n", fd, statBuf, rc);
    return rc;
}

__attribute__ ((visibility("default"))) ssize_t write (int fildes
                                                       , const void *buf
                                                       , size_t nbyte)
{
    ssize_t rc;
    struct stat s_fstat;

    if (old_write == NULL)
        old_write = dlsym (REAL_LIBC, "write");

    if (old_fxstat == NULL)
        old_fxstat = dlsym (REAL_LIBC, "__fxstat");

    memset (&s_fstat, 0, sizeof (struct stat));

    old_fxstat (_STAT_VER, fildes, &s_fstat);

    if(CheckForMagic(&s_fstat, 0))
    {
        errno = EIO;
        return -1;
    }

    rc = old_write (fildes, buf, nbyte);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] write(fildes=[%d], buf=[%p], nbyte[%zu]) => rc[%zd]\n", fildes, buf, (unsigned long)nbyte, (long)rc);
    return rc;
}

__attribute__ ((visibility("default"))) int fstat64 (int fd
                                                     , struct stat64 *statBuf)
{
    int rc;
    struct stat64 s_fstat;

    if (old_fxstat64 == NULL)
        old_fxstat64 = dlsym (REAL_LIBC, "__fxstat64");

    memset (&s_fstat, 0, sizeof (struct stat));

    old_fxstat64 (_STAT_VER, fd, &s_fstat);

    if(CheckForMagic64(&s_fstat, 0))
    {
        errno = ENOENT;
        return -1;
    }

    rc = old_fxstat64 (_STAT_VER, fd, statBuf);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] fstat64(fd=[%d], statBuf=[%p]) => rc[%d]\n", fd, statBuf, rc);
    return rc;
}

__attribute__ ((visibility("default"))) int __fxstat (int ver
                                                      , int fildes
                                                      , struct stat *statBuf)
{
    int rc;
    struct stat s_fstat;

    if (old_fxstat == NULL)
        old_fxstat = dlsym (REAL_LIBC, "__fxstat");

    memset (&s_fstat, 0, sizeof (struct stat));

    old_fxstat (ver, fildes, &s_fstat);

    if(CheckForMagic(&s_fstat, 0))
    {
        errno = ENOENT;
        return -1;
    }

    rc = old_fxstat (ver, fildes, statBuf);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] __fxstat(ver=[%d], fildes=[%d], statBuf=[%p]) => rc[%d]\n", ver, fildes, statBuf, rc);
    return rc;
}

__attribute__ ((visibility("default"))) int __fxstat64 (int ver
                                                        , int fildes
                                                        , struct stat64 *statBuf)
{
    int rc;
    struct stat64 s_fstat;

    if (old_fxstat64 == NULL)
        old_fxstat64 = dlsym (REAL_LIBC, "__fxstat64");

    memset (&s_fstat, 0, sizeof (struct stat));

    old_fxstat64 (ver, fildes, &s_fstat);

    if(CheckForMagic64(&s_fstat, 0))
    {
        errno = ENOENT;
        return -1;
    }

    rc = old_fxstat64 (ver, fildes, statBuf);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] __fxstat64(ver=[%d], fildes=[%d], statBuf=[%p]) => rc[%d]\n", ver, fildes, statBuf, rc);
    return rc;
}

__attribute__ ((visibility("default"))) int lstat (const char *file
                                                   , struct stat *statBuf)
{
    int rc;
    struct stat s_fstat;

    if (old_lxstat == NULL)
        old_lxstat = dlsym (REAL_LIBC, "__lxstat");

    memset (&s_fstat, 0, sizeof (struct stat));

    old_lxstat (_STAT_VER, file, &s_fstat);

    if(CheckForMagic(&s_fstat, file))
    {
        errno = ENOENT;
        return -1;
    }

    rc = old_lxstat (_STAT_VER, file, statBuf);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] lstat(file=[%s], statBuf=[%p]) => rc[%d]\n", file, statBuf, rc);
    return rc;
}

__attribute__ ((visibility("default"))) int lstat64 (const char *file
                                                     , struct stat64 *statBuf)
{
    int rc;
    struct stat64 s_fstat;

    if (old_lxstat64 == NULL)
        old_lxstat64 = dlsym (REAL_LIBC, "__lxstat64");

    memset (&s_fstat, 0, sizeof (struct stat));

    old_lxstat64 (_STAT_VER, file, &s_fstat);

    if(CheckForMagic64(&s_fstat, file))
    {
        errno = ENOENT;
        return -1;
    }

    rc = old_lxstat64 (_STAT_VER, file, statBuf);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] lstat64(file=[%s], statBuf=[%p]) => rc[%d]\n", file, statBuf, rc);
    return rc;
}

__attribute__ ((visibility("default"))) int __lxstat (int ver
                                                      , const char *file
                                                      , struct stat *statBuf)
{
    int rc;
    struct stat s_fstat;

    if (old_lxstat == NULL)
        old_lxstat = dlsym (REAL_LIBC, "__lxstat");

    memset (&s_fstat, 0, sizeof (struct stat));

    old_lxstat (ver, file, &s_fstat);

    if(CheckForMagic(&s_fstat, file))
    {
        errno = ENOENT;
        return -1;
    }

    rc = old_lxstat (ver, file, statBuf);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] __lxstat(ver=[%d], file=[%s], statBuf=[%p]) => rc[%d]\n", ver, file, statBuf, rc);
    return rc;
}

__attribute__ ((visibility("default"))) int __lxstat64 (int ver
                                                        , const char *file
                                                        , struct stat64 *statBuf)
{
    int rc;
    struct stat64 s_fstat;

    if (old_lxstat64 == NULL)
        old_lxstat64 = dlsym (REAL_LIBC, "__lxstat64");

    memset (&s_fstat, 0, sizeof (struct stat));

    old_lxstat64 (ver, file, &s_fstat);

    if(CheckForMagic64(&s_fstat, file))
    {
        errno = ENOENT;
        return -1;
    }

    rc = old_lxstat64 (ver, file, statBuf);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] __lxstat64(ver=[%d], file=[%s], statBuf=[%p]) => rc[%d]\n", ver, file, statBuf, rc);
    return rc;
}

__attribute__ ((visibility("default"))) int open (const char *pathname
                                                  , int flags
                                                  , mode_t mode)
{
    int rc;
    struct stat s_fstat;

    if (old_open == NULL)
        old_open = dlsym (REAL_LIBC, "open");

    if (old_xstat == NULL)
        old_xstat = dlsym (REAL_LIBC, "__xstat");

    memset (&s_fstat, 0, sizeof (struct stat));

    old_xstat (_STAT_VER, pathname, &s_fstat);

    if(CheckForMagic(&s_fstat, pathname))
    {
        errno = ENOENT;
        return -1;
    }

    rc = old_open (pathname, flags, mode);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] open(pathname=[%s], flags=[%d], mode=[%lu]) => rc[%d]\n", pathname, flags, (unsigned long)mode, rc);
    return rc;
}

__attribute__ ((visibility("default"))) int open64 (const char *pathname
                                                  , int flags
                                                  , mode_t mode)
{
    int rc;
    struct stat64 s_fstat;

    if (old_open64 == NULL)
        old_open64 = dlsym (REAL_LIBC, "open64");

    if (old_xstat64 == NULL)
        old_xstat64 = dlsym (REAL_LIBC, "__xstat64");

    memset (&s_fstat, 0, sizeof (struct stat));

    old_xstat64 (_STAT_VER, pathname, &s_fstat);

    if(CheckForMagic64(&s_fstat, pathname))
    {
        errno = ENOENT;
        return -1;
    }

    rc = old_open64 (pathname, flags, mode);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] open64(pathname=[%s], flags=[%d], mode=[%lu]) => rc[%d]\n", pathname, flags, (unsigned long)mode, rc);
    return rc;
}

__attribute__ ((visibility("default"))) int statfs (const char *path
                                                , struct statfs *statBuf)
{
    int rc;
    struct stat s_fstat;

    if (old_statfs == NULL)
        old_statfs = dlsym (REAL_LIBC, "statfs");

    if (old_xstat == NULL)
        old_xstat = dlsym (REAL_LIBC, "__xstat");

    memset (&s_fstat, 0, sizeof (struct stat));

    old_xstat (_STAT_VER, path, &s_fstat);

    if(CheckForMagic(&s_fstat, path))
    {
        errno = ENOENT;
        return -1;
    }

    rc = old_statfs (path, statBuf);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] statfs(path=[%s], statBuf=[%p]) => rc[%d]\n", path, statBuf, rc);
    return rc;
}

__attribute__ ((visibility("default"))) int openat (int dirfd
                                                    , const char *pathname
                                                    , int flags
                                                    , mode_t mode)
{
    int rc;
    struct stat s_fstat;
    char absPath[PATH_MAX];

    if (old_openat == NULL)
        old_openat = dlsym (REAL_LIBC, "openat");

    if (old_xstat == NULL)
        old_xstat = dlsym (REAL_LIBC, "__xstat");

    memset (&s_fstat, 0, sizeof (struct stat));

    ssize_t rlRc = -1;
    if(dirfd == AT_FDCWD)
    {
        char* cwd = get_current_dir_name();
        strcpy(absPath, cwd);
        rlRc = (ssize_t)strlen(absPath);
        free(cwd);
    }
    else
    {
        char fdPath[64];
        sprintf(fdPath, "/proc/self/fd/%d", dirfd);
        rlRc =readlink(fdPath, absPath, sizeof(absPath));
    }

    if(rlRc > 0)
    {
        strcat(absPath, "/");
        strcat(absPath, pathname);

        old_xstat (_STAT_VER, absPath, &s_fstat);

        if(CheckForMagic(&s_fstat, absPath))
        {
            errno = ENOENT;
            return -1;
        }
    }
    else
    {
        errno = ENOENT;
        return -1;
    }

    rc = old_openat (dirfd, pathname, flags, mode);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] openat(dirfd=[%d], pathname=[%s], flags=[%d], mode=[%lu]) => rc[%d]\n", dirfd, pathname, flags, (unsigned long)mode, rc);
    return rc;
}

__attribute__ ((visibility("default"))) int openat64 (int dirfd
                                                    , const char *pathname
                                                    , int flags
                                                    , mode_t mode)
{
    int rc;
    struct stat64 s_fstat;
    char fdPath[64];
    char absPath[PATH_MAX];

    if (old_openat64 == NULL)
        old_openat64 = dlsym (REAL_LIBC, "openat64");

    if (old_xstat64 == NULL)
        old_xstat64 = dlsym (REAL_LIBC, "__xstat64");

    memset (&s_fstat, 0, sizeof (struct stat));

    sprintf(fdPath, "/proc/self/fd/%d", dirfd);

    if(readlink(fdPath, absPath, sizeof(absPath)) > 0)
    {
        strcat(absPath, "/");
        strcat(absPath, pathname);

        old_xstat64 (_STAT_VER, absPath, &s_fstat);

        if(CheckForMagic64(&s_fstat, absPath))
        {
            errno = ENOENT;
            return -1;
        }
    }

    rc = old_openat64 (dirfd, pathname, flags, mode);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] openat64(dirfd=[%d], pathname=[%s], flags=[%d], mode=[%lu]) => rc[%d]\n", dirfd, pathname, flags, (unsigned long)mode, rc);
    return rc;
}

__attribute__ ((visibility("default"))) int chdir (const char *pathname)
{
    int rc;
    struct stat s_fstat;

    if (old_chdir == NULL)
        old_chdir = dlsym (REAL_LIBC, "chdir");

    if (old_xstat == NULL)
        old_xstat = dlsym (REAL_LIBC, "__xstat");

    memset (&s_fstat, 0, sizeof (struct stat));

    old_xstat (_STAT_VER, pathname, &s_fstat);

    if(CheckForMagic(&s_fstat, pathname))
    {
        errno = ENOENT;
        return -1;
    }

    rc = old_chdir (pathname);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] chdir(pathname=[%s]) => rc[%d]\n", pathname, rc);
    return rc;
}

__attribute__ ((visibility("default"))) int fchdir (int fd)
{
    int rc;
    struct stat s_fstat;
    char fdPath[64];
    char absPath[PATH_MAX];

    if (old_fchdir == NULL)
        old_fchdir = dlsym (REAL_LIBC, "fchdir");

    if (old_xstat == NULL)
        old_xstat = dlsym (REAL_LIBC, "__xstat");

    sprintf(fdPath, "/proc/self/fd/%d", fd);

    if(readlink(fdPath, absPath, sizeof(absPath)) > 0)
    {
        old_xstat (_STAT_VER, absPath, &s_fstat);

        if(CheckForMagic(&s_fstat, absPath))
        {
            errno = ENOENT;
            return -1;
        }
    }

    rc = old_fchdir (fd);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] fchdir(fd=[%d]) => rc[%d]\n", fd, rc);
    return rc;
}

__attribute__ ((visibility("default"))) int rmdir (const char *pathname)
{
    int rc;
    struct stat s_fstat;

    if (old_rmdir == NULL)
        old_rmdir = dlsym (REAL_LIBC, "rmdir");

    if (old_xstat == NULL)
        old_xstat = dlsym (REAL_LIBC, "__xstat");

    memset (&s_fstat, 0, sizeof (struct stat));

    old_xstat (_STAT_VER, pathname, &s_fstat);

    if(CheckForMagic(&s_fstat, pathname))
    {
        errno = ENOENT;
        return -1;
    }

    rc = old_rmdir (pathname);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] rmdir(pathname=[%s]) => rc[%d]\n", pathname, rc);
    return rc;
}

__attribute__ ((visibility("default"))) int stat (const char *path
                                                  , struct stat *statBuf)
{
    int rc;
    struct stat s_fstat;

    if (old_xstat == NULL)
        old_xstat = dlsym (REAL_LIBC, "__xstat");

    memset (&s_fstat, 0, sizeof (struct stat));

    old_xstat (_STAT_VER, path, &s_fstat);

    if(CheckForMagic(&s_fstat, path))
    {
        errno = ENOENT;
        return -1;
    }

    rc = old_xstat (3, path, statBuf);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] stat(path=[%s], statBuf=[%p]) => rc[%d]\n", path, statBuf, rc);
    return rc;
}

__attribute__ ((visibility("default"))) int stat64 (const char *path
                                                    , struct stat64 *statBuf)
{
    int rc;
    struct stat64 s_fstat;

    if (old_xstat64 == NULL)
        old_xstat64 = dlsym (REAL_LIBC, "__xstat64");

    memset (&s_fstat, 0, sizeof (struct stat));

    old_xstat64 (_STAT_VER, path, &s_fstat);

    if(CheckForMagic64(&s_fstat, path))
    {
        errno = ENOENT;
        return -1;
    }

    rc = old_xstat64 (_STAT_VER, path, statBuf);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] stat64(path=[%s], statBuf=[%p]) => rc[%d]\n", path, statBuf, rc);
    return rc;
}

__attribute__ ((visibility("default"))) int __xstat (int ver
                                                     , const char *path
                                                     , struct stat *statBuf)
{
    int rc;
    struct stat s_fstat;

    if (old_xstat == NULL)
        old_xstat = dlsym (REAL_LIBC, "__xstat");

    memset (&s_fstat, 0, sizeof (struct stat));

    old_xstat (ver, path, &s_fstat);

    memset (&s_fstat, 0, sizeof (struct stat));

    if(CheckForMagic(&s_fstat, path))
    {
        errno = ENOENT;
        return -1;
    }

    rc = old_xstat (ver, path, statBuf);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] __xstat(ver=[%d], path=[%s], statBuf=[%p]) => rc[%d]\n", ver, path, statBuf, rc);
    return rc;
}

__attribute__ ((visibility("default"))) int __xstat64 (int ver
                                                       , const char *path
                                                       , struct stat64 *statBuf)
{
    int rc;
    struct stat64 s_fstat;

    if (old_xstat64 == NULL)
        old_xstat64 = dlsym (REAL_LIBC, "__xstat64");

    memset (&s_fstat, 0, sizeof (struct stat));

    old_xstat64 (ver, path, &s_fstat);

    if(CheckForMagic64(&s_fstat, path))
    {
        errno = ENOENT;
        return -1;
    }

    rc = old_xstat64 (ver, path, statBuf);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] __xstat64(ver=[%d], path=[%s], statBuf=[%p]) => rc[%d]\n", ver, path, statBuf, rc);
    return rc;
}

__attribute__ ((visibility("default"))) int unlink (const char *pathname)
{
    int rc;
    struct stat s_fstat;

    if (old_unlink == NULL)
        old_unlink = dlsym (REAL_LIBC, "unlink");

    if (old_xstat == NULL)
        old_xstat = dlsym (REAL_LIBC, "__xstat");

    memset (&s_fstat, 0, sizeof (struct stat));

    old_xstat (_STAT_VER, pathname, &s_fstat);

    if(CheckForMagic(&s_fstat, pathname))
    {
        errno = ENOENT;
        return -1;
    }

    rc = old_unlink (pathname);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] unlink(pathname=[%s]) => rc[%d]\n", pathname, rc);
    return rc;
}

__attribute__ ((visibility("default"))) int unlinkat (int dirfd
                                                      , const char *pathname
                                                      , int flags)
{
    int rc;
    struct stat s_fstat;

    if (old_unlinkat == NULL)
        old_unlinkat = dlsym (REAL_LIBC, "unlinkat");

    if (old_fxstat == NULL)
        old_fxstat = dlsym (REAL_LIBC, "__fxstat");

    memset (&s_fstat, 0, sizeof (struct stat));

    old_fxstat (_STAT_VER, dirfd, &s_fstat);

    if(CheckForMagic(&s_fstat, pathname))
    {
        errno = ENOENT;
        return -1;
    }

    rc = old_unlinkat (dirfd, pathname, flags);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] unlinkat(dirfd=[%d], pathname=[%s]) => rc[%d]\n", dirfd, pathname, rc);
    return rc;
}

struct CustomDIR
{
    DIR * dir;
    const char* name;
};

__attribute__ ((visibility("default"))) DIR * fdopendir (int fd)
{
    DIR * rc;
    struct stat s_fstat;

    if (old_fdopendir == NULL)
        old_fdopendir = dlsym (REAL_LIBC, "fdopendir");

    if (old_fxstat == NULL)
        old_fxstat = dlsym (REAL_LIBC, "__fxstat");

    memset (&s_fstat, 0, sizeof (struct stat));

    old_fxstat (_STAT_VER, fd, &s_fstat);

    if(CheckForMagic(&s_fstat, 0))
    {
        errno = ENOENT;
        return NULL;
    }

    rc = old_fdopendir (fd);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] fdopendir(fd=[%d]) => rc[%p]\n", fd, rc);
    if(!rc)
        return rc;

    {
        struct CustomDIR* customDIR = malloc(sizeof(struct CustomDIR));
        customDIR->dir = rc;
        customDIR->name = 0;
        return (DIR*)customDIR;
    }
}

__attribute__ ((visibility("default"))) DIR * opendir (const char *name)
{
    DIR * rc;
    struct stat s_fstat;

    if (old_opendir == NULL)
        old_opendir = dlsym (REAL_LIBC, "opendir");

    if (old_xstat == NULL)
        old_xstat = dlsym (REAL_LIBC, "__xstat");

    memset (&s_fstat, 0, sizeof (struct stat));

    old_xstat (_STAT_VER, name, &s_fstat);

    if(CheckForMagic(&s_fstat, name))
    {
        errno = ENOENT;
        return NULL;
    }

    rc = old_opendir (name);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] opendir(name=[%s]) => rc[%p]\n", name, rc);
    if(!rc)
        return rc;

    {
        struct CustomDIR* customDIR = malloc(sizeof(struct CustomDIR));
        customDIR->dir = rc;
        customDIR->name = strdup(name);
        return (DIR*)customDIR;
    }
}

__attribute__ ((visibility("default"))) int closedir (DIR *dirp)
{
    int rc;
    if (old_closedir == NULL)
        old_closedir = dlsym (REAL_LIBC, "closedir");

    if(!dirp)
        return old_closedir(dirp);
    struct CustomDIR* customDIR = (struct CustomDIR*)dirp;
    if(customDIR->name)
        free((void*)customDIR->name);
    dirp = customDIR->dir;

    rc = old_closedir(dirp);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] closedir(dirp=[%p]) => rc[%d]\n", dirp, rc);
    return rc;
}

__attribute__ ((visibility("default"))) void rewinddir (DIR *dirp)
{
    if (old_rewinddir == NULL)
        old_rewinddir = dlsym (REAL_LIBC, "rewinddir");

    if(!dirp)
        return old_rewinddir(dirp);
    struct CustomDIR* customDIR = (struct CustomDIR*)dirp;
    dirp = customDIR->dir;

    old_rewinddir(dirp);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] rewinddir(dirp=[%p])\n", dirp);
}
__attribute__ ((visibility("default"))) void seekdir (DIR *dirp
                                                      , long int pos)
{
    if (old_seekdir == NULL)
        old_seekdir = dlsym (REAL_LIBC, "seekdir");

    if(!dirp)
        return old_seekdir(dirp, pos);
    struct CustomDIR* customDIR = (struct CustomDIR*)dirp;
    dirp = customDIR->dir;

    old_seekdir(dirp, pos);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] seekdir(dirp=[%p], pos=[%ld])\n", dirp, pos);
}
__attribute__ ((visibility("default"))) long int telldir (DIR *dirp)
{
    long int rc;
    if (old_telldir == NULL)
        old_telldir = dlsym (REAL_LIBC, "telldir");

    if(!dirp)
        return old_telldir(dirp);
    struct CustomDIR* customDIR = (struct CustomDIR*)dirp;
    dirp = customDIR->dir;

    rc = old_telldir(dirp);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] telldir(dirp=[%p]) => rc[%ld]\n", dirp, rc);
    return rc;
}
__attribute__ ((visibility("default"))) int dirfd (DIR *dirp)
{
    int rc;
    if (old_dirfd == NULL)
        old_dirfd = dlsym (REAL_LIBC, "dirfd");

    if(!dirp)
        return old_dirfd(dirp);
    struct CustomDIR* customDIR = (struct CustomDIR*)dirp;
    dirp = customDIR->dir;

    rc = old_dirfd(dirp);
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] dirfd(dirp=[%p]) => rc[%d]\n", dirp, rc);
    return rc;
}

__attribute__ ((visibility("default"))) struct dirent * readdir (DIR * dirp)
{
    struct dirent * rc;
    struct CustomDIR* customDIR = (struct CustomDIR*)dirp;
    struct dirent *dir;
    char absPath[PATH_MAX];
    struct stat s_fstat;

    if (!old_readdir)
        old_readdir = dlsym (REAL_LIBC, "readdir");
    if(!dirp)
        return old_readdir(dirp);

    size_t parentLen = customDIR->name ? strlen(customDIR->name)+1 : 0;
    if(parentLen)
    {
        memcpy(absPath, customDIR->name, parentLen);
        absPath[parentLen-1] = '/';
    }

    if (old_xstat == NULL)
        old_xstat = dlsym (REAL_LIBC, "__xstat");

    do
    {
        dir = old_readdir (customDIR->dir);
        if(dir == NULL)
            break;

        if (!strcmp (dir->d_name, ".") || !strcmp (dir->d_name, "..") || !strcmp (dir->d_name, "/"))
            break;

        strcpy(absPath+parentLen, dir->d_name);

        memset (&s_fstat, 0, sizeof (struct stat));
        old_xstat (_STAT_VER, absPath, &s_fstat);

        if(CheckForMagic(&s_fstat, absPath))
            continue;

        break;
    }
    while (dir);

    rc = dir;
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] readdir(dirp=[%p]) => rc[%p]\n", dirp, rc);
    return rc;
}

__attribute__ ((visibility("default"))) int readdir_r (DIR * dirp
                                                       , struct dirent * entry
                                                       , struct dirent ** result)
{
    struct CustomDIR* customDIR = (struct CustomDIR*)dirp;
    struct dirent *dir;
    char absPath[PATH_MAX];
    struct stat s_fstat;
    int rc = 0;

    if (!old_readdir_r)
        old_readdir_r = dlsym (REAL_LIBC, "readdir_r");
    if(!dirp)
        return old_readdir_r(dirp, entry, result);

    size_t parentLen = customDIR->name ? strlen(customDIR->name)+1 : 0;
    if(parentLen)
    {
        memcpy(absPath, customDIR->name, parentLen);
        absPath[parentLen-1] = '/';
    }

    if (old_xstat == NULL)
        old_xstat = dlsym (REAL_LIBC, "__xstat");

    do
    {
        rc = old_readdir_r (customDIR->dir, entry, result);
        if((rc != 0) || (result && !(*result)))
            break;

        dir = (*result);

        if (!strcmp (dir->d_name, ".") || !strcmp (dir->d_name, "..") || !strcmp (dir->d_name, "/"))
            break;

        strcpy(absPath+parentLen, dir->d_name);

        memset (&s_fstat, 0, sizeof (struct stat));
        old_xstat (_STAT_VER, absPath, &s_fstat);

        if(CheckForMagic(&s_fstat, absPath))
            continue;

        break;
    }
    while (dir);

    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] readdir_r(dirp=[%p], entry=[%p], result=[%p]) => rc[%d]\n", dirp, entry, result, rc);
    return rc;
}

__attribute__ ((visibility("default"))) struct dirent64 * readdir64 (DIR * dirp)
{
    struct dirent64 * rc;
    struct CustomDIR* customDIR = (struct CustomDIR*)dirp;
    struct dirent64 *dir;
    char absPath[PATH_MAX];
    struct stat64 s_fstat;

    if (!old_readdir64)
        old_readdir64 = dlsym (REAL_LIBC, "readdir64");
    if(!dirp)
        return old_readdir64(dirp);

    size_t parentLen = customDIR->name ? strlen(customDIR->name)+1 : 0;
    if(parentLen)
    {
        memcpy(absPath, customDIR->name, parentLen);
        absPath[parentLen-1] = '/';
    }

    if (old_xstat64 == NULL)
        old_xstat64 = dlsym (REAL_LIBC, "__xstat64");

    do
    {
        dir = old_readdir64 (customDIR->dir);
        if(dir == NULL)
            break;

        if (!strcmp (dir->d_name, ".") || !strcmp (dir->d_name, "..") || !strcmp (dir->d_name, "/"))
            continue;

        strcpy(absPath+parentLen, dir->d_name);

        memset (&s_fstat, 0, sizeof (struct stat));
        old_xstat64 (_STAT_VER, absPath, &s_fstat);

        if(CheckForMagic64(&s_fstat, absPath))
            continue;

        break;
    }
    while (dir);

    rc = dir;
    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] readdir64(dirp=[%p]) => rc[%p]\n", dirp, rc);
    return rc;
}

__attribute__ ((visibility("default"))) int readdir64_r (DIR * dirp
                                                         , struct dirent64 * entry
                                                         , struct dirent64 ** result)
{
    struct CustomDIR* customDIR = (struct CustomDIR*)dirp;
    struct dirent64 *dir;
    char absPath[PATH_MAX];
    struct stat64 s_fstat;
    int rc = 0;

    if (!old_readdir64_r)
        old_readdir64_r = dlsym (REAL_LIBC, "readdir64_r");
    if(!dirp)
        return old_readdir64_r(dirp, entry, result);

    size_t parentLen = customDIR->name ? strlen(customDIR->name)+1 : 0;
    if(parentLen)
    {
        memcpy(absPath, customDIR->name, parentLen);
        absPath[parentLen-1] = '/';
    }

    if (old_xstat64 == NULL)
        old_xstat64 = dlsym (REAL_LIBC, "__xstat64");

    do
    {
        rc = old_readdir64_r (customDIR->dir, entry, result);
        if((rc != 0) || (result && !(*result)))
            break;

        dir = (*result);

        if (!strcmp (dir->d_name, ".") || !strcmp (dir->d_name, "..") || !strcmp (dir->d_name, "/"))
            break;

        strcpy(absPath+parentLen, dir->d_name);

        memset (&s_fstat, 0, sizeof (struct stat));
        old_xstat64 (_STAT_VER, absPath, &s_fstat);

        if(CheckForMagic64(&s_fstat, absPath))
            continue;

        break;
    }
    while (dir);

    if(HIDEFS_DEBUG)
        fprintf (stderr, "[-] readdir64_r(dirp=[%p], entry=[%p], result=[%p]) => rc[%d]\n", dirp, entry, result, rc);
    return rc;
}
