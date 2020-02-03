#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pcre.h>
#include <errno.h>

#ifdef WIN32
#include <io.h>
#include <fcntl.h>
#include <windows.h>
#endif

#include "HashTable.h"
#include "lookup3.h"
#include "config.h"
#include "pcre_internal.h"
#include "glstring.hpp"

int no_decoration = 0;
int full_line = 0;

#define OVECCOUNT ((128+1) * 3)    // should be a multiple of 3

#undef min
#undef max

#define min(a, b) ((a < b) ? a : b)
#define max(a, b) ((a > b) ? a : b)

typedef struct LogItem_ {

    int  count;
    char text;

} LogItem, *PLogItem;

union AggItem
{
    char*  text;
    double value;
};

typedef struct LogItemAgg_ {

    int  count;
    int  currCount;
    char*          names[((OVECCOUNT/3) - 1)];
    union AggItem  items[((OVECCOUNT/3) - 1)];

} LogItemAgg, *PLogItemAgg;

void* FillHashItemArray(void *data, void *userdata);
int compareHashItem(const void* a, const void* b);
int StringFormatPCRE(int isKey, char* copy, char* subject, int namecount, int retCode, int* ovector, unsigned char *name_table, int name_entry_size);
std::string& ReplacePCRE(std::string & string, int allow_anonymous, const char* subject, int namecount, int retCode, int* ovector, unsigned char *name_table, int name_entry_size);
void AggregateFormatPCRE(PLogItemAgg pLogItem, char* subject, int namecount, int* ovector, unsigned char *name_table, int name_entry_size, int fieldOffset = 0);
int PrintAggregate(PLogItemAgg pLogItem, FILE* file, int namecount);
int GetPrecision(double dbl);
pcre* LoadCompiledRegExp(char* regExpFileName, pcre_extra** pExtra);
char DecodeCString(char* szCHex, int* pCount);

#ifdef WIN32
#ifdef __cplusplus
extern "C" {
#endif
    int PCRETest(int argc, char **argv);
    int PCREGrep(int argc, char **argv);
    int DFTables(int argc, char **argv);
    int TailMain(int argc, char **argv);
    int WcMain(int argc, char **argv);
    int UniqMain(int argc, char **argv);
    int SortMain(int argc, char **argv);
    int HashMain(int argc, char **argv);
    unsigned long int byteflip(unsigned long int value, int n);
#ifdef __cplusplus
}  /* extern "C" */
#endif
#else
int PCRETest(int argc, char **argv);
int PCREGrep(int argc, char **argv);
int DFTables(int argc, char **argv);
int TailMain(int argc, char **argv);
int WcMain(int argc, char **argv);
int UniqMain(int argc, char **argv);
int SortMain(int argc, char **argv);
int HashMain(int argc, char **argv);
unsigned long int byteflip(unsigned long int value, int n);
#endif

void exceptionHandler(int size)
{
    fprintf(stderr, "exceptionHandler(): Failed to allocate memory(%d)!", size);
    exit(1);
}

int MatchFunction(const int mode, const void *key1, const void *key2)
{
    CHItem* obj1 = (CHItem*)key1;
    CHItem* obj2 = (CHItem*)key2;

    if(!obj1 || !obj2) return LIST_NOMATCH;
    if(obj1->keySize != obj2->keySize) return LIST_NOMATCH;

    if(!memcmp(obj1->key, obj2->key, obj1->keySize)) return LIST_MATCH;

    return LIST_NOMATCH;
}

int DeLiner(FILE* file, int mode, int show_all_lines, char* start, char* end, int inc_count, char** inc_patterns, int exc_count, char** exc_patterns, int options, int compile_input, int single_head);

static int gLineSize = (1024*256);


void DisplayDetailedUsage(FILE * file, int showSecret)
{
    if(file)
    {
        fprintf(file, "\nSyntax: LogProcessor {OPTIONS} {MODE} [RegExp] ...\n");
        fprintf(file, "\n");
        fprintf(file, "    LogProcessor Modes  \n");
        fprintf(file, "  ----------------------\n");
        fprintf(file, "\n");
        fprintf(file, "  Simple GREP\n");
        fprintf(file, "     --simple [Selection/Fields]\n");
        fprintf(file, "\n");
        fprintf(file, "  Sequential grouped GREP\n");
        fprintf(file, "     --group [Selection/Fields] [Key]\n");
        fprintf(file, "\n");
        fprintf(file, "  Global grouped GREP\n");
        fprintf(file, "     --global [Selection/Fields] [Key]\n");
        fprintf(file, "\n");
        fprintf(file, "  Aggregated grouped GREP\n");
        fprintf(file, "     --aggregate [Selection/Fields] [Key]\n");
        fprintf(file, "\n");
        fprintf(file, "  Block de-liner\n");
        fprintf(file, "     --deline {--include=...} {--exclude=...} [Start] <End>\n");
        fprintf(file, "\n");
        fprintf(file, "  Block grep\n");
        fprintf(file, "     --block {--include=...} {--exclude=...} [Start] <End>\n");
        fprintf(file, "\n");
        fprintf(file, "  Transform\n");
        fprintf(file, "     --transform [Selection/Fields] [Transformation]\n");
        fprintf(file, "\n");
        fprintf(file, "  Replace\n");
        fprintf(file, "     --replace [Selection/Fields] [Replacement]\n");
        fprintf(file, "\n");
        fprintf(file, "  Global replace\n");
        fprintf(file, "     --replaceGlobal [Selection/Fields] [Replacement]\n");
        fprintf(file, "\n");
        fprintf(file, "  List replacement\n");
        fprintf(file, "     --replaceList [Selection/Fields] [Key] listFile [Key] [Value]\n");
        fprintf(file, "\n");
        fprintf(file, "  List replacement\n");
        fprintf(file, "     --replaceGlobalList [Selection/Fields] [Key] listFile [Key] [Value]\n");
        fprintf(file, "\n");
        if(showSecret)
        {
            fprintf(file, "  Compile to STDOUT\n");
            fprintf(file, "     --compile [RegExp]\n");
            fprintf(file, "\n");
            fprintf(file, "\n");
            fprintf(file, "  Extra Programs:\n");
            fprintf(file, "\n");
            fprintf(file, "     Echo to STDOUT\n");
            fprintf(file, "         --echo [Output]\n");
            fprintf(file, "\n");
            fprintf(file, "     Echo to STDERR\n");
            fprintf(file, "         --errEcho [Output]\n");
            fprintf(file, "\n");
            fprintf(file, "     Tail\n");
            fprintf(file, "         --tail [FileName]\n");
            fprintf(file, "\n");
            fprintf(file, "     Hash\n");
            fprintf(file, "         --hash -SIZE\n");
            fprintf(file, "\n");
            fprintf(file, "     Word Count\n");
            fprintf(file, "         --wc {options}\n");
            fprintf(file, "\n");
            fprintf(file, "     Unique\n");
            fprintf(file, "         --uniq {options}\n");
            fprintf(file, "\n");
            fprintf(file, "     Sort\n");
            fprintf(file, "         --sort {options}\n");
            fprintf(file, "\n");
            fprintf(file, "     Advanced Grep\n");
            fprintf(file, "         --grep ...\n");
            fprintf(file, "\n");
            fprintf(file, "     Test environment\n");
            fprintf(file, "         --test ...\n");
            fprintf(file, "\n");
            fprintf(file, "     DF Tables\n");
            fprintf(file, "         --tables ...\n");
            fprintf(file, "\n");
        }
        fprintf(file, "\n");
        fprintf(file, "  OPTIONS:\n");
        fprintf(file, "     --ignore-case          Ignore case distinctions\n");
        fprintf(file, "     --no-line-numbers      Suppress line numbers\n");
        fprintf(file, "     --single-head          Allow single lines when delining\n");
        fprintf(file, "     --no-decoration        Don't decorate output\n");
        fprintf(file, "     --full-line            Display full line, ignoring subpatterns\n");
        fprintf(file, "     --show-all-lines       De-liner will display all unmatched lines\n");
        fprintf(file, "     --allow-anonymous      Allow anonymous subpatterns\n");
        fprintf(file, "                            when transforming\n");
        if(showSecret)
        {
            fprintf(file, "     --useCompiled          Use compiled RegExp's\n");
            fprintf(file, "     --hashTableSize=?      Change hashTable size (16384)\n");        
            fprintf(file, "     --lineSize=?           Change line size in KB (1024*256)\n");        
        }
        fprintf(file, "\n");
        fprintf(file, "\n");
        fprintf(file, "  NOTES:\n");
        fprintf(file, "     All RegExp's use standard Pearl syntax\n");
        fprintf(file, "     where fields can be defined by using\n");
        fprintf(file, "     subpatterns (numbered or named)\n");
        fprintf(file, "\n");
        fprintf(file, "     Aggregation uses named subpatterns with\n");
        fprintf(file, "     the following formats:\n");
        fprintf(file, "         ?_FRT - First Value\n");
        fprintf(file, "         ?_LST - Last Value\n");
        fprintf(file, "         ?_MIN - Numerical minimum\n");
        fprintf(file, "         ?_MAX - Numerical maximum\n");
        fprintf(file, "         ?_AVG - Numerical average\n");
        fprintf(file, "         ?_SUM - Numerical sum\n");
        fprintf(file, "         ?_TMN - Textual minimum\n");
        fprintf(file, "         ?_TMX - Textual maximum\n");
        fprintf(file, "\n");
        fprintf(file, "     Replace and transform support named subpatterns\n");
        fprintf(file, "     in these formats:\n");
        fprintf(file, "         ?_UCASE     - Upper case\n");
        fprintf(file, "         ?_LCASE     - Lower case\n");
        fprintf(file, "         ?_SPC_#     - Pad string output\n");
        fprintf(file, "         ?_PAD_#_#   - Pad number output\n");
        fprintf(file, "         ?_ADD_#     - Addition\n");
        fprintf(file, "         ?_SUB_#     - Subtraction\n");
        fprintf(file, "         ?_MUL_#     - Multiplication\n");
        fprintf(file, "         ?_DIV_#     - Division\n");
        fprintf(file, "         ?_MOD_#     - Modulus\n");
        fprintf(file, "         ?_AND_#     - Bitwise AND\n");
        fprintf(file, "         ?_XOR_#     - Bitwise exclusive OR\n");
        fprintf(file, "         ?_OR_#      - Bitwise inclusive OR\n");
        fprintf(file, "         ?_LSHIFT_#  - Bitwise shift left\n");
        fprintf(file, "         ?_RSHIFT_#  - Bitwise shift right\n");
        fprintf(file, "\n");
        fflush(file);
    }
}

