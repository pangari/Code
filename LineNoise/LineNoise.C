/*
 * Copyright (c) 2010-2016, Salvatore Sanfilippo <antirez at gmail dot com>
 * Copyright (c) 2010-2013, Pieter Noordhuis <pcnoordhuis at gmail dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  *  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  *  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "LineNoise.H"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

enum
{
    DefaultHistoryMaxLength = 100,
    MaxLine = 4096,
};

LineNoise::LineNoise()
    : history_max_len(DefaultHistoryMaxLength)
    , history_len(0)
    , history(nullptr)
    , enableCompletionCallback(false)
    , enableHintsCallback(false)
    , rawmode(false)
    , mlmode(false)
    , padding{}
{
    memset(&orig_termios, 0, sizeof(orig_termios));
    (void)padding;

    static_assert(sizeof(LineNoise) == 104, "");
}

LineNoise::~LineNoise() { FreeHistory(); }

/* The state structure represents the state during line editing.
 * We pass this state to functions implementing specific editing
 * functionalities. */
struct State
{
    int ifd;            /* Terminal stdin file descriptor. */
    int ofd;            /* Terminal stdout file descriptor. */
    char *buf;          /* Edited line buffer. */
    size_t buflen;      /* Edited line buffer size. */
    const char *prompt; /* Prompt to display. */
    size_t plen;        /* Prompt length. */
    size_t pos;         /* Current cursor position. */
    size_t oldpos;      /* Previous refresh cursor position. */
    size_t len;         /* Current edited line length. */
    size_t cols;        /* Number of columns in terminal. */
    size_t maxrows;     /* Maximum num of rows used so far (multiline mode) */
    int history_index;  /* The history index we are currently editing. */
    char padding[4];

    State()
    {
        memset(this, 0, sizeof(*this));

        static_assert(sizeof(State) == 88, "");
    }
};

enum
{
    KEY_NULL = 0,   /* NULL */
    CTRL_A = 1,     /* Ctrl+a */
    CTRL_B = 2,     /* Ctrl-b */
    CTRL_C = 3,     /* Ctrl-c */
    CTRL_D = 4,     /* Ctrl-d */
    CTRL_E = 5,     /* Ctrl-e */
    CTRL_F = 6,     /* Ctrl-f */
    CTRL_H = 8,     /* Ctrl-h */
    TAB = 9,        /* Tab */
    CTRL_K = 11,    /* Ctrl+k */
    CTRL_L = 12,    /* Ctrl+l */
    ENTER = 13,     /* Enter */
    CTRL_N = 14,    /* Ctrl-n */
    CTRL_P = 16,    /* Ctrl-p */
    CTRL_T = 20,    /* Ctrl-t */
    CTRL_U = 21,    /* Ctrl+u */
    CTRL_W = 23,    /* Ctrl+w */
    ESC = 27,       /* Escape */
    BACKSPACE = 127 /* Backspace */
};

/* We define a very simple "append buffer" structure, that is an heap
 * allocated string where we can append to. This is useful in order to
 * write all the escape sequences in a buffer and flush them to the standard
 * output in a single call, to avoid flickering effects. */
struct AppendBuffer
{
    char *buf;
    size_t len;

    AppendBuffer()
        : buf(nullptr)
        , len(0)
    {
    }
    ~AppendBuffer() { free(buf); }

    void Append(const char *str, size_t newLen)
    {
        char *newBuf = reinterpret_cast<char *>(realloc(buf, len + newLen));
        memcpy(newBuf + len, str, newLen);
        buf = newBuf;
        len += newLen;
    }
};

static bool IsUnsupportedTerm(void)
{
    static const char *unsupported_term[] = { "dumb", "cons25", "emacs", nullptr };

    char *term = getenv("TERM");
    if (term == nullptr)
        return false;
    for (int j = 0; unsupported_term[j]; j++)
    {
        if (!strcasecmp(term, unsupported_term[j]))
            return true;
    }
    return false;
}

