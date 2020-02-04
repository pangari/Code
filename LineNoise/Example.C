#include "LineNoise.H"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

class TestLineNoise : public LineNoise
{
public:
    void CompletionCallback(const char *buf, LineNoise::Completions &lc) override
    {
        if (buf[0] == 'h')
        {
            AddCompletion(lc, "hello");
            AddCompletion(lc, "hello there");
        }
    }

    const char *HintsCallback(const char *buf, int &color, int &bold) override
    {
        if (!strcasecmp(buf, "hello"))
        {
            color = 35;
            bold = 0;
            return " World";
        }
        return nullptr;
    }
};

int main(int argc, char **argv)
{
    char *line;
    char *prgname = argv[0];

    TestLineNoise ln;

    ln.SetCompletionCallback(true);
    ln.SetHintsCallback(true);

    /* Parse options, with --multiline we enable multi line editing. */
    while (argc > 1)
    {
        argc--;
        argv++;
        if (!strcmp(*argv, "--multiline"))
        {
            ln.SetMultiLine(true);
            printf("Multi-line mode enabled.\n");
        }
        else if (!strcmp(*argv, "--keycodes"))
        {
            ln.PrintKeyCodes();
            exit(0);
        }
        else
        {
            fprintf(stderr, "Usage: %s [--multiline] [--keycodes]\n", prgname);
            exit(1);
        }
    }

    /* Load history from file. The history file is just a plain text file
     * where entries are separated by newlines. */
    ln.HistoryLoad("history.txt"); /* Load the history at startup */

    /* Now this is the main loop of the typical linenoise-based application.
     * The call to linenoise() will block as long as the user types something
     * and presses enter.
     *
     * The typed string is returned as a malloc() allocated string by
     * linenoise, so the user needs to free() it. */
    while ((line = ln.Prompt("hello> ")) != NULL)
    {
        /* Do something with the string. */
        if (line[0] != '\0' && line[0] != '/')
        {
            printf("echo: '%s'\n", line);
            ln.HistoryAdd(line);           /* Add to the history. */
            ln.HistorySave("history.txt"); /* Save the history on disk. */
        }
        else if (!strncmp(line, "/historylen", 11))
        {
            /* The "/historylen" command will change the history len. */
            int len = atoi(line + 11);
            ln.HistorySetMaxLen(len);
        }
        else if (line[0] == '/')
        {
            printf("Unreconized command: %s\n", line);
        }
        free(line);
    }
    return 0;
}