int LogProcessor(int argc, char **argv);

int main(int argc, char **argv)
{
    int retCode;

#ifdef WIN32
    CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
    _setmode(_fileno(stderr), _O_BINARY);
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &consoleScreenBufferInfo);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
        FOREGROUND_INTENSITY | FOREGROUND_RED |
        FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif

    retCode = LogProcessor(argc, argv);

#ifdef WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), consoleScreenBufferInfo.wAttributes);
#endif

    return retCode;
}

#define MAX_PATTERN_COUNT 100

int             inc_count = 0;
char*           inc_patterns[MAX_PATTERN_COUNT];
int             exc_count = 0;
char*           exc_patterns[MAX_PATTERN_COUNT];

void ResetVector(int * ovector, int length, bool force = false)
{
    for(int idx = 0; idx < length; idx++)
    {
        if(force || (ovector[idx] >= 0))
            ovector[idx] = -1;
        else if(!force) 
            break;
    }
}

void ShiftVector(int * ovector, int length, int shift)
{
    for(int idx = 0; idx < length; idx++)
        if(ovector[idx] >= 0)
            ovector[idx] += shift;
}

static char _WorkBuffer[1024*256];
static int _WritePosition = 0;
static int _ReadPosition = 0;
static int _MaxCount = sizeof(_WorkBuffer);

#define GET_LINE(_FinalBuffer, _FinalCount, _File) (GetLine(_WorkBuffer, _WritePosition, _ReadPosition, _MaxCount, _FinalBuffer, _FinalCount, _File))

bool GetLine(char * _WorkBuffer, int & _WritePosition, int & _ReadPosition, int _MaxCount, char * _FinalBuffer, int & _FinalCount, FILE * _File)
{
    int _TotalCount = 0;
    char * _TempBuffer;
    int pos = 0;
    int idx, count;

    while(_FinalCount)
    {
        count = (_WritePosition - _ReadPosition);
        _TempBuffer = &_WorkBuffer[_ReadPosition];

        if(_FinalCount < count) 
        {
            count = _FinalCount;
        }

        for(idx = 0; idx < count; idx++)
        {
            _FinalBuffer[pos] = _TempBuffer[idx];
            if(_FinalBuffer[pos++] == '\n') 
            {
                _ReadPosition += (idx+1);
                _FinalCount -= (idx+1);
                return true;
            }
        }

        _TotalCount += count;
        _FinalCount -= count;
        _WritePosition = 0;
        _ReadPosition = 0;

        _WritePosition = (int)fread(_WorkBuffer, 1, _MaxCount, _File);

        if(_WritePosition == 0) 
        {
            return ((_TotalCount == 0) ? false : true);
        }
    }

    return true;
}