char *LineNoise::Prompt(const char *prompt)
{
    char buf[MaxLine];

    if (IsUnsupportedTerm())
    {
        size_t len;

        if (prompt)
            printf("%s", prompt);
        fflush(stdout);
        if (fgets(buf, MaxLine, stdin) == nullptr)
            return nullptr;

        len = strlen(buf);
        while (len && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
        {
            len--;
            buf[len] = '\0';
        }
        return strdup(buf);
    }
    else
    {
        ssize_t count = Raw(buf, MaxLine, prompt);
        if (count == -1)
            return nullptr;

        return strdup(buf);
    }
}

void LineNoise::AddCompletion(Completions &lc, const char *str)
{
    size_t len = strlen(str);
    char *copy = reinterpret_cast<char *>(malloc(len + 1));
    memcpy(copy, str, len + 1);
    lc.cvec = reinterpret_cast<char **>(realloc(lc.cvec, sizeof(char *) * (lc.len + 1)));
    lc.cvec[lc.len++] = copy;
}

/* Using a circular buffer is smarter, but a bit more complex to handle. */
bool LineNoise::HistoryAdd(const char *line)
{
    char *linecopy;

    if (history_max_len == 0)
        return 0;

    if (history == nullptr)
    {
        history = reinterpret_cast<char **>(malloc(sizeof(char *) * static_cast<size_t>(history_max_len)));
        if (history == nullptr)
            return 0;
        memset(history, 0, (sizeof(char *) * static_cast<size_t>(history_max_len)));
    }

    /* Don't add duplicated lines. */
    if (history_len && !strcmp(history[history_len - 1], line))
        return 0;

    linecopy = strdup(line);
    if (!linecopy)
        return 0;

    if (history_len == history_max_len)
    {
        free(history[0]);
        memmove(history, history + 1, sizeof(char *) * static_cast<size_t>(history_max_len - 1));
        history_len--;
    }

    history[history_len] = linecopy;
    history_len++;

    return 1;
}

int LineNoise::HistorySetMaxLen(int len)
{
    char **newBuf;

    if (len < 1)
        return 0;

    if (history)
    {
        int tocopy = history_len;

        newBuf = reinterpret_cast<char **>(malloc(sizeof(char *) * static_cast<size_t>(len)));

        if (len < tocopy)
        {
            for (int j = 0; j < tocopy - len; j++)
                free(history[j]);
            tocopy = len;
        }

        memset(newBuf, 0, sizeof(char *) * static_cast<size_t>(len));
        memcpy(newBuf, history + (history_max_len - tocopy), sizeof(char *) * static_cast<size_t>(tocopy));
        free(history);
        history = newBuf;
    }

    history_max_len = len;
    if (history_len > history_max_len)
        history_len = history_max_len;

    return 1;
}

/* Save the history in the specified file. On success 0 is returned
 * otherwise -1 is returned. */
int LineNoise::HistorySave(const char *filename)
{
    mode_t old_umask = umask(S_IXUSR | S_IRWXG | S_IRWXO);
    FILE *fp = fopen(filename, "w");
    umask(old_umask);

    if (fp == nullptr)
        return -1;

    chmod(filename, S_IRUSR | S_IWUSR);

    for (int j = 0; j < history_len; j++)
        fprintf(fp, "%s\n", history[j]);

    fclose(fp);
    return 0;
}

/* Load the history from the specified file. If the file does not exist
 * zero is returned and no operation is performed.
 */
bool LineNoise::HistoryLoad(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    char buf[MaxLine];

    if (fp == nullptr)
        return false;

    while (fgets(buf, MaxLine, fp) != nullptr)
    {
        char *p = strchr(buf, '\r');

        if (!p)
            p = strchr(buf, '\n');

        if (p)
            *p = '\0';

        HistoryAdd(buf);
    }
    fclose(fp);

    return true;
}

/* Clear the screen. Used to handle ctrl+l */
void LineNoise::ClearScreen(void)
{
    if (write(STDIN_FILENO, "\x1b[H\x1b[2J", 7) <= 0)
    {
        /* nothing to do, just to avoid warning. */
    }
}

/* This special mode is used by linenoise in order to print scan codes
 * on screen for debugging / development purposes. It is implemented
 * by the linenoise_example program using the --keycodes option. */
void LineNoise::PrintKeyCodes(void)
{
    char quit[4];

    printf("Linenoise key codes debugging mode.\n"
           "Press keys to see scan codes. Type 'quit' at any time to exit.\n");

    if (EnableRawMode(STDIN_FILENO) == -1)
        return;

    memset(quit, ' ', 4);
    while (1)
    {
        char c;
        ssize_t nread;

        nread = read(STDIN_FILENO, &c, 1);
        if (nread <= 0)
            continue;
        memmove(quit, quit + 1, sizeof(quit) - 1); /* shift string to left. */
        quit[sizeof(quit) - 1] = c;                /* Insert current char on the right. */
        if (memcmp(quit, "quit", sizeof(quit)) == 0)
            break;

        printf("'%c' %02x (%d) (type quit to exit)\n", isprint(c) ? c : '?', static_cast<int>(c), static_cast<int>(c));
        printf("\r"); /* Go left edge manually, we are in raw mode. */
        fflush(stdout);
    }
    DisableRawMode(STDIN_FILENO);
}

/* Use the ESC [6n escape sequence to query the horizontal cursor position
 * and return it. On error -1 is returned, on success the position of the
 * cursor. */
static int GetCursorPosition(int ifd, int ofd)
{
    char buf[32];
    int cols, rows;
    unsigned int i = 0;

    /* Report cursor location */
    if (write(ofd, "\x1b[6n", 4) != 4)
        return -1;

    /* Read the response: ESC [ rows ; cols R */
    while (i < sizeof(buf) - 1)
    {
        if (read(ifd, buf + i, 1) != 1)
            break;
        if (buf[i] == 'R')
            break;
        i++;
    }
    buf[i] = '\0';

    /* Parse it. */
    if (buf[0] != ESC || buf[1] != '[')
        return -1;
    if (sscanf(buf + 2, "%d;%d", &rows, &cols) != 2)
        return -1;
    return cols;
}

void LineNoise::FreeHistory()
{
    if (history)
    {
        for (int j = 0; j < history_len; j++)
            free(history[j]);
        free(history);
        history_len = 0;
    }
}

int LineNoise::EnableRawMode(int fd)
{
    struct termios raw;

    if (!isatty(STDIN_FILENO))
        goto fatal;

    if (tcgetattr(fd, &orig_termios) == -1)
        goto fatal;

    /* modify the original mode */
    raw = orig_termios;

    /* input modes: no break, no CR to NL, no parity check, no strip char,
     * no start/stop output control. */
    raw.c_iflag &= static_cast<tcflag_t>(~(BRKINT | ICRNL | INPCK | ISTRIP | IXON));

    /* output modes - disable post processing */
    raw.c_oflag &= static_cast<tcflag_t>(~(OPOST));

    /* control modes - set 8 bit chars */
    raw.c_cflag |= (CS8);

    /* local modes - choing off, canonical off, no extended functions,
     * no signal chars (^Z,^C) */
    raw.c_lflag &= static_cast<tcflag_t>(~(ECHO | ICANON | IEXTEN | ISIG));

    /* control chars - set return condition: min number of bytes and timer.
     * We want read to return every single byte, without timeout. */
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0; /* 1 byte, no timer */

    /* put terminal in raw mode after flushing */
    if (tcsetattr(fd, TCSAFLUSH, &raw) < 0)
        goto fatal;

    rawmode = true;
    return true;

fatal:
    errno = ENOTTY;
    return false;
}

void LineNoise::DisableRawMode(int fd)
{
    /* Don't even check the return value as it's too late. */
    if (rawmode && tcsetattr(fd, TCSAFLUSH, &orig_termios) != -1)
        rawmode = false;
}

size_t LineNoise::GetColumns(int ifd, int ofd)
{
    struct winsize ws;

    if (ioctl(1, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        /* ioctl() failed. Try to query the terminal itself. */
        int start, cols;

        /* Get the initial position so we can restore it later. */
        start = GetCursorPosition(ifd, ofd);
        if (start == -1)
            goto failed;

        /* Go to right margin and get position. */
        if (write(ofd, "\x1b[999C", 6) != 6)
            goto failed;
        cols = GetCursorPosition(ifd, ofd);
        if (cols == -1)
            goto failed;

        /* Restore position. */
        if (cols > start)
        {
            char seq[32];
            snprintf(seq, 32, "\x1b[%dD", cols - start);
            if (write(ofd, seq, strlen(seq)) == -1)
            {
                /* Can't recover... */
            }
        }
        return static_cast<size_t>(cols);
    }
    else
    {
        return ws.ws_col;
    }

failed:
    return 80;
}

/* Helper of RefreshSingleLine() and RefreshMultiLine() to show hints
 * to the right of the prompt. */
void LineNoise::RefreshShowHints(struct AppendBuffer &ab, struct State &state, size_t plen)
{
    char seq[64];
    if (enableHintsCallback && plen + state.len < state.cols)
    {
        int color = -1, bold = 0;
        const char *hint = HintsCallback(state.buf, color, bold);
        if (hint)
        {
            size_t hintlen = strlen(hint);
            size_t hintmaxlen = state.cols - (plen + state.len);
            if (hintlen > hintmaxlen)
                hintlen = hintmaxlen;
            if (bold == 1 && color == -1)
                color = 37;
            if (color != -1 || bold != 0)
                snprintf(seq, 64, "\033[%d;%d;49m", bold, color);
            else
                seq[0] = '\0';
            ab.Append(seq, strlen(seq));
            ab.Append(hint, hintlen);
            if (color != -1 || bold != 0)
                ab.Append("\033[0m", 4);
            /* Call the function to free the hint returned. */
            FreeHintsCallback(hint);
        }
    }
}

void LineNoise::RefreshSingleLine(struct State &state)
{
    char seq[64];
    size_t plen = strlen(state.prompt);

    int fd = state.ofd;
    char *buf = state.buf;
    size_t len = state.len;
    size_t pos = state.pos;

    while ((plen + state.pos) >= state.cols)
    {
        state.buf++;
        state.len--;
        state.pos--;
    }
    while (plen + state.len > state.cols)
    {
        state.len--;
    }

    struct AppendBuffer ab;

    /* Cursor to left edge */
    snprintf(seq, 64, "\r");
    ab.Append(seq, strlen(seq));

    /* Write the prompt and the current buffer content */
    ab.Append(state.prompt, strlen(state.prompt));
    ab.Append(buf, len);

    /* Show hits if any. */
    RefreshShowHints(ab, state, plen);

    /* Erase to right */
    snprintf(seq, 64, "\x1b[0K");
    ab.Append(seq, strlen(seq));

    /* Move cursor to original position. */
    snprintf(seq, 64, "\r\x1b[%dC", static_cast<int>(pos + plen));
    ab.Append(seq, strlen(seq));

    if (write(fd, ab.buf, ab.len) == -1)
    {
        /* Can't recover from write error. */
    }
}

/* Debugging macro. */
#if 0
FILE *lndebug_fp = NULL;
#define lndebug(...)                                                                                                   \
    do                                                                                                                 \
    {                                                                                                                  \
        if (lndebug_fp == NULL)                                                                                        \
        {                                                                                                              \
            lndebug_fp = fopen("/tmp/lndebug.txt", "a");                                                               \
            fprintf(lndebug_fp, "[%d %d %d] p: %d, rows: %d, rpos: %d, max: %d, oldmax: %d\n", (int)state.len,         \
                    (int)state.pos, (int)state.oldpos, plen, rows, rpos, (int)state.maxrows, old_rows);                \
        }                                                                                                              \
        fprintf(lndebug_fp, ", " __VA_ARGS__);                                                                         \
        fflush(lndebug_fp);                                                                                            \
    } while (0)
#else
#define lndebug(fmt, ...)
#endif

/* Multi line low level line refresh.
 * Rewrite the currently edited line accordingly to the buffer content,
 * cursor position, and number of columns of the terminal. */
void LineNoise::RefreshMultiLine(struct State &state)
{
    char seq[64];
    int plen = static_cast<int>(strlen(state.prompt));
    int rows = (plen + state.len + state.cols - 1) / state.cols; /* rows used by current buf. */
    int rpos = (plen + state.oldpos + state.cols) / state.cols;  /* cursor relative row. */
    int rpos2;                                                   /* rpos after refresh. */
    int col;                                                     /* colum position, zero-based. */
    int old_rows = state.maxrows;
    int fd = state.ofd, j;

    /* Update maxrows if needed. */
    if (rows > static_cast<int>(state.maxrows))
        state.maxrows = rows;

    /* First step: clear all the lines used before. To do so start by
     * going to the last row. */
    struct AppendBuffer ab;

    if (old_rows - rpos > 0)
    {
        lndebug("go down %d", old_rows - rpos);
        snprintf(seq, 64, "\x1b[%dB", old_rows - rpos);
        ab.Append(seq, strlen(seq));
    }

    /* Now for every row clear it, go up. */
    for (j = 0; j < old_rows - 1; j++)
    {
        lndebug("clear+up");
        snprintf(seq, 64, "\r\x1b[0K\x1b[1A");
        ab.Append(seq, strlen(seq));
    }

    /* Clean the top line. */
    lndebug("clear");
    snprintf(seq, 64, "\r\x1b[0K");
    ab.Append(seq, strlen(seq));

    /* Write the prompt and the current buffer content */
    ab.Append(state.prompt, strlen(state.prompt));
    ab.Append(state.buf, state.len);

    /* Show hits if any. */
    RefreshShowHints(ab, state, plen);

    /* If we are at the very end of the screen with our prompt, we need to
     * emit a newline and move the prompt to the first column. */
    if (state.pos && state.pos == state.len && (state.pos + plen) % state.cols == 0)
    {
        lndebug("<newline>");
        ab.Append("\n", 1);
        snprintf(seq, 64, "\r");
        ab.Append(seq, strlen(seq));
        rows++;
        if (rows > static_cast<int>(state.maxrows))
            state.maxrows = rows;
    }

    /* Move cursor to right position. */
    rpos2 = (plen + state.pos + state.cols) / state.cols; /* current cursor relative row. */
    lndebug("rpos2 %d", rpos2);

    /* Go up till we reach the expected positon. */
    if (rows - rpos2 > 0)
    {
        lndebug("go-up %d", rows - rpos2);
        snprintf(seq, 64, "\x1b[%dA", (rows - rpos2));
        ab.Append(seq, strlen(seq));
    }

    /* Set column. */
    col = (plen + static_cast<int>(state.pos)) % static_cast<int>(state.cols);
    lndebug("set col %d", 1 + col);
    if (col)
        snprintf(seq, 64, "\r\x1b[%dC", col);
    else
        snprintf(seq, 64, "\r");
    ab.Append(seq, strlen(seq));

    lndebug("\n");
    state.oldpos = state.pos;

    if (write(fd, ab.buf, ab.len) == -1)
    {
        /* Can't recover from write error. */
    }
}

/* Calls the two low level functions refreshSingleLine() or
 * refreshMultiLine() according to the selected mode. */
void LineNoise::RefreshLine(struct State &state)
{
    if (mlmode)
        RefreshMultiLine(state);
    else
        RefreshSingleLine(state);
}

void LineNoise::Beep()
{
    fprintf(stderr, "\x7");
    fflush(stderr);
}

void LineNoise::FreeCompletions(Completions &lc)
{
    size_t i;
    for (i = 0; i < lc.len; i++)
        free(lc.cvec[i]);
    if (lc.cvec != nullptr)
        free(lc.cvec);
}

char LineNoise::CompleteLine(struct State &ls)
{
    Completions lc = { 0, nullptr };
    char c = 0;

    CompletionCallback(ls.buf, lc);
    if (lc.len == 0)
    {
        Beep();
    }
    else
    {
        size_t stop = 0, i = 0;

        while (!stop)
        {
            /* Show completion or original buffer */
            if (i < lc.len)
            {
                struct State saved = ls;

                ls.len = ls.pos = strlen(lc.cvec[i]);
                ls.buf = lc.cvec[i];
                RefreshLine(ls);
                ls.len = saved.len;
                ls.pos = saved.pos;
                ls.buf = saved.buf;
            }
            else
            {
                RefreshLine(ls);
            }

            ssize_t nread = read(ls.ifd, &c, 1);
            if (nread <= 0)
            {
                FreeCompletions(lc);
                return -1;
            }

            switch (c)
            {
                case TAB:
                    i = (i + 1) % (lc.len + 1);
                    if (i == lc.len)
                        Beep();
                    break;
                case ESC:
                    /* Re-show original buffer */
                    if (i < lc.len)
                        RefreshLine(ls);
                    stop = 1;
                    break;
                default:
                    /* Update buffer and return */
                    if (i < lc.len)
                    {
                        int nwritten = snprintf(ls.buf, ls.buflen, "%s", lc.cvec[i]);
                        ls.len = ls.pos = (nwritten >= 0 ? static_cast<size_t>(nwritten) : 0);
                    }
                    stop = 1;
                    break;
            }
        }
    }

    FreeCompletions(lc);

    /* Return last read character */
    return c;
}

/* Insert the character 'c' at cursor current position. */
int LineNoise::EditInsert(struct State &l, char c)
{
    if (l.len < l.buflen)
    {
        if (l.len == l.pos)
        {
            l.buf[l.pos] = c;
            l.pos++;
            l.len++;
            l.buf[l.len] = '\0';
            if ((!mlmode && l.plen + l.len < l.cols && !enableHintsCallback))
            {
                /* Avoid a full update of the line in the
                 * trivial case. */
                if (write(l.ofd, &c, 1) == -1)
                    return -1;
            }
            else
            {
                RefreshLine(l);
            }
        }
        else
        {
            memmove(l.buf + l.pos + 1, l.buf + l.pos, l.len - l.pos);
            l.buf[l.pos] = c;
            l.len++;
            l.pos++;
            l.buf[l.len] = '\0';
            RefreshLine(l);
        }
    }
    return 0;
}

/* Move cursor on the left. */
void LineNoise::EditMoveLeft(struct State &l)
{
    if (l.pos > 0)
    {
        l.pos--;
        RefreshLine(l);
    }
}

/* Move cursor on the right. */
void LineNoise::EditMoveRight(struct State &l)
{
    if (l.pos != l.len)
    {
        l.pos++;
        RefreshLine(l);
    }
}

/* Move cursor to the start of the line. */
void LineNoise::EditMoveHome(struct State &l)
{
    if (l.pos != 0)
    {
        l.pos = 0;
        RefreshLine(l);
    }
}

/* Move cursor to the end of the line. */
void LineNoise::EditMoveEnd(struct State &l)
{
    if (l.pos != l.len)
    {
        l.pos = l.len;
        RefreshLine(l);
    }
}

/* Substitute the currently edited line with the next or previous history
 * entry as specified by 'dir'. */
#define LINENOISE_HISTORY_NEXT 0
#define LINENOISE_HISTORY_PREV 1

void LineNoise::EditHistoryNext(struct State &l, int dir)
{
    if (history_len > 1)
    {
        /* Update the current history entry before to
         * overwrite it with the next one. */
        free(history[history_len - 1 - l.history_index]);
        history[history_len - 1 - l.history_index] = strdup(l.buf);
        /* Show the new entry */
        l.history_index += (dir == LINENOISE_HISTORY_PREV) ? 1 : -1;
        if (l.history_index < 0)
        {
            l.history_index = 0;
            return;
        }
        else if (l.history_index >= history_len)
        {
            l.history_index = history_len - 1;
            return;
        }
        strncpy(l.buf, history[history_len - 1 - l.history_index], l.buflen);
        l.buf[l.buflen - 1] = '\0';
        l.len = l.pos = strlen(l.buf);
        RefreshLine(l);
    }
}

/* Delete the character at the right of the cursor without altering the cursor
 * position. Basically this is what happens with the "Delete" keyboard key. */
void LineNoise::EditDelete(struct State &l)
{
    if (l.len > 0 && l.pos < l.len)
    {
        memmove(l.buf + l.pos, l.buf + l.pos + 1, l.len - l.pos - 1);
        l.len--;
        l.buf[l.len] = '\0';
        RefreshLine(l);
    }
}

/* Backspace implementation. */
void LineNoise::EditBackspace(struct State &l)
{
    if (l.pos > 0 && l.len > 0)
    {
        memmove(l.buf + l.pos - 1, l.buf + l.pos, l.len - l.pos);
        l.pos--;
        l.len--;
        l.buf[l.len] = '\0';
        RefreshLine(l);
    }
}

/* Delete the previosu word, maintaining the cursor at the start of the
 * current word. */
void LineNoise::EditDeletePrevWord(struct State &l)
{
    size_t old_pos = l.pos;
    size_t diff;

    while (l.pos > 0 && l.buf[l.pos - 1] == ' ')
        l.pos--;
    while (l.pos > 0 && l.buf[l.pos - 1] != ' ')
        l.pos--;
    diff = old_pos - l.pos;
    memmove(l.buf + l.pos, l.buf + old_pos, l.len - old_pos + 1);
    l.len -= diff;
    RefreshLine(l);
}

ssize_t LineNoise::Prompt(int stdin_fd, int stdout_fd, char *buf, size_t buflen, const char *prompt)
{
    struct State l;

    /* Populate the linenoise state that we pass to functions implementing
     * specific editing functionalities. */
    l.ifd = stdin_fd;
    l.ofd = stdout_fd;
    l.buf = buf;
    l.buflen = buflen;
    l.prompt = prompt;
    l.plen = strlen(prompt);
    l.oldpos = l.pos = 0;
    l.len = 0;
    l.cols = GetColumns(stdin_fd, stdout_fd);
    l.maxrows = 0;
    l.history_index = 0;

    /* Buffer starts empty. */
    l.buf[0] = '\0';

    /* Make sure there is always space for the nulterm */
    l.buflen--;

    /* The latest history entry is always our current buffer, that
     * initially is just an empty string. */
    HistoryAdd("");

    if (write(l.ofd, prompt, l.plen) == -1)
        return -1;

    while (1)
    {
        char c;
        ssize_t nread;
        char seq[3];

        nread = read(l.ifd, &c, 1);
        if (nread <= 0)
            return static_cast<ssize_t>(l.len);

        /* Only autocomplete when the callback is set. It returns < 0 when
         * there was an error reading from fd. Otherwise it will return the
         * character that should be handled next. */
        if (c == 9 && enableCompletionCallback)
        {
            c = CompleteLine(l);
            /* Return on errors */
            if (c < 0)
                return static_cast<ssize_t>(l.len);
            /* Read next character when 0 */
            if (c == 0)
                continue;
        }

        switch (c)
        {
            case ENTER: /* enter */
                history_len--;
                free(history[history_len]);
                if (mlmode)
                    EditMoveEnd(l);
                if (enableHintsCallback)
                {
                    /* Force a refresh without hints to leave the previous
                     * line as the user typed it after a newline. */
                    enableHintsCallback = false;
                    RefreshLine(l);
                    enableHintsCallback = true;
                }
                return static_cast<int>(l.len);
            case CTRL_C: /* ctrl-c */
                errno = EAGAIN;
                return -1;
            case BACKSPACE: /* backspace */
            case 8:         /* ctrl-h */
                EditBackspace(l);
                break;
            case CTRL_D: /* ctrl-d, remove char at right of cursor, or if the
                    line is empty, act as end-of-file. */
                if (l.len > 0)
                {
                    EditDelete(l);
                }
                else
                {
                    history_len--;
                    free(history[history_len]);
                    return -1;
                }
                break;
            case CTRL_T: /* ctrl-t, swaps current character with previous. */
                if (l.pos > 0 && l.pos < l.len)
                {
                    char aux = buf[l.pos - 1];
                    buf[l.pos - 1] = buf[l.pos];
                    buf[l.pos] = aux;
                    if (l.pos != l.len - 1)
                        l.pos++;
                    RefreshLine(l);
                }
                break;
            case CTRL_B: /* ctrl-b */
                EditMoveLeft(l);
                break;
            case CTRL_F: /* ctrl-f */
                EditMoveRight(l);
                break;
            case CTRL_P: /* ctrl-p */
                EditHistoryNext(l, LINENOISE_HISTORY_PREV);
                break;
            case CTRL_N: /* ctrl-n */
                EditHistoryNext(l, LINENOISE_HISTORY_NEXT);
                break;
            case ESC: /* escape sequence */
                /* Read the next two bytes representing the escape sequence.
                 * Use two calls to handle slow terminals returning the two
                 * chars at different times. */
                if (read(l.ifd, seq, 1) == -1)
                    break;
                if (read(l.ifd, seq + 1, 1) == -1)
                    break;

                /* ESC [ sequences. */
                if (seq[0] == '[')
                {
                    if (seq[1] >= '0' && seq[1] <= '9')
                    {
                        /* Extended escape, read additional byte. */
                        if (read(l.ifd, seq + 2, 1) == -1)
                            break;
                        if (seq[2] == '~')
                        {
                            switch (seq[1])
                            {
                                case '3': /* Delete key. */
                                    EditDelete(l);
                                    break;
                            }
                        }
                    }
                    else
                    {
                        switch (seq[1])
                        {
                            case 'A': /* Up */
                                EditHistoryNext(l, LINENOISE_HISTORY_PREV);
                                break;
                            case 'B': /* Down */
                                EditHistoryNext(l, LINENOISE_HISTORY_NEXT);
                                break;
                            case 'C': /* Right */
                                EditMoveRight(l);
                                break;
                            case 'D': /* Left */
                                EditMoveLeft(l);
                                break;
                            case 'H': /* Home */
                                EditMoveHome(l);
                                break;
                            case 'F': /* End*/
                                EditMoveEnd(l);
                                break;
                        }
                    }
                }

                /* ESC O sequences. */
                else if (seq[0] == 'O')
                {
                    switch (seq[1])
                    {
                        case 'H': /* Home */
                            EditMoveHome(l);
                            break;
                        case 'F': /* End*/
                            EditMoveEnd(l);
                            break;
                    }
                }
                break;
            default:
                if (EditInsert(l, c))
                    return -1;
                break;
            case CTRL_U: /* Ctrl+u, delete the whole line. */
                buf[0] = '\0';
                l.pos = l.len = 0;
                RefreshLine(l);
                break;
            case CTRL_K: /* Ctrl+k, delete from current to end of line. */
                buf[l.pos] = '\0';
                l.len = l.pos;
                RefreshLine(l);
                break;
            case CTRL_A: /* Ctrl+a, go to the start of the line */
                EditMoveHome(l);
                break;
            case CTRL_E: /* ctrl+e, go to the end of the line */
                EditMoveEnd(l);
                break;
            case CTRL_L: /* ctrl+l, clear screen */
                ClearScreen();
                RefreshLine(l);
                break;
            case CTRL_W: /* ctrl+w, delete previous word */
                EditDeletePrevWord(l);
                break;
        }
    }
}

ssize_t LineNoise::Raw(char *buf, size_t buflen, const char *prompt)
{
    ssize_t count;

    if (buflen == 0)
    {
        errno = EINVAL;
        return -1;
    }

    if (!isatty(STDIN_FILENO))
    {
        if (fgets(buf, static_cast<int>(buflen), stdin) == nullptr)
            return -1;
        count = static_cast<ssize_t>(strlen(buf));
        if (count && buf[count - 1] == '\n')
        {
            count--;
            buf[count] = '\0';
        }
    }
    else
    {
        if (!EnableRawMode(STDIN_FILENO))
            return -1;
        count = static_cast<ssize_t>(Prompt(STDIN_FILENO, STDOUT_FILENO, buf, buflen, prompt));
        DisableRawMode(STDIN_FILENO);
        printf("\n");
    }
    return count;
}
