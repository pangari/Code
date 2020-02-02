#pragma once

#include <stdlib.h>
#include <sys/types.h>
#include <termios.h>

class LineNoise
{
public:
    LineNoise();
    virtual ~LineNoise();

    struct Completions
    {
        size_t len;
        char **cvec;
    };

    void SetMultiLine(bool value) { mlmode = value; }
    void SetCompletionCallback(bool value) { enableCompletionCallback = value; }
    void SetHintsCallback(bool value) { enableHintsCallback = value; }

    virtual void CompletionCallback(const char *str, Completions &lc)
    {
        (void)str;
        (void)lc;
    }
    virtual const char *HintsCallback(const char *str, int &color, int &bold)
    {
        (void)str;
        (void)color;
        (void)bold;
        return nullptr;
    }
    virtual void FreeHintsCallback(const void *ptr) { (void)ptr; }

private:
    struct termios orig_termios;
    int history_max_len;
    int history_len;
    char **history;
    bool enableCompletionCallback;
    bool enableHintsCallback;
    bool rawmode;
    bool mlmode;
    char padding[4];

public:
    char *Prompt(const char *prompt);
    void AddCompletion(Completions &lc, const char *str);
    bool HistoryAdd(const char *line);
    int HistorySetMaxLen(int len);

    /* Save the history in the specified file. On success 0 is returned
     * otherwise -1 is returned. */
    int HistorySave(const char *filename);

    /* Load the history from the specified file. If the file does not exist
     * zero is returned and no operation is performed.
     */
    bool HistoryLoad(const char *filename);

    /* Clear the screen. Used to handle ctrl+l */
    void ClearScreen(void);

    /* This special mode is used by linenoise in order to print scan codes
     * on screen for debugging / development purposes. It is implemented
     * by the linenoise_example program using the --keycodes option. */
    void PrintKeyCodes(void);

private:
    void FreeHistory();
    int EnableRawMode(int fd);
    void DisableRawMode(int fd);
    size_t GetColumns(int ifd, int ofd);

    /* Helper of RefreshSingleLine() and RefreshMultiLine() to show hints
     * to the right of the prompt. */
    void RefreshShowHints(struct AppendBuffer &ab, struct State &state, size_t plen);
    void RefreshSingleLine(struct State &state);

    /* Multi line low level line refresh.
     * Rewrite the currently edited line accordingly to the buffer content,
     * cursor position, and number of columns of the terminal. */
    void RefreshMultiLine(struct State &state);

    /* Calls the two low level functions refreshSingleLine() or
     * refreshMultiLine() according to the selected mode. */
    void RefreshLine(struct State &state);

    void Beep();

    void FreeCompletions(Completions &lc);

    char CompleteLine(struct State &ls);

    /* Insert the character 'c' at cursor current position. */
    int EditInsert(struct State &l, char c);

    /* Move cursor on the left. */
    void EditMoveLeft(struct State &l);

    /* Move cursor on the right. */
    void EditMoveRight(struct State &l);

    /* Move cursor to the start of the line. */
    void EditMoveHome(struct State &l);

    /* Move cursor to the end of the line. */
    void EditMoveEnd(struct State &l);

    /* Substitute the currently edited line with the next or previous history
     * entry as specified by 'dir'. */
    void EditHistoryNext(struct State &l, int dir);

    /* Delete the character at the right of the cursor without altering the cursor
     * position. Basically this is what happens with the "Delete" keyboard key. */
    void EditDelete(struct State &l);

    /* Backspace implementation. */
    void EditBackspace(struct State &l);

    /* Delete the previosu word, maintaining the cursor at the start of the
     * current word. */
    void EditDeletePrevWord(struct State &l);

    ssize_t Prompt(int stdin_fd, int stdout_fd, char *buf, size_t buflen, const char *prompt);

    ssize_t Raw(char *buf, size_t buflen, const char *prompt);
};