int LogProcessor(int argc, char **argv)
{
    pcre *filter_re = NULL;
    pcre *unique_re = NULL;
    pcre_extra* filter_ex = NULL;
    pcre_extra *unique_ex = NULL;
    char* copy = NULL;
    char* subject = NULL;
    char uniquekey[65536+1] = "";
    char tmpUniquekey[65536+1] = "";
    const char *error;
    char *filter = NULL;
    char *unique = NULL;
    char *filename = NULL;
    char *replaceStr = NULL;
    unsigned char *name_table_filter;
    unsigned char *name_table_unique;
    unsigned char *tabptr;
    int erroffset;
    int namecount_filter;
    int namecount_unique;
    int name_entry_size_filter;
    int name_entry_size_unique;
    int ovector[OVECCOUNT];
    int ovector2[OVECCOUNT];
    int uniquecount = 0;
    int uniquekey_len = 0;
    int rc, rc2, idx;
    int copy_len;
    int lg_rc = -1;
    unsigned int line_number = 0;
    CHTbl htbl;

    int simple = 0;
    int group = 0;
    int global_unique = 0;
    int global_aggregate = 0;
    int global_aggregate_linked = 0;
    int fieldOffset = 0;
    int deline = 0;
    int options = 0;
    int no_line_numbers = 0;
    int show_all_lines = 0;
    int single_head = 0;
    int compile_output = 0;
    int compile_input = 0;
    int transform = 0;
    int allow_anonymous = 0;
    int hash_table_size = 16384;
    int echo = 0;
    int errEcho = 0;
    int replace = 0;

    ResetVector(ovector, OVECCOUNT, true);
    ResetVector(ovector2, OVECCOUNT, true);

    FILE* file = stdin;

    if((argc > 1) &&
        (argv[1][0] == '<') &&
        (argv[1][1] == ' ') &&
        (argv[1][2] != '\0'))
    {
        file = fopen(&argv[1][2], "rb");
        if(!file) stdin;
        argc--;
        argv++;
    }

    FILE* origFile = file;

    while(argc > 1 && argv[1][0] == '-' && argv[1][1] == '-')
    {
        if(!strcmp("help", &argv[1][2]))                    { DisplayDetailedUsage(stderr, 0); return 1; }
        if(!strcmp("HeLp", &argv[1][2]))                    { DisplayDetailedUsage(stderr, 1); return 1; }
        else if(!strcmp("simple", &argv[1][2]))             simple = 1;
        else if(!strcmp("group", &argv[1][2]))              group = 1;
        else if(!strcmp("global", &argv[1][2]))             global_unique = 1;
        else if(!strcmp("aggregate", &argv[1][2]))          global_aggregate = 1; // Frt, Lst, Min Max Avg Sum, TMn, TMx
        else if(!strcmp("deline", &argv[1][2]))             deline = 1;
        else if(!strcmp("block", &argv[1][2]))              deline = 2;
        else if(!strcmp("ignore-case", &argv[1][2]))        options |= PCRE_CASELESS;
        else if(!strcmp("show-all-lines", &argv[1][2]))     show_all_lines = 1;
        else if(!strcmp("no-line-numbers", &argv[1][2]))    no_line_numbers = 1;
        else if(!strcmp("single-head", &argv[1][2]))        single_head = 1;
        else if(!strcmp("no-decoration", &argv[1][2]))      no_decoration = 1;
        else if(!strcmp("full-line", &argv[1][2]))          full_line = 1;
        else if(!strcmp("compile", &argv[1][2]))            compile_output = 1;
        else if(!strcmp("useCompiled", &argv[1][2]))        compile_input = 1;
        else if(!strcmp("transform", &argv[1][2]))          transform = 1;
        else if(!strcmp("replace", &argv[1][2]))            replace = 1;
        else if(!strcmp("replaceGlobal", &argv[1][2]))      replace = 2;
        else if(!strcmp("replaceList", &argv[1][2]))        replace = 3;
        else if(!strcmp("replaceGlobalList", &argv[1][2]))  replace = 33;
        else if(!strcmp("allow-anonymous", &argv[1][2]))    allow_anonymous = 1;
        else if(!strcmp("echo", &argv[1][2]))               echo = 1;
        else if(!strcmp("errEcho", &argv[1][2]))               errEcho = 1;
        else if(!strncmp("hashTableSize=", &argv[1][2], 14))hash_table_size = strtoul(&argv[1][16], NULL, 10);
        else if(!strncmp("lineSize=", &argv[1][2], 9))      gLineSize = strtoul(&argv[1][11], NULL, 10)*1024;
        else if(!strncmp("include=", &argv[1][2], 8))       inc_patterns[inc_count++] = &argv[1][10];
        else if(!strncmp("exclude=", &argv[1][2], 8))       exc_patterns[exc_count++] = &argv[1][10];
        else if(!strcmp("tail", &argv[1][2]))               return TailMain((argc - 1), (argv + 1));
        else if(!strcmp("wc", &argv[1][2]))                 return WcMain((argc - 1), (argv + 1));
        else if(!strcmp("uniq", &argv[1][2]))               return UniqMain((argc - 1), (argv + 1));
        else if(!strcmp("sort", &argv[1][2]))               return SortMain((argc - 1), (argv + 1));
        else if(!strcmp("hash", &argv[1][2]))               return HashMain((argc - 1), (argv + 1));
        else if(!strcmp("test", &argv[1][2]))               return PCRETest((argc - 1), (argv + 1));
        else if(!strcmp("grep", &argv[1][2]))               return PCREGrep((argc - 1), (argv + 1));
        else if(!strcmp("tables", &argv[1][2]))             return DFTables((argc - 1), (argv + 1));
        argc--;
        argv++;
    }

    if(deline)
    {
        if(!inc_count) inc_patterns[inc_count++] = ".*";
        if(argc == 2)
            return DeLiner(file, deline, show_all_lines, argv[1], NULL, inc_count, inc_patterns, exc_count, exc_patterns, options, compile_input, single_head);
        else if(argc == 3)
            return DeLiner(file, deline, show_all_lines, argv[1], argv[2], inc_count, inc_patterns, exc_count, exc_patterns, options, compile_input, single_head);
        else
        {
            DisplayDetailedUsage(stderr, 0);
            return 1;
        }
    }

    copy = (char*)malloc(gLineSize+1);
    subject = (char*)malloc(gLineSize+1);

    chtbl_exception_handler(exceptionHandler);
    list_exception_handler(exceptionHandler);

    memset(uniquekey, 0, sizeof(uniquekey));

    switch(argc - 1)
    {
    case 0:
        DisplayDetailedUsage(stderr, 0);
        return 0;
        break;

    case 2:
        if(transform || replace) replaceStr = argv[2];
        unique = argv[2];
        if(!chtbl_init(&htbl, hash_table_size, (int(*)(const void*, const int))hashstring, (int(*)(const int, const void*, const void*))MatchFunction, (void(*)(void*))free) != 0)
        {
            fprintf(stderr, "Allocation error: Unable to create HashMap\n");
            return 1;
        }

    case 1:
        if(((argc-1) == 1) && !(simple + compile_output + echo + errEcho))
        {
            if(unique) chtbl_destroy(&htbl);
            DisplayDetailedUsage(stderr, 0);
            return 1;
        }
        else
        {
            filter = argv[1];
        }
        break;

    default:
        if(global_aggregate && (((argc - 1) == 2) || (((argc - 1) % 3) == 0)))
        {
            filter = argv[1];
            unique = argv[2];
            if((argc - 1) > 2)
            {
                filename = argv[3];
                global_aggregate_linked = 1;
            }

            if(!chtbl_init(&htbl, hash_table_size, (int(*)(const void*, const int))hashstring, (int(*)(const int, const void*, const void*))MatchFunction, (void(*)(void*))free) != 0)
            {
                fprintf(stderr, "Allocation error: Unable to create HashMap\n");
                return 1;
            }
            break;
        }
        else if(((replace % 10) == 3) && ((argc - 1) == 5))
        {
            filename = argv[3];

            file = fopen(filename, "rb");
            unique = argv[4];
            filter = argv[5];

            if(!chtbl_init(&htbl, hash_table_size, (int(*)(const void*, const int))hashstring, (int(*)(const int, const void*, const void*))MatchFunction, (void(*)(void*))free) != 0)
            {
                fprintf(stderr, "Allocation error: Unable to create HashMap\n");
                return 1;
            }
            break;
        }
        DisplayDetailedUsage(stderr, 0);
        return 1;
    }

    if(global_aggregate_linked)
    {
        argc -= 3;
        argv += 3;
        file = fopen(filename, "rb");
    }
    namecount_filter = 0;

TheMainWhileLoop:

    while(file)
    {
        if(echo || errEcho)
        {
            int idx = 0;
            int pos = 0;
            int inPos = 0;
            int outPos = 0;
            char* buf = filter;

            pos = (int)strlen(filter);

            while(inPos < pos)
            {
                buf[outPos++] = DecodeCString(&buf[inPos], &idx);
                inPos += idx;
            }

            buf[outPos] = 0;
            fputs(filter, (errEcho ? stderr : stdout));

            return 0;
        }
        else if(filter)
        {
            filter_re = compile_input ? LoadCompiledRegExp(filter, &filter_ex) :
                pcre_compile(filter,    // the pattern
                options,                // default options
                &error,                 // for error message
                &erroffset,             // for error offset
                NULL);                  // use default character tables

        if(filter_re == NULL)
        {
            if(unique) chtbl_destroy(&htbl);
            fprintf(stderr, "LogProcessor: [filter] compilation failed at offset %d: %s\n", erroffset, error);
            if(filter_re) pcre_free(filter_re);
            if(unique_re) pcre_free(unique_re);
            return 1;
        }
        if(!filter_ex) filter_ex = pcre_study(filter_re, 0, &error);

        if(compile_output)
        {
            uschar sbuf[8];
            int true_size = ((real_pcre *)filter_re)->size;
            int true_study_size = 0;
            pcre_extra* extra = pcre_study(filter_re, 0, &error);

            if(unique) chtbl_destroy(&htbl);

            if(extra) true_study_size = ((pcre_study_data *)(extra->study_data))->size;

            sbuf[0] = (true_size >> 24)  & 255;
            sbuf[1] = (true_size >> 16)  & 255;
            sbuf[2] = (true_size >>  8)  & 255;
            sbuf[3] = (true_size)  & 255;

            sbuf[4] = (true_study_size >> 24)  & 255;
            sbuf[5] = (true_study_size >> 16)  & 255;
            sbuf[6] = (true_study_size >>  8)  & 255;
            sbuf[7] = (true_study_size)  & 255;

            fwrite(sbuf, 1, 8, stdout);
            fwrite(filter_re, 1, true_size, stdout);
            if(extra) fwrite(extra->study_data, 1, true_study_size, stdout);
            if(extra) pcre_free(extra);

            if(filter_re) pcre_free(filter_re);
            if(unique_re) pcre_free(unique_re);
            return 0;
        }

        fieldOffset += namecount_filter;

        (void)pcre_fullinfo(filter_re,                      // the compiled pattern
            NULL,                           // no extra data - we didn't study the pattern
            PCRE_INFO_NAMECOUNT,            // number of named substrings
            &namecount_filter);             // where to put the answer

        if(namecount_filter)
        {
            (void)pcre_fullinfo(filter_re,                  // the compiled pattern
                NULL,                       // no extra data - we didn't study the pattern
                PCRE_INFO_NAMETABLE,        // address of the table
                &name_table_filter);        // where to put the answer

            (void)pcre_fullinfo(filter_re,                  // the compiled pattern
                NULL,                       // no extra data - we didn't study the pattern
                PCRE_INFO_NAMEENTRYSIZE,    // size of each entry in the table
                &name_entry_size_filter);   // where to put the answer
        }
        }

        if(replaceStr)
        {
            int idx = 0;
            int pos = 0;
            int inPos = 0;
            int outPos = 0;
            char* buf = replaceStr;

            pos = (int)strlen(replaceStr);

            while(inPos < pos)
            {
                buf[outPos++] = DecodeCString(&buf[inPos], &idx);
                inPos += idx;
            }

            buf[outPos] = 0;
        }
        else if(unique)
        {
            unique_re = compile_input ? LoadCompiledRegExp(unique, &unique_ex) :
                pcre_compile(unique,    // the pattern
                options,                // default options
                &error,                 // for error message
                &erroffset,             // for error offset
                NULL);                  // use default character tables

        if(unique_re == NULL)
        {
            if(unique) chtbl_destroy(&htbl);
            fprintf(stderr, "LogProcessor: [unique] compilation failed at offset %d: %s\n", erroffset, error);
            if(filter_re) pcre_free(filter_re);
            if(unique_re) pcre_free(unique_re);
            return 1;
        }
        if(!unique_ex) unique_ex = pcre_study(unique_re, 0, &error);

        (void)pcre_fullinfo(unique_re,                      // the compiled pattern
            NULL,                           // no extra data - we didn't study the pattern
            PCRE_INFO_NAMECOUNT,            // number of named substrings
            &namecount_unique);             // where to put the answer

        if(namecount_unique)
        {
            (void)pcre_fullinfo(unique_re,                  // the compiled pattern
                NULL,                       // no extra data - we didn't study the pattern
                PCRE_INFO_NAMETABLE,        // address of the table
                &name_table_unique);        // where to put the answer

            (void)pcre_fullinfo(unique_re,                  // the compiled pattern
                NULL,                       // no extra data - we didn't study the pattern
                PCRE_INFO_NAMEENTRYSIZE,    // size of each entry in the table
                &name_entry_size_unique);   // where to put the answer
        }
        }

        while(feof(file) != EOF)
        {
            int lineSize = gLineSize;
            if(GET_LINE(subject, lineSize, file))
            {
                unsigned int subject_length = (gLineSize - lineSize);

                line_number++;

                while(subject_length)
                {
                    if((subject[subject_length-1] == '\r' || subject[subject_length-1] == '\n'))
                        subject[--subject_length] = 0;
                    else
                        break;
                }

                if(filter)
                {
                    ResetVector(ovector, OVECCOUNT);
                    rc = pcre_exec(filter_re,           // the compiled pattern
                        NULL,                 // no extra data - we didn't study the pattern
                        subject,              // the subject string
                        (int)subject_length,      // the length of the subject
                        0,                    // start at offset 0 in the subject
                        0,                    // default options
                        ovector,              // output vector for substring information
                        OVECCOUNT);           // number of elements in the output vector
                }
                else
                {
                    rc = 1;
                }

                if(rc < 0)
                {
                    if(rc == PCRE_ERROR_NOMATCH)
                    {
                        if(replaceStr)
                        {
                            fputs(subject, stdout);
                            fputs("\n", stdout);
                        }
                        continue;
                    }
                    else
                    {
                        if(filter_re) pcre_free(filter_re);
                        if(unique_re) pcre_free(unique_re);
                        return 1;
                    }
                }
                else
                {
                    if(replaceStr)
                    {
                        if(rc > 0)
                        {
                            if(replace)
                            {
                                if(rc < 0)
                                {
                                    fputs(subject, stdout);
                                }
                                else
                                {
                                    std::string result(subject);

                                    while(rc > 0)
                                    {
                                        int substring_offset = ovector[0];
                                        const char *substring_start = result.c_str() + substring_offset;
                                        int substring_length = ovector[1] - ovector[0];
                                        std::string string(replaceStr);
                                        ReplacePCRE(string, allow_anonymous, result.c_str(), namecount_filter, rc, ovector, name_table_filter, name_entry_size_filter);
                                        result.replace(substring_offset, substring_length, string.c_str());

                                        if(replace == 2)
                                        {
                                            // Try again for global replace
                                            int shift = substring_offset + (int)string.length();
                                            ResetVector(ovector, OVECCOUNT);

                                            rc = pcre_exec(filter_re,NULL,
                                                result.c_str() + shift,
                                                (int)result.length() - shift,
                                                0,0,ovector,OVECCOUNT);
                                            if(rc > 0)
                                            {
                                                // We need to shift the vector as well
                                                ShiftVector(ovector, OVECCOUNT, shift);
                                            }
                                        }
                                        else
                                            break;
                                    }

                                    fputs(result.c_str(), stdout);
                                }
                            }
                            else
                            {
                                std::string string(replaceStr);
                                ReplacePCRE(string, allow_anonymous, subject, namecount_filter, rc, ovector, name_table_filter, name_entry_size_filter);
                                fputs(string.c_str(), stdout);
                            }
                        }
                        fputs("\n", stdout);
                    }
                    else if(unique)
                    {
                        ResetVector(ovector2, OVECCOUNT);
                        rc2 = pcre_exec(unique_re,          // the compiled pattern
                            NULL,                 // no extra data - we didn't study the pattern
                            subject,              // the subject string
                            (int)subject_length,      // the length of the subject
                            0,                    // start at offset 0 in the subject
                            0,                    // default options
                            ovector2,             // output vector for substring information
                            OVECCOUNT);           // number of elements in the output vector

                        if(rc2 < 0)
                        {
                            if(rc2 == PCRE_ERROR_NOMATCH)
                            {
                                 if(replace)
                                 {
                                     fputs(subject, stdout);
                                     fputs("\n", stdout);
                                 }
                                continue;
                            }
                            else
                            {
                                if(filter_re) pcre_free(filter_re);
                                if(unique_re) pcre_free(unique_re);
                                return 1;
                            }
                        }
                        else
                        {
                            lg_rc = rc;

                            if((replace % 10) == 4)
                            {
                                CHItem hItem;
                                PLogItem logItem;

                                memset(&hItem, 0, sizeof(hItem));

                                uniquekey_len = StringFormatPCRE(1, uniquekey, subject, namecount_unique, rc2, ovector2, name_table_unique, name_entry_size_unique);

                                logItem = (PLogItem)chtbl_lookup(&htbl, uniquekey, uniquekey_len, NULL);

                                if(logItem)
                                {
                                    if(rc == 1)
                                    {
                                        fputs(&logItem->text, stdout);
                                    }
                                    else if(rc == 2)
                                    {
                                        idx = 1;
                                        fwrite(subject, 1, ovector[2*idx], stdout);
                                        fputs(&logItem->text, stdout);
                                        fputs(subject + ovector[2*idx+1], stdout);
                                    }
                                    else
                                    {
                                        std::string string(subject);

                                        for(idx = (rc - 1); idx >= 1; idx--)
                                        {
                                            string.replace(ovector[2*idx], (ovector[2*idx+1] - ovector[2*idx]), &logItem->text);
                                        }

                                        fputs(string.c_str(), stdout);
                                    }
                                }
                                else
                                {
                                    fputs(subject, stdout);
                                }
                                fputs("\n", stdout);
                            }
                            else if(global_unique || ((replace % 10) == 3))
                            {
                                CHItem hItem;
                                PLogItem logItem;

                                memset(&hItem, 0, sizeof(hItem));

                                uniquekey_len = StringFormatPCRE(1, uniquekey, subject, namecount_unique, rc2, ovector2, name_table_unique, name_entry_size_unique);

                                logItem = (PLogItem)chtbl_lookup(&htbl, uniquekey, uniquekey_len, NULL);

                                if(!logItem)
                                {
                                    copy_len = StringFormatPCRE(0, copy, subject, namecount_filter, rc, ovector, name_table_filter, name_entry_size_filter);
                                    logItem = (PLogItem)malloc(sizeof(LogItem) + copy_len);
                                    logItem->count = 0;
                                    memset(&logItem->text, 0, copy_len + 1);
                                    memcpy(&logItem->text, copy, copy_len);
                                    chtbl_insert(&htbl, uniquekey, uniquekey_len, logItem);
                                }

                                logItem->count++;
                            }
                            else if(global_aggregate)
                            {
                                CHItem hItem;
                                PLogItemAgg logItem;

                                memset(&hItem, 0, sizeof(hItem));

                                uniquekey_len = StringFormatPCRE(1, uniquekey, subject, namecount_unique, rc2, ovector2, name_table_unique, name_entry_size_unique);

                                logItem = (PLogItemAgg)chtbl_lookup(&htbl, uniquekey, uniquekey_len, NULL);

                                if(!logItem)
                                {
                                    logItem = (PLogItemAgg)malloc(sizeof(LogItemAgg));
                                    memset(logItem, 0, sizeof(LogItemAgg));
                                    chtbl_insert(&htbl, uniquekey, uniquekey_len, logItem);
                                }

                                AggregateFormatPCRE(logItem, subject, namecount_filter, ovector, name_table_filter, name_entry_size_filter, fieldOffset);
                                logItem->count++;
                                logItem->currCount++;
                            }
                            else
                            {
                                StringFormatPCRE(1, tmpUniquekey, subject, namecount_unique, rc2, ovector2, name_table_unique, name_entry_size_unique);
                                if(strcmp(uniquekey, tmpUniquekey))
                                {
                                    if(uniquekey[0])
                                    {
                                        if(namecount_filter)
                                            fprintf(stdout, "Key<%s>, Count<%.4d>, %s\n", uniquekey, uniquecount, copy);
                                        else
                                        {
                                            if(rc == 1)
                                            {
                                                fprintf(stdout, "[%s][%.4d] --->> %s\n", uniquekey, uniquecount, copy);
                                            }
                                            else
                                            {
                                                fprintf(stdout, "%s\t%.4d\t%s\n", uniquekey, uniquecount, copy);
                                            }
                                        }
                                    }

                                    StringFormatPCRE(0, copy, subject, namecount_filter, rc, ovector, name_table_filter, name_entry_size_filter);
                                    StringFormatPCRE(1, uniquekey, subject, namecount_unique, rc2, ovector2, name_table_unique, name_entry_size_unique);
                                    uniquecount = 1;
                                }
                                else
                                {
                                    uniquecount++;
                                }
                            }
                        }
                    }
                    else
                    {
                        if(rc == 1)
                        {
                            if(!no_line_numbers)
                                fprintf(stdout, "%u --->> ", line_number);

                            fputs(subject, stdout);
                            fputs("\n", stdout);
                        }
                        else
                        {
                            if(namecount_filter)
                            {
                                fprintf(stdout, "Line<%u>", line_number);

                                tabptr = name_table_filter;
                                for (idx = 0; idx < namecount_filter; idx++)
                                {
                                    int n = (tabptr[0] << 8) | tabptr[1];

                                    fprintf(stdout, ", %s<%.*s>", tabptr + 2, ovector[2*n+1] - ovector[2*n], subject + ovector[2*n]);
                                    tabptr += name_entry_size_filter;
                                }
                                fputs("\n", stdout);
                            }
                            else
                            {
                                if(!no_line_numbers)
                                    fprintf(stdout, "%u\t", line_number);

                                for(idx = 1; idx < rc; idx++)
                                {
                                    char *substring_start = subject + ovector[2*idx];
                                    int substring_length = ovector[2*idx+1] - ovector[2*idx];

                                    fprintf(stdout, "%s%.*s", (idx > 1 ? "\t" : ""), substring_length, substring_start);
                                }
                                fputs("\n", stdout);
                            }
                        }

                    }
                }
            }
            else
            {
                if(file && (file != stdin))  fclose(file);
                file = NULL;

                if(global_aggregate_linked && ((argc - 1) >= 3))
                {
                    filter = argv[1];
                    unique = argv[2];
                    filename = argv[3];
                    argc -= 3;
                    argv += 3;
                    file = fopen(filename, "rb");
                    if(filter_re) pcre_free(filter_re);
                    if(unique_re) pcre_free(unique_re);
                }
                break;
            }
        }
    }

    if(file && (file != stdin))  fclose(file);
    file = NULL;

    if((replace % 10) == 3)
    {
        replace++;
        file = origFile;
        filter = argv[1];
        unique = argv[2];

        goto TheMainWhileLoop;
    }
    else if(global_unique)
    {
        int tableSize = chtbl_size(&htbl);
        CHItem* itemTable = (CHItem*)malloc(sizeof(CHItem)*tableSize);

        memset(itemTable, 0, sizeof(CHItem)*tableSize);

        chtbl_for_each_call(&htbl, FillHashItemArray,itemTable);

        qsort(itemTable,
            tableSize,
            sizeof(CHItem),
            compareHashItem);

        for(idx = 0; idx < tableSize; idx++)
        {
            PLogItem logItem = (PLogItem)itemTable[idx].value;

            if(namecount_filter)
            {
                fprintf(stdout, "Key<%.*s>, Count<%.4d>, %s\n", itemTable[idx].keySize, itemTable[idx].key,
                    logItem->count, &logItem->text);
            }
            else
            {
                if(lg_rc == 1)
                {
                    fprintf(stdout, "[%.*s][%.4d] --->> %s\n", itemTable[idx].keySize, itemTable[idx].key,
                        logItem->count, &logItem->text);
                }
                else
                {
                    fprintf(stdout, "%.*s\t%.4d\t%s\n", itemTable[idx].keySize, itemTable[idx].key,
                        logItem->count, &logItem->text);
                }
            }
        }
    }
    else if(global_aggregate)
    {
        int tableSize = chtbl_size(&htbl);
        CHItem* itemTable = (CHItem*)malloc(sizeof(CHItem)*tableSize);

        memset(itemTable, 0, sizeof(CHItem)*tableSize);

        chtbl_for_each_call(&htbl, FillHashItemArray,itemTable);

        qsort(itemTable,
            tableSize,
            sizeof(CHItem),
            compareHashItem);

        for(idx = 0; idx < tableSize; idx++)
        {
            PLogItemAgg logItem = (PLogItemAgg)itemTable[idx].value;

            if(namecount_filter + fieldOffset)
            {
                fprintf(stdout, "Key<%.*s>, Count<%.4d>", itemTable[idx].keySize, itemTable[idx].key,
                    logItem->count);

                PrintAggregate(logItem, stdout, namecount_filter + fieldOffset);
            }
        }
    }
    else
    {
        if(unique && uniquecount)
        {
            if(namecount_filter)
                fprintf(stdout, "Key<%s>, Count<%.4d>, %s\n", uniquekey, uniquecount, copy);
            else
            {
                if(lg_rc == 1)
                {
                    fprintf(stdout, "[%s][%.4d] --->> %s\n", uniquekey, uniquecount, copy);
                }
                else
                {
                    fprintf(stdout, "%s\t%.4d\t%s\n", uniquekey, uniquecount, copy);
                }
            }
        }
    }

    if(unique) chtbl_destroy(&htbl);
    if(filter_re) pcre_free(filter_re);
    if(unique_re) pcre_free(unique_re);

    return 0;
}

