/*
PuTTY is copyright 1997-2017 Simon Tatham.

Portions copyright Robert de Bath, Joris van Rantwijk, Delian
Delchev, Andreas Schultz, Jeroen Massar, Wez Furlong, Nicolas Barry,
Justin Bradford, Ben Harris, Malcolm Smith, Ahmad Khalifa, Markus
Kuhn, Colin Watson, Christopher Staite, and CORE SDI S.A.

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation files
(the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#if !defined(__ARGV_H__)
#define __ARGV_H__

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/*
* Split a complete command line into argc/argv, attempting to do
* it exactly the same way Windows itself would do it (so that
* console utilities, which receive argc and argv from Windows,
* will have their command lines processed in the same way as GUI
* utilities which get a whole command line and must break it
* themselves).
*
* Does not modify the input command line.
*
* The final parameter (argstart) is used to return a second array
* of char * pointers, the same length as argv, each one pointing
* at the start of the corresponding element of argv in the
* original command line. So if you get half way through processing
* your command line in argc/argv form and then decide you want to
* treat the rest as a raw string, you can. If you don't want to,
* `argstart' can be safely left NULL.
*/
void SplitIntoArgV(const char* cmdline, int& argc, char*** argv, const char* **argstart,
                            char* programName)
{
    const char* p;
    char* outputline, *q;
    char** outputargv;
    const char** outputargstart;
    int outputargc;

    /*
    * At first glance the rules appeared to be:
    *
    *  - Single quotes are not special characters.
    *
    *  - Double quotes are removed, but within them spaces cease
    *    to be special.
    *
    *  - Backslashes are _only_ special when a sequence of them
    *    appear just before a double quote. In this situation,
    *    they are treated like C backslashes: so \" just gives a
    *    literal quote, \\" gives a literal backslash and then
    *    opens or closes a double-quoted segment, \\\" gives a
    *    literal backslash and then a literal quote, \\\\" gives
    *    two literal backslashes and then opens/closes a
    *    double-quoted segment, and so forth. Note that this
    *    behaviour is identical inside and outside double quotes.
    *
    *  - Two successive double quotes become one literal double
    *    quote, but only _inside_ a double-quoted segment.
    *    Outside, they just form an empty double-quoted segment
    *    (which may cause an empty argument word).
    *
    *  - That only leaves the interesting question of what happens
    *    when one or more backslashes precedes two or more double
    *    quotes, starting inside a double-quoted string. And the
    *    answer to that appears somewhat bizarre. Here I tabulate
    *    number of backslashes (across the top) against number of
    *    quotes (down the left), and indicate how many backslashes
    *    are output, how many quotes are output, and whether a
    *    quoted segment is open at the end of the sequence:
    *
    *                      backslashes
    *
    *               0         1      2      3      4
    *
    *         0   0,0,y  |  1,0,y  2,0,y  3,0,y  4,0,y
    *            --------+-----------------------------
    *         1   0,0,n  |  0,1,y  1,0,n  1,1,y  2,0,n
    *    q    2   0,1,n  |  0,1,n  1,1,n  1,1,n  2,1,n
    *    u    3   0,1,y  |  0,2,n  1,1,y  1,2,n  2,1,y
    *    o    4   0,1,n  |  0,2,y  1,1,n  1,2,y  2,1,n
    *    t    5   0,2,n  |  0,2,n  1,2,n  1,2,n  2,2,n
    *    e    6   0,2,y  |  0,3,n  1,2,y  1,3,n  2,2,y
    *    s    7   0,2,n  |  0,3,y  1,2,n  1,3,y  2,2,n
    *         8   0,3,n  |  0,3,n  1,3,n  1,3,n  2,3,n
    *         9   0,3,y  |  0,4,n  1,3,y  1,4,n  2,3,y
    *        10   0,3,n  |  0,4,y  1,3,n  1,4,y  2,3,n
    *        11   0,4,n  |  0,4,n  1,4,n  1,4,n  2,4,n
    *
    *
    *      [Test fragment was of the form "a\\\"""b c" d.]
    *
    * There is very weird mod-3 behaviour going on here in the
    * number of quotes, and it even applies when there aren't any
    * backslashes! How ghastly.
    *
    * With a bit of thought, this extremely odd diagram suddenly
    * coalesced itself into a coherent, if still ghastly, model of
    * how things work:
    *
    *  - As before, backslashes are only special when one or more
    *    of them appear contiguously before at least one double
    *    quote. In this situation the backslashes do exactly what
    *    you'd expect: each one quotes the next thing in front of
    *    it, so you end up with n/2 literal backslashes (if n is
    *    even) or (n-1)/2 literal backslashes and a literal quote
    *    (if n is odd). In the latter case the double quote
    *    character right after the backslashes is used up.
    *
    *  - After that, any remaining double quotes are processed. A
    *    string of contiguous unescaped double quotes has a mod-3
    *    behaviour:
    *
    *     * inside a quoted segment, a quote ends the segment.
    *     * _immediately_ after ending a quoted segment, a quote
    *       simply produces a literal quote.
    *     * otherwise, outside a quoted segment, a quote begins a
    *       quoted segment.
    *
    *    So, for example, if we started inside a quoted segment
    *    then two contiguous quotes would close the segment and
    *    produce a literal quote; three would close the segment,
    *    produce a literal quote, and open a new segment. If we
    *    started outside a quoted segment, then two contiguous
    *    quotes would open and then close a segment, producing no
    *    output (but potentially creating a zero-length argument);
    *    but three quotes would open and close a segment and then
    *    produce a literal quote.
    */

    /*
    * First deal with the simplest of all special cases: if there
    * aren't any arguments, return 0,NULL,NULL.
    */
    while (*cmdline && isspace(*cmdline))
        cmdline++;
    if (!*cmdline)
    {
        if (programName)
        {
            argc = 1;
            if (argv)
            {
                (*argv) = (char**)malloc(sizeof(char*));
                (*argv)[0] = programName;
            }
            if (argstart)
            {
                (*argstart) = (const char**)malloc(sizeof(char*));
                (*argv)[0] = programName;
            }
        }
        else
        {
            argc = 0;
            if (argv)
                *argv = NULL;
            if (argstart)
                *argstart = NULL;
        }
        return;
    }

    size_t cmdLen = strlen(cmdline);
    outputline = (char*)malloc((1 + cmdLen) * sizeof(char));
    outputargv = (char**)malloc(((cmdLen + 1 + 2) / 2) * sizeof(char*));
    outputargstart = (const char**)malloc(((cmdLen + 1 + 2 + 2) / 2) * sizeof(char*));

    p = cmdline;
    q = outputline;
    outputargc = 0;

    if (programName)
    {
        outputargv[outputargc] = programName;
        outputargstart[outputargc] = programName;
        outputargc++;
    }

    while (*p)
    {
        int quote;

        /* Skip whitespace searching for start of argument. */
        while (*p && isspace(*p))
            p++;
        if (!*p)
            break;

        /* We have an argument; start it. */
        outputargv[outputargc] = q;
        outputargstart[outputargc] = p;
        outputargc++;
        quote = 0;

        /* Copy data into the argument until it's finished. */
        while (*p)
        {
            if (!quote && isspace(*p))
                break; /* argument is finished */

            if (*p == '"' || *p == '\\')
            {
                /*
                * We have a sequence of zero or more backslashes
                * followed by a sequence of zero or more quotes.
                * Count up how many of each, and then deal with
                * them as appropriate.
                */
                int slashes = 0, quotes = 0;
                while (*p == '\\')
                    slashes++, p++;
                while (*p == '"')
                    quotes++, p++;

                if (!quotes)
                {
                    /*
                    * Special case: if there are no quotes,
                    * slashes are not special at all, so just copy
                    * n slashes to the output string.
                    */
                    while (slashes--)
                        *q++ = '\\';
                }
                else
                {
                    /* Slashes annihilate in pairs. */
                    while (slashes >= 2)
                        slashes -= 2, *q++ = '\\';

                    /* One remaining slash takes out the first quote. */
                    if (slashes)
                        quotes--, *q++ = '"';

                    if (quotes > 0)
                    {
                        /* Outside a quote segment, a quote starts one. */
                        if (!quote)
                            quotes--, quote = 1;

                        /* Now we produce (n+1)/3 literal quotes... */
                        for (int i = 3; i <= quotes + 1; i += 3)
                            *q++ = '"';

                        /* ... and end in a quote segment iff 3 divides n. */
                        quote = (quotes % 3 == 0);
                    }
                }
            }
            else
            {
                *q++ = *p++;
            }
        }

        /* At the end of an argument, just append a trailing NUL. */
        *q++ = '\0';
    }

    outputargv[outputargc] = NULL;
    outputargstart[outputargc] = NULL;

    char** newOutputargv = (char**)realloc(outputargv, (outputargc + 1) * sizeof(char*));
    outputargv = newOutputargv;

    const char** newOutputargstart = (const char**)realloc(outputargstart, (outputargc + 1) * sizeof(char*));
    outputargstart = newOutputargstart;

    argc = outputargc;
    if (argv)
        *argv = outputargv;
    else
        free(outputargv);
    if (argstart)
        *argstart = outputargstart;
    else
        free(outputargstart);
}

#endif