void* FillHashItemArray(void *data, void *userdata)
{
    CHItem* pItem = (CHItem*)data;
    CHItem* pItemTable = (CHItem*)userdata;

    memcpy(pItemTable, pItem, sizeof(CHItem));

    return ++pItemTable;
}

int compareHashItem(const void* a, const void* b)
{
    CHItem* arg1 = (CHItem*)a;
    CHItem* arg2 = (CHItem*)b;

    int compare = memcmp(arg1->key, arg2->key, min(arg1->keySize, arg2->keySize));

    return compare;
}

int StringFormatPCRE(int isKey, char* copy, char* subject, int namecount, int retCode, int* ovector, unsigned char *name_table, int name_entry_size)
{
    int idx;
    int copy_len = 0;
    unsigned char *tabptr;

    if(namecount)
    {
        tabptr = name_table;
        for (idx = 0; idx < namecount; idx++)
        {
            int n = (tabptr[0] << 8) | tabptr[1];

            if(idx > 0) copy_len += sprintf(&copy[copy_len], isKey ? ":" : ", ");

            if(isKey)
                copy_len += sprintf(&copy[copy_len], "%.*s",  ovector[2*n+1] - ovector[2*n], subject + ovector[2*n]);
            else
                copy_len += sprintf(&copy[copy_len], "%s<%.*s>", tabptr + 2, ovector[2*n+1] - ovector[2*n], subject + ovector[2*n]);

            tabptr += name_entry_size;
        }
    }
    else
    {
        if(retCode == 1)
        {
            if(isKey)
            {
                char *substring_start = subject + ovector[0];
                int substring_length = ovector[1] - ovector[0];
                copy_len += sprintf(&copy[copy_len], "%.*s", substring_length, substring_start);
            }
            else
            {
                copy_len += sprintf(&copy[copy_len], "%s", subject);
                if(copy_len && copy[copy_len-1] == '\n') copy[--copy_len] = 0;
            }
        }
        else
        {
            for(idx = 1; idx < retCode; idx++)
            {
                char *substring_start = subject + ovector[2*idx];
                int substring_length = ovector[2*idx+1] - ovector[2*idx];

                if(idx > 1) copy_len += sprintf(&copy[copy_len], isKey ? ":" : "\t");

                copy_len += sprintf(&copy[copy_len], "%.*s", substring_length, substring_start);
            }
        }
    }
    return copy_len;
}

int SimpleStringFormatPCRE(FILE* copy, char* subject, int retCode, int* ovector)
{
    int idx;
    int copy_len = 0;

    if(full_line || (retCode == 1))
    {
        char *substring_start = subject + ovector[0];
        int substring_length = ovector[1] - ovector[0];
        if(no_decoration)
            copy_len += fprintf(copy, "%.*s", substring_length, substring_start);
        else
            copy_len += fprintf(copy, "[%.*s]", substring_length, substring_start);
    }
    else
    {
        if(!no_decoration) fputs("[", copy);
        for(idx = 1; idx < retCode; idx++)
        {
            char *substring_start = subject + ovector[2*idx];
            int substring_length = ovector[2*idx+1] - ovector[2*idx];
            if(!no_decoration && (idx > 1)) copy_len += fprintf(copy, ",");
            if(no_decoration) 
                copy_len += fprintf(copy, "%.*s", substring_length, substring_start);
            else
                copy_len += fprintf(copy, "<%.*s>", substring_length, substring_start);
        }
        if(!no_decoration) fputs("]", copy);
    }
    fflush(copy);

    return copy_len;
}

std::string& ReplacePCRE(std::string & string, int allow_anonymous, const char* subject, int namecount, int retCode, int* ovector, unsigned char *name_table, int name_entry_size)
{
    int idx;
    int copy_len = 0;
    unsigned char *tabptr;
    char repStr[128+1];

    if(namecount)
    {
        tabptr = name_table;
        for (idx = 0; idx < namecount; idx++)
        {
            int n = (tabptr[0] << 8) | tabptr[1];
            const char *substring_start = subject + ovector[2*n];
            int substring_length = ovector[2*n+1] - ovector[2*n];
            const char* name = (char*)(tabptr + 2);

            sprintf(repStr, "(?P=%s)", name);

            if(substring_length >= 0)
            {
                const char* sep = strchr(name, '_');
                if(sep)
                {
                    if(!strcmp(sep+1, "UCASE"))
                        GLString::Replace(string, repStr, GLString(substring_start, substring_length).ToUpper().std_str());
                    else if(!strcmp(sep+1, "LCASE"))
                        GLString::Replace(string, repStr, GLString(substring_start, substring_length).ToLower().std_str());
                    else if(!strncmp(sep+1, "SPC_", 4))
                    {
                        char* stop;
                        name = sep+1;
                        sep = strchr(name, '_');

                        int pad = strtol(sep+1, &stop, 10);
                        GLString::Replace(string, repStr, Format("%-*.*s", pad, pad, GLString(substring_start, substring_length).c_str()));
                    }
                    else 
                    {
                        name = sep+1;
                        sep = strchr(name, '_');

                        if(sep && (*(sep+1) >= '0') && (*(sep+1) <= '9'))
                        {
                            double value = strtod(GLString(substring_start, substring_length).c_str(), NULL);

                            if(!strncmp(name, "PAD_", 4))
                            {
                                char* stop;

                                int pad = strtol(sep+1, &stop, 10);
                                int prec = (stop[0] = '_') ? strtol(stop+1, NULL, 10) : 0;

                                GLString::Replace(string, repStr, Format("%0*.*f", pad, prec, value));
                            }
                            else
                            {
                                if(!strncmp(name, "BASE_", 5))
                                {
                                    int base = strtol(sep+1, NULL, 10);
                                    value = strtol(GLString(substring_start, substring_length).c_str(), NULL, base);
                                }
                                else if(!strncmp(name, "ADD_", 4))
                                    value += strtol(sep+1, NULL, 10);
                                else if(!strncmp(name, "SUB_", 4))
                                    value -= strtol(sep+1, NULL, 10);
                                else if(!strncmp(name, "MUL_", 4))
                                    value *= strtol(sep+1, NULL, 10);
                                else if(!strncmp(name, "DIV_", 4))
                                    value /= strtol(sep+1, NULL, 10);
                                else if(!strncmp(name, "MOD_", 4))
                                    value = ((long)value) % strtol(sep+1, NULL, 10);
                                else if(!strncmp(name, "AND_", 4))
                                    value = ((long)value) & strtol(sep+1, NULL, 10);
                                else if(!strncmp(name, "XOR_", 3))
                                    value = ((long)value) ^ strtol(sep+1, NULL, 10);
                                else if(!strncmp(name, "OR_", 3))
                                    value = ((long)value) | strtol(sep+1, NULL, 10);
                                else if(!strncmp(name, "LSHIFT_", 7))
                                    value = ((long)value) << strtol(sep+1, NULL, 10);
                                else if(!strncmp(name, "RSHIFT_", 7))
                                    value = ((long)value) >> strtol(sep+1, NULL, 10);

                                GLString::Replace(string, repStr, Format("%.*f", GetPrecision(value), value));
                            }
                        }
                    }
                }
                else
                    GLString::Replace(string, repStr, std::string(substring_start, substring_length));
            }
            else
                GLString::Replace(string, repStr, "");

            tabptr += name_entry_size;
        }
    }

    if(allow_anonymous)
    {
        for(idx = 0; idx < retCode; idx++)
        {
            const char *substring_start = subject + ovector[2*idx];
            int substring_length = ovector[2*idx+1] - ovector[2*idx];

            sprintf(repStr, "\\%d", idx);

            if(substring_length > 0)
                GLString::Replace(string, repStr, std::string(substring_start, substring_length));
            else
                GLString::Replace(string, repStr, "");
        }
    }

    return string;
}

enum AggType
{
    AggFirst,
    AggLast,
    AggMin,
    AggMax,
    AggAvg,
    AggSum,
    AggTMn,
    AggTMx
};

void AggregateFormatPCRE(PLogItemAgg pLogItem, char* subject, int namecount, int* ovector, unsigned char *name_table, int name_entry_size, int fieldOffset)
{
    int idx;
    unsigned char *tabptr;
    int                fieldLen;
    int                fieldNameLen;
    char*            fieldName;
    char*            fieldValue;
    enum AggType    fieldType;
    char            textCopy[65536+1];
    double            numberCopy;
    if(namecount)
    {
        tabptr = name_table;
        for (idx = fieldOffset; idx < (namecount + fieldOffset); idx++)
        {
            int n = (tabptr[0] << 8) | tabptr[1];

            fieldType = AggFirst;
            fieldName = (char*)(tabptr + 2);
            fieldNameLen = (int)strlen(fieldName);
            fieldLen = (int)(ovector[2*n+1] - ovector[2*n]);
            fieldValue = (char*)(subject + ovector[2*n]);

            memcpy(textCopy, fieldValue, fieldLen);
            textCopy[fieldLen] = 0;

            if(fieldNameLen > 4)
            {
                if(fieldName[fieldNameLen-4] == '_')
                {
                    if((fieldName[fieldNameLen-3] == 'M') && (fieldName[fieldNameLen-2] == 'I') && (fieldName[fieldNameLen-1] == 'N'))
                    {
                        fieldType = AggMin;
                    }
                    else if((fieldName[fieldNameLen-3] == 'M') && (fieldName[fieldNameLen-2] == 'A') && (fieldName[fieldNameLen-1] == 'X'))
                    {
                        fieldType = AggMax;
                    }
                    else if((fieldName[fieldNameLen-3] == 'A') && (fieldName[fieldNameLen-2] == 'V') && (fieldName[fieldNameLen-1] == 'G'))
                    {
                        fieldType = AggAvg;
                    }
                    else if((fieldName[fieldNameLen-3] == 'S') && (fieldName[fieldNameLen-2] == 'U') && (fieldName[fieldNameLen-1] == 'M'))
                    {
                        fieldType = AggSum;
                    }
                    else if((fieldName[fieldNameLen-3] == 'T') && (fieldName[fieldNameLen-2] == 'M') && (fieldName[fieldNameLen-1] == 'N'))
                    {
                        fieldType = AggTMn;
                    }
                    else if((fieldName[fieldNameLen-3] == 'T') && (fieldName[fieldNameLen-2] == 'M') && (fieldName[fieldNameLen-1] == 'X'))
                    {
                        fieldType = AggTMx;
                    }
                    else if((fieldName[fieldNameLen-3] == 'F') && (fieldName[fieldNameLen-2] == 'R') && (fieldName[fieldNameLen-1] == 'T'))
                    {
                        fieldType = AggFirst;
                    }
                    else if((fieldName[fieldNameLen-3] == 'L') && (fieldName[fieldNameLen-2] == 'S') && (fieldName[fieldNameLen-1] == 'T'))
                    {
                        fieldType = AggLast;
                    }
                }
            }

            if(!pLogItem->names[idx])
            {
                pLogItem->currCount = 0;
                pLogItem->names[idx] = (char*)malloc(fieldNameLen + 1);
                memcpy(pLogItem->names[idx], fieldName, fieldNameLen);
                pLogItem->names[idx][fieldNameLen] = 0;

                switch(fieldType)
                {
                case AggMin:
                case AggMax:
                case AggAvg:
                case AggSum:
                    pLogItem->items[idx].value = strtod(textCopy, NULL);
                    break;
                case AggFirst:
                case AggLast:
                case AggTMn:
                case AggTMx:
                default:
                    pLogItem->items[idx].text = (char*)malloc(fieldLen + 1);
                    memcpy(pLogItem->items[idx].text, fieldValue, fieldLen);
                    pLogItem->items[idx].text[fieldLen] = 0;
                    break;
                }
            }
            else
            {
                switch(fieldType)
                {
                case AggMin:
                case AggMax:
                case AggAvg:
                case AggSum:
                    numberCopy = strtod(textCopy, NULL);
                    break;
                }

                switch(fieldType)
                {
                case AggMin:
                    if(numberCopy < pLogItem->items[idx].value) pLogItem->items[idx].value = numberCopy;
                    break;
                case AggMax:
                    if(numberCopy > pLogItem->items[idx].value) pLogItem->items[idx].value = numberCopy;
                    break;
                case AggAvg:
                    pLogItem->items[idx].value = ((pLogItem->items[idx].value * pLogItem->currCount) + numberCopy) / (pLogItem->currCount + 1);
                    break;
                case AggSum:
                    pLogItem->items[idx].value += numberCopy;
                    break;
                case AggTMn:
                    if(strcmp(textCopy, pLogItem->items[idx].text) < 0)
                    {
                        free(pLogItem->items[idx].text);
                        pLogItem->items[idx].text = (char*)malloc(fieldLen + 1);
                        memcpy(pLogItem->items[idx].text, fieldValue, fieldLen);
                        pLogItem->items[idx].text[fieldLen] = 0;
                    }
                    break;
                case AggTMx:
                    if(strcmp(textCopy, pLogItem->items[idx].text) > 0)
                    {
                        free(pLogItem->items[idx].text);
                        pLogItem->items[idx].text = (char*)malloc(fieldLen + 1);
                        memcpy(pLogItem->items[idx].text, fieldValue, fieldLen);
                        pLogItem->items[idx].text[fieldLen] = 0;
                    }
                    break;
                case AggLast:
                    pLogItem->items[idx].text = (char*)realloc(pLogItem->items[idx].text, fieldLen + 1);
                    memcpy(pLogItem->items[idx].text, fieldValue, fieldLen);
                    pLogItem->items[idx].text[fieldLen] = 0;
                    break;
                }
            }

            tabptr += name_entry_size;
        }
    }
}

int PrintAggregate(PLogItemAgg pLogItem, FILE* file, int namecount)
{
    int idx;
    int copy_len = 0;
    int                fieldNameLen;
    char*            fieldName;
    enum AggType    fieldType;

    if(namecount)
    {
        for (idx = 0; idx < namecount; idx++)
        {
            fieldType = AggFirst;
            fieldName = pLogItem->names[idx];
            if(!fieldName) continue;
            fieldNameLen = (int)strlen(fieldName);

            if(fieldNameLen > 4)
            {
                if((fieldName[fieldNameLen-4] == '_') && (fieldName[fieldNameLen-3] == 'M') && (fieldName[fieldNameLen-2] == 'I') && (fieldName[fieldNameLen-1] == 'N'))
                {
                    fieldType = AggMin;
                }
                else if((fieldName[fieldNameLen-4] == '_') && (fieldName[fieldNameLen-3] == 'M') && (fieldName[fieldNameLen-2] == 'A') && (fieldName[fieldNameLen-1] == 'X'))
                {
                    fieldType = AggMax;
                }
                else if((fieldName[fieldNameLen-4] == '_') && (fieldName[fieldNameLen-3] == 'A') && (fieldName[fieldNameLen-2] == 'V') && (fieldName[fieldNameLen-1] == 'G'))
                {
                    fieldType = AggAvg;
                }
                else if((fieldName[fieldNameLen-4] == '_') && (fieldName[fieldNameLen-3] == 'S') && (fieldName[fieldNameLen-2] == 'U') && (fieldName[fieldNameLen-1] == 'M'))
                {
                    fieldType = AggSum;
                }
                else if((fieldName[fieldNameLen-4] == '_') && (fieldName[fieldNameLen-3] == 'T') && (fieldName[fieldNameLen-2] == 'M') && (fieldName[fieldNameLen-1] == 'M'))
                {
                    fieldType = AggTMn;
                }
                else if((fieldName[fieldNameLen-4] == '_') && (fieldName[fieldNameLen-3] == 'T') && (fieldName[fieldNameLen-2] == 'M') && (fieldName[fieldNameLen-1] == 'X'))
                {
                    fieldType = AggTMx;
                }
                else if((fieldName[fieldNameLen-4] == '_') && (fieldName[fieldNameLen-3] == 'F') && (fieldName[fieldNameLen-2] == 'R') && (fieldName[fieldNameLen-1] == 'T'))
                {
                    fieldType = AggFirst;
                }
                else if((fieldName[fieldNameLen-4] == '_') && (fieldName[fieldNameLen-3] == 'L') && (fieldName[fieldNameLen-2] == 'S') && (fieldName[fieldNameLen-1] == 'T'))
                {
                    fieldType = AggLast;
                }
            }

            fprintf(file, ", %.*s", fieldNameLen, fieldName);

            switch(fieldType)
            {
            case AggMin:
            case AggMax:
            case AggAvg:
            case AggSum:
                fprintf(file, "<%.*f>", GetPrecision(pLogItem->items[idx].value), pLogItem->items[idx].value);
                break;
            case AggFirst:
            case AggLast:
            case AggTMn:
            case AggTMx:
            default:
                fprintf(file, "<%s>", pLogItem->items[idx].text);
                break;
            }
        }
        fprintf(file, "\n");

    }

    return copy_len;
}

int GetPrecision(double dbl)
{
    int prec = 0;
    char* dot;
    char buffer[128+1];
    size_t len;

    len = sprintf(buffer, "%.8f", dbl);

    while(len && (buffer[len-1] == '0')) buffer[--len] = 0;

    dot = strrchr(buffer, '.');
    if(dot) prec = (int)((&buffer[0] + len) - (dot + 1));

    return prec;
}

int DeLiner(FILE* file, int mode, int show_all_lines, char* start, char* end, int inc_count, char** inc_patterns, int exc_count, char** exc_patterns, int options, int compile_input, int single_head)
{
    pcre *start_re = NULL;
    pcre *end_re = NULL;
    pcre_extra *start_ex = NULL;
    pcre_extra *end_ex = NULL;

    pcre*           inc_pcre[MAX_PATTERN_COUNT];
    pcre_extra*     inc_ext[MAX_PATTERN_COUNT];
    pcre*           exc_pcre[MAX_PATTERN_COUNT];
    pcre_extra*     exc_ext[MAX_PATTERN_COUNT];

    const char *error;
    int erroffset;

    int idx, jdx;
    char* subject  = NULL;
    char* head = (char*)malloc(gLineSize+1);
    char* line = (char*)malloc(gLineSize+1);
    unsigned int subject_length = 0;
    unsigned int line_number = 0;
    unsigned int logical_line_number = 0;
    unsigned int startLine = 0;
    unsigned int includeLine = 0;
    unsigned int endLine = 0;
    unsigned int headLine = 0;

    int start_ovector[OVECCOUNT], start_ovector_save[OVECCOUNT], end_ovector[OVECCOUNT], include_ovector[OVECCOUNT], exclude_ovector[OVECCOUNT];
    int start_rc, start_rc_save, end_rc, include_rc, exclude_rc;

    ResetVector(start_ovector, OVECCOUNT, true);
    ResetVector(start_ovector_save, OVECCOUNT, true);
    ResetVector(end_ovector, OVECCOUNT, true);
    ResetVector(include_ovector, OVECCOUNT, true);
    ResetVector(exclude_ovector, OVECCOUNT, true);

    start_rc = start_rc_save = end_rc = include_rc = exclude_rc = -1;

    if(start && !start[0]) start = NULL;
    if(end && !end[0]) end = NULL;

    if(!start) return 1;

    if(start)
    {
        start_re = compile_input ? LoadCompiledRegExp(start, &start_ex) :
            pcre_compile(start,    // the pattern
            options,                // default options
            &error,                 // for error message
            &erroffset,             // for error offset
            NULL);                  // use default character tables

    if(start_re == NULL)
    {
        fprintf(stderr, "LogProcessor: [start] compilation failed at offset %d: %s\n", erroffset, error);
        return 1;
    }
    if(!start_ex) start_ex = pcre_study(start_re, 0, &error);
    }

    if(end)
    {
        end_re = compile_input ? LoadCompiledRegExp(end, &end_ex) :
            pcre_compile(end,    // the pattern
            options,                // default options
            &error,                 // for error message
            &erroffset,             // for error offset
            NULL);                  // use default character tables
    if(end_re == NULL)
    {
        if(start_re) pcre_free(start_re);
        fprintf(stderr, "LogProcessor: [end] compilation failed at offset %d: %s\n", erroffset, error);
        return 1;
    }
    if(!end_ex) end_ex = pcre_study(end_re, 0, &error);
    }


    memset(inc_pcre, 0, sizeof(inc_pcre));
    memset(inc_ext, 0, sizeof(inc_ext));
    if(inc_count > 0)
    {
        for(jdx = idx = 0; idx < inc_count; idx++)
        {
            inc_pcre[jdx] = compile_input ? LoadCompiledRegExp(inc_patterns[idx], &inc_ext[jdx]) :
                pcre_compile(inc_patterns[idx], // the pattern
                options,                        // default options
                &error,                         // for error message
                &erroffset,                     // for error offset
                NULL);                          // use default character tables
        if(inc_pcre[idx] == NULL)
        {
            fprintf(stderr, "LogProcessor: [include(%d)] compilation failed at offset %d: %s\n", idx, erroffset, error);
            continue;
        }
        if(!inc_ext[jdx]) inc_ext[jdx] = pcre_study(inc_pcre[jdx], 0, &error);
        jdx++;
        }
    }

    memset(exc_pcre, 0, sizeof(exc_pcre));
    memset(exc_ext, 0, sizeof(exc_ext));
    if(exc_count > 0)
    {
        for(jdx = idx = 0; idx < exc_count; idx++)
        {
            exc_pcre[jdx] = compile_input ? LoadCompiledRegExp(exc_patterns[idx], &exc_ext[jdx]) :
                pcre_compile(exc_patterns[idx], // the pattern
                options,                        // default options
                &error,                         // for error message
                &erroffset,                     // for error offset
                NULL);                          // use default character tables
        if(exc_pcre[idx] == NULL)
        {
            fprintf(stderr, "LogProcessor: [exclude(%d)] compilation failed at offset %d: %s\n", idx, erroffset, error);
            continue;
        }
        if(!exc_ext[jdx]) exc_ext[jdx] = pcre_study(exc_pcre[jdx], 0, &error);
        jdx++;
        }
    }

    headLine = 0;
    startLine = 1;

    while(feof(file) != EOF)
    {
        int lineSize = gLineSize;
        subject = line;
        if(GET_LINE(subject, lineSize, file))
        {
            line_number++;
            includeLine = 0;
            endLine = 0;

            subject_length = (gLineSize - lineSize);
            while(subject_length)
            {
                if((subject[subject_length-1] == '\r' || subject[subject_length-1] == '\n'))
                    subject[--subject_length] = 0;
                else
                    break;
            }

            ResetVector(start_ovector, OVECCOUNT);
            start_rc = pcre_exec(start_re,      // the compiled pattern
                NULL,                 // no extra data - we didn't study the pattern
                subject,              // the subject string
                (int)subject_length,  // the length of the subject
                0,                    // start at offset 0 in the subject
                0,                    // default options
                start_ovector,        // output vector for substring information
                OVECCOUNT);           // number of elements in the output vector

            if(start_rc < 0)
            {
                if(start_rc != PCRE_ERROR_NOMATCH) break;
            }
            else
            {
                if(single_head && (start_rc_save >= 0))
                {
                    if(logical_line_number >= 1) fputs("\n", stdout);
                    SimpleStringFormatPCRE(stdout, head, start_rc_save, start_ovector_save);
                }

                strcpy(head, line);
                start_rc_save = start_rc;
                memcpy(start_ovector_save, start_ovector, sizeof(start_ovector_save));
                if(!startLine) logical_line_number++;
                startLine = 0;
                headLine = 0;
                continue;
            }

            if(!startLine)
            {
                if(inc_count)
                {
                    idx = 0;
                    while(inc_pcre[idx])
                    {
                        ResetVector(include_ovector, OVECCOUNT);
                        include_rc = pcre_exec(inc_pcre[idx],   // the compiled pattern
                            inc_ext[idx],             // extra data
                            subject,                  // the subject string
                            (int)subject_length,      // the length of the subject
                            0,                        // start at offset 0 in the subject
                            0,                        // default options
                            include_ovector,          // output vector for substring information
                            OVECCOUNT);               // number of elements in the output vector

                        if(include_rc >= 0) 
                        {
                            includeLine = 1;
                            break;
                        }
                        idx++;
                    }
                }
                if(includeLine && exc_count)
                {
                    idx = 0;
                    while(exc_pcre[idx])
                    {
                        ResetVector(exclude_ovector, OVECCOUNT);
                        exclude_rc = pcre_exec(exc_pcre[idx],   // the compiled pattern
                            exc_ext[idx],             // extra data
                            subject,                  // the subject string
                            (int)subject_length,      // the length of the subject
                            0,                        // start at offset 0 in the subject
                            0,                        // default options
                            exclude_ovector,          // output vector for substring information
                            OVECCOUNT);               // number of elements in the output vector

                        if(exclude_rc >= 0) 
                        {
                            includeLine = 0;
                            break;
                        }
                        idx++;
                    }
                }
                if(end_re)
                {
                    ResetVector(end_ovector, OVECCOUNT);
                    end_rc = pcre_exec(end_re,          // the compiled pattern
                        NULL,                 // no extra data - we didn't study the pattern
                        subject,              // the subject string
                        (int)subject_length,  // the length of the subject
                        0,                    // start at offset 0 in the subject
                        0,                    // default options
                        end_ovector,          // output vector for substring information
                        OVECCOUNT);           // number of elements in the output vector

                    if(end_rc >= 0) endLine = 1;
                }

                if(!headLine && (includeLine || endLine))
                {
                    if(logical_line_number >= 1) fputs("\n", stdout);
                    SimpleStringFormatPCRE(stdout, head, start_rc_save, start_ovector_save);
                    start_rc_save = -1;
                    headLine = 1;
                }
                if(includeLine)
                {
                    if(mode==2) 
                        fputs("\n", stdout);
                    else if(!no_decoration) 
                        fputs("\t", stdout);

                    SimpleStringFormatPCRE(stdout, line, include_rc, include_ovector);
                    include_rc = -1;
                }
                if(endLine)
                {
                    startLine = 1;
                    logical_line_number++;
                    if(!includeLine)
                    {
                        if(mode==2) 
                            fputs("\n", stdout);
                        else if(!no_decoration) 
                            fputs("\t", stdout);
                        SimpleStringFormatPCRE(stdout, line, end_rc, end_ovector);
                        end_rc = -1;
                    }
                }
            }
            else if((show_all_lines) && (mode == 1))
            {
                if(logical_line_number >= 1) fputs("\n", stdout);
                fputs(subject, stdout);
                logical_line_number++;
            }
        }
        else
        {
            break;
        }
    }
    if(logical_line_number) fputs("\n", stdout);

    if(single_head && (start_rc_save >= 0))
    {
        SimpleStringFormatPCRE(stdout, head, start_rc_save, start_ovector_save);
        if(logical_line_number) fputs("\n", stdout);
    }

    pcre_free(start_re);
    pcre_free(end_re);

    idx = 0;
    while(inc_pcre[idx]) pcre_free(inc_pcre[idx++]);

    idx = 0;
    while(exc_pcre[idx]) pcre_free(exc_pcre[idx++]);

    return 0;
}

pcre* LoadCompiledRegExp(char* regExpFileName, pcre_extra** pExtra)
{
    pcre *regExp = NULL;
    uschar sbuf[8];
    int true_size = 0;
    int true_study_size = 0;
    unsigned long int magic = 0;

    FILE* file = fopen(regExpFileName, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "LogProcessor: [compiled] failed to open %s: %s\n", regExpFileName, strerror(errno));
        return NULL;
    }

    if (fread(sbuf, 1, 8, file) != 8)
    {
        fprintf(stderr, "LogProcessor: [compiled] read failed on %s: %s\n", regExpFileName, strerror(errno));
        fclose(file);
        return NULL;
    }

    true_size = (sbuf[0] << 24) | (sbuf[1] << 16) | (sbuf[2] << 8) | sbuf[3];
    true_study_size = (sbuf[4] << 24) | (sbuf[5] << 16) | (sbuf[6] << 8) | sbuf[7];

    regExp = (real_pcre *)pcre_malloc(true_size);

    if (fread(regExp, 1, true_size, file) != true_size)
    {
        fprintf(stderr, "LogProcessor: [compiled] read failed on %s: %s\n", regExpFileName, strerror(errno));
        pcre_free(regExp);
        fclose(file);
        return NULL;
    }

    magic = ((real_pcre *)regExp)->magic_number;

    if (magic != MAGIC_NUMBER)
        if (byteflip(magic, sizeof(magic)) != MAGIC_NUMBER)
        {
            fprintf(stderr, "LogProcessor: [compiled] data in %s is not a compiled RegExp\n", regExpFileName);
            pcre_free(regExp);
            fclose(file);
            return NULL;
        }

        if(pExtra && true_study_size != 0)
        {
            pcre_study_data *psd;

            (*pExtra) = (pcre_extra*)pcre_malloc(sizeof(pcre_extra) + true_study_size);
            if(*pExtra)
            {
                (*pExtra)->flags = PCRE_EXTRA_STUDY_DATA;

                psd = (pcre_study_data *)(((char *)(*pExtra)) + sizeof(pcre_extra));
                (*pExtra)->study_data = psd;

                if (fread(psd, 1, true_study_size, file) != true_study_size)
                {
                    fprintf(stderr, "LogProcessor: [compiled] read failed on %s: %s\n", regExpFileName, strerror(errno));
                    pcre_free(*pExtra);
                    pcre_free(regExp);
                    fclose(file);
                    return NULL;
                }
            }
        }

        fclose(file);

        return regExp;
}

char DecodeCString(char* szCHex, int* pCount)
{
    char ch = 0;
    int count = 0;
    char szHex[3];
    char szOct[4];

    if(!szCHex)
    {
        if(pCount) (*pCount) = count;
        return ch;
    }

    count++;

    if(szCHex[0] != '\\')
    {
        if(pCount) (*pCount) = count;
        return szCHex[0];
    }

    switch(szCHex[1])
    {
    case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
        count += 1;
        szOct[0] = szCHex[1];
        szOct[1] = 0;
        if(szCHex[2] >= '0' && szCHex[2] <= '7')
        {
            count += 1;
            szOct[1] = szCHex[2];
            szOct[2] = 0;
            if(szCHex[3] >= '0' && szCHex[3] <= '7')
            {
                count += 1;
                szOct[2] = szCHex[3];
                szOct[3] = 0;
            }
        }
        ch = (char)strtoul(szOct, NULL, 8);
        break;
    case 'a':
        count++; ch = '\a'; break;
    case 'b':
        count++; ch = '\b'; break;
    case 't':
        count++; ch = '\t'; break;
    case 'n':
        count++; ch = '\n'; break;
    case 'v':
        count++; ch = '\v'; break;
    case 'f':
        count++; ch = '\f'; break;
    case 'r':
        count++; ch = '\r'; break;
    case '\"':
        count++; ch = '\"'; break;
    case '\'':
        count++; ch = '\''; break;
    case '\?':
        count++; ch = '\?'; break;
    case '\\':
        count++; ch = '\\'; break;
    case 'x':
        count += 3;
        szHex[0] = szCHex[2];
        szHex[1] = szCHex[3];
        szHex[2] = 0;
        ch = (char)strtoul(szHex, NULL, 16);
        break;
    }

    if(pCount) (*pCount) = count;

    return ch;
}
