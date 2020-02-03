#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

int ProcessStream(FILE* inFile, FILE* outFile, int maxLine = 64);

int PrintToFile(FILE * _File, const char * _Format, ...)
{
    int _Ret;
    va_list _ArgList;
    static char buffer[(1024*64)+1];

    va_start(_ArgList, _Format);
    if(!_File)
        _Ret = vsnprintf(buffer, (sizeof(buffer)-1),_Format, _ArgList);
    else
    {
        _Ret = vfprintf(_File, _Format, _ArgList);
        fflush(_File);
    }
    va_end(_ArgList);
    return _Ret;
}

int main(int argc, char** argv)
{
    if(argc == 2)
    {
        FILE* inFile = fopen(argv[1], "rb");
        if(inFile)
        {
            int maxLine = ProcessStream(inFile, NULL, 0);
            fseek(inFile, 0, SEEK_SET);
            ProcessStream(inFile, stdout, maxLine + 4);
            fclose(inFile);
        }
    }
    else
    {
        fprintf(stderr, "\nUsage: %s [InputFile]\n\n", argv[0]);
    }
    return 0;
}

int ProcessStream(FILE* inFile, FILE* outFile, int maxLine)
{
    size_t  len = 0;
    char    buffer[(1024*64)+1];

    short   currentDepth, stackDepth = -1;
    char    nameStack[8][1024];
    int     maxSizeStack[8];
    int     minSizeStack[8];
    char*   spaceBuff;
    int     spaceSize;

    char    *savePos, *pos;
    int     maxWrite = 0;
    int     minMsgSize, maxMsgSize;
    int     maxNumberSize, maxTextSize;
    int     totalMaxMsgSize;

    short   writeCount, idx, jdx, kdx;
    size_t  structCount = 0;
    size_t  blockComment = 0;

    char*   name;
    char    type;
    short   maxSize;
    short   minSize;
    char*   desc;

    char*   printfFormat;
    char*   printfArgs;

    spaceBuff = (char*)malloc(maxLine+1);
    memset(spaceBuff, ' ', maxLine);
    spaceBuff[maxLine] = 0;

    printfFormat = (char*)malloc(1024*64);
    printfArgs = (char*)malloc(1024*64);

    totalMaxMsgSize = maxNumberSize = maxTextSize = 0;

    PrintToFile(outFile, "\n");
    PrintToFile(outFile, "#pragma pack(1)\n");
    PrintToFile(outFile, "\n");
    PrintToFile(outFile, "\n");
    PrintToFile(outFile, "typedef struct\n");
    PrintToFile(outFile, "{\n");
    PrintToFile(outFile, "    char data;\n");
    PrintToFile(outFile, "} T_TXT;\n");
    PrintToFile(outFile, "typedef struct\n");
    PrintToFile(outFile, "{\n");
    PrintToFile(outFile, "    char data;\n");
    PrintToFile(outFile, "} T_NUM;\n");
    PrintToFile(outFile, "\n");

    while(feof(inFile) != EOF)
    {
        currentDepth = 0;
        if(fgets(buffer, sizeof(buffer)-1, inFile))
        {
            len = strlen(buffer);
            while(buffer[len -1] == '\n' || buffer[len -1] == '\r') buffer[(len--) -1] = 0;
            pos = &buffer[0];
            while(*pos == ' ') {pos++;}
            while(*pos == '\t') {currentDepth++; pos++;}

            if(pos[0] == '/' && pos[1] == '/') continue;
            if(pos[0] == '/' && pos[1] == '*') {blockComment = 1; continue;}
            if(pos[0] == '*' && pos[1] == '/') {blockComment = 0; continue;}
            if(blockComment) continue;

            if(!*pos) continue;
            if(!currentDepth)
            {
                if(structCount)
                    for(idx = stackDepth; idx >= 0; idx--)
                    {
                        PrintToFile(outFile, "\n");
                        for(jdx = 0; jdx < idx; jdx++) PrintToFile(outFile, "    ");
                        if(idx)
                        {
                            if(maxSizeStack[idx] > 1)
                                PrintToFile(outFile, "} %s[%d];\n", nameStack[idx], maxSizeStack[idx]);
                            else
                                PrintToFile(outFile, "} %s;\n", nameStack[idx]);
                        }
                        else
                        {
                            PrintToFile(outFile, "} %s, *P%s;\n", nameStack[idx], nameStack[idx]);
                            PrintToFile(outFile, "\n");
                            PrintToFile(outFile, "#define SIZEOF_%s_MIN (%d)\n", nameStack[idx], minMsgSize);
                            PrintToFile(outFile, "#define SIZEOF_%s_MAX (%d)\n", nameStack[idx], maxMsgSize);
                            PrintToFile(outFile, "#define %s_PRINTF_FORMAT \"%%s\"\"%s\"\"%%s\"\n", nameStack[idx], printfFormat);
                            PrintToFile(outFile, "#define %s_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix%s, suffix\n", nameStack[idx], printfArgs);
                            if(maxMsgSize > totalMaxMsgSize) totalMaxMsgSize = maxMsgSize;
                        }
                    }

                maxMsgSize = minMsgSize = 0;
                savePos = pos;
                while(*pos != ' ') pos++;
                if(!*pos) continue;
                stackDepth = currentDepth;
                memcpy(&nameStack[stackDepth][0], savePos, pos - savePos);
                nameStack[stackDepth][pos - savePos] = 0;
                maxSizeStack[stackDepth] = 1;
                minSizeStack[stackDepth] = 1;
                PrintToFile(outFile, "\n");
                writeCount = PrintToFile(outFile, "typedef struct tag%s", nameStack[0]);
                spaceSize = (maxLine - writeCount); if(spaceSize < 0) spaceSize = 0;
                if(writeCount > maxWrite) maxWrite = writeCount;
                writeCount += PrintToFile(outFile, "%.*s//%s\n", spaceSize, spaceBuff, pos);
                PrintToFile(outFile, "{\n");
                structCount++;
                printfFormat[0] = 0;
                printfArgs[0] = 0;
            }
            else
            {
                savePos = pos;
                while(*pos != ' ') pos++;
                if(!*pos) continue;
                *pos++ = 0;
                name = savePos;

                while(*pos == ' ') pos++;
                if(!*pos) continue;
                type = *pos;

                while(*pos != '(') pos++;
                if(!*pos) continue;
                while(*pos < '0' || *pos > '9') pos++;
                if(!*pos) continue;
                minSize = (short)atol(pos);

                while(*pos >= '0' && *pos <= '9') pos++;
                if(!*pos) continue;
                if(*pos == ',')
                {
                    while(*pos < '0' || *pos > '9') pos++;
                    if(!*pos) continue;
                    maxSize = (short)atol(pos);
                }
                else
                    maxSize = minSize;

                while(*pos != ' ') pos++;
                if(!*pos) continue;
                desc = pos;

                while(currentDepth < (stackDepth+1))
                {
                    PrintToFile(outFile, "\n");
                    for(jdx = 0; jdx < stackDepth; jdx++) PrintToFile(outFile, "    ");
                    if(stackDepth)
                    {
                        if(maxSizeStack[stackDepth] > 1)
                            PrintToFile(outFile, "} %s[%d];\n", nameStack[stackDepth], maxSizeStack[stackDepth]);
                        else
                            PrintToFile(outFile, "} %s;\n", nameStack[stackDepth]);
                    }
                    else
                    {
                        PrintToFile(outFile, "} %s, *P%s;\n", nameStack[stackDepth], nameStack[stackDepth]);
                        PrintToFile(outFile, "\n");
                        PrintToFile(outFile, "#define SIZEOF_%s_MIN (%d)\n", nameStack[stackDepth], minMsgSize);
                        PrintToFile(outFile, "#define SIZEOF_%s_MAX (%d)\n", nameStack[stackDepth], maxMsgSize);
                        PrintToFile(outFile, "#define %s_PRINTF_FORMAT \"%%s\"\"%s\"\"%%s\"\n", nameStack[stackDepth], printfFormat);
                        PrintToFile(outFile, "#define %s_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix%s, suffix\n", nameStack[stackDepth], printfArgs);
                        if(maxMsgSize > totalMaxMsgSize) totalMaxMsgSize = maxMsgSize;
                    }
                    stackDepth--;
                }

                writeCount = 0;
                for(idx = 0; idx < currentDepth; idx++) writeCount += PrintToFile(outFile, "    ");

                switch(type)
                {
                case '9':
                case 'X':
                    {
                        const char* TYPE = "T_TXT";
                        if(type == '9') TYPE = "T_NUM";

                        for(idx = 0, kdx = 1; idx <= stackDepth; idx++) kdx *= maxSizeStack[idx];
                        maxMsgSize += (maxSize * kdx);
                        for(idx = 0, kdx = 1; idx <= stackDepth; idx++) kdx *= minSizeStack[idx];
                        minMsgSize += (maxSize * kdx);

                        if(maxSize == 1)
                        {
                            if(!(stackDepth && (maxSizeStack[1] > 1)))
                            {
                                if(strcmp(name, "ETX"))
                                {
                                    sprintf(printfArgs + strlen(printfArgs), ", seperator, &obj->");
                                    for(idx = 1; idx <= stackDepth; idx++)
                                        sprintf(printfArgs + strlen(printfArgs), "%s.", nameStack[idx]);
                                    sprintf(printfArgs + strlen(printfArgs), "%s.data", name);
                                }
                            }
                        }
                        else
                        {
                            if(!(stackDepth && (maxSizeStack[1] > 1)))
                            {
                                if(strcmp(name, "ETX"))
                                {
                                    sprintf(printfArgs + strlen(printfArgs), ", seperator, obj->");
                                    for(idx = 1; idx <= stackDepth; idx++)
                                        sprintf(printfArgs + strlen(printfArgs), "%s.", nameStack[idx]);
                                    sprintf(printfArgs + strlen(printfArgs), "%s", name);
                                }
                            }
                        }

                        if(maxSize == 1)
                        {
                            if(!(stackDepth && (maxSizeStack[1] > 1)))
                            {
                                if(strcmp(name, "ETX"))
                                {
                                    sprintf(printfFormat + strlen(printfFormat), "%%s");
                                    for(idx = 1; idx <= stackDepth; idx++)
                                        sprintf(printfFormat + strlen(printfFormat), "%s.", nameStack[idx]);
                                    sprintf(printfFormat + strlen(printfFormat), "%s", name);
                                    sprintf(printfFormat + strlen(printfFormat), "[%%.%ds];", maxSize);
                                }
                            }

                            writeCount += PrintToFile(outFile, "%s %s;", TYPE, name);
                        }
                        else
                        {
                            if(!(stackDepth && (maxSizeStack[1] > 1)))
                            {
                                if(strcmp(name, "ETX"))
                                {
                                    sprintf(printfFormat + strlen(printfFormat), "%%s");
                                    for(idx = 1; idx <= stackDepth; idx++)
                                        sprintf(printfFormat + strlen(printfFormat), "%s.", nameStack[idx]);
                                    sprintf(printfFormat + strlen(printfFormat), "%s", name);
                                    sprintf(printfFormat + strlen(printfFormat), "[%%.%ds];", maxSize);
                                }
                            }

                            writeCount += PrintToFile(outFile, "%s %s[%d];", TYPE, name, maxSize);
                        }

                        if(type == '9')
                        {
                            if(maxSize > maxNumberSize) maxNumberSize = maxSize;
                        }
                        else
                        {
                            if(maxSize > maxTextSize) maxTextSize = maxSize;
                        }

                        if(writeCount > maxWrite) maxWrite = writeCount;
                        spaceSize = (maxLine - writeCount); if(spaceSize < 0) spaceSize = 0;
                        writeCount += PrintToFile(outFile, "%.*s//%s\n", spaceSize, spaceBuff, desc);
                    }
                    break;
                case 'S':
                    {
                        stackDepth = currentDepth;
                        memcpy(&nameStack[stackDepth][0], name, (int)strlen(name));
                        nameStack[stackDepth][strlen(name)] = 0;
                        minSizeStack[stackDepth] = minSize;
                        maxSizeStack[stackDepth] = maxSize;
                        writeCount += PrintToFile(outFile, "struct tag%s", name);
                        if(writeCount > maxWrite) maxWrite = writeCount;
                        spaceSize = (maxLine - writeCount); if(spaceSize < 0) spaceSize = 0;
                        writeCount += PrintToFile(outFile, "%.*s//%s\n", spaceSize, spaceBuff, desc);
                        for(idx = 0; idx < currentDepth; idx++) PrintToFile(outFile, "    ");
                        PrintToFile(outFile, "{\n");
                    }
                    break;
                }
            }
        }
        else break;
    }

    if(structCount)
    {
        for(idx = stackDepth; idx >= 0; idx--)
        {
            PrintToFile(outFile, "\n");
            for(jdx = 0; jdx < idx; jdx++) PrintToFile(outFile, "    ");
            if(idx)
            {
                if(maxSizeStack[idx] > 1)
                    PrintToFile(outFile, "} %s[%d];\n", nameStack[idx], maxSizeStack[idx]);
                else
                    PrintToFile(outFile, "} %s;\n", nameStack[idx]);
            }
            else
            {
                PrintToFile(outFile, "} %s, *P%s;\n", nameStack[idx], nameStack[idx]);
                PrintToFile(outFile, "\n");
                PrintToFile(outFile, "#define SIZEOF_%s_MIN (%d)\n", nameStack[idx], minMsgSize);
                PrintToFile(outFile, "#define SIZEOF_%s_MAX (%d)\n", nameStack[idx], maxMsgSize);
                PrintToFile(outFile, "#define %s_PRINTF_FORMAT \"%%s\"\"%s\"\"%%s\"\n", nameStack[idx], printfFormat);
                PrintToFile(outFile, "#define %s_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix%s, suffix\n", nameStack[idx], printfArgs);
                if(maxMsgSize > totalMaxMsgSize) totalMaxMsgSize = maxMsgSize;
            }
        }

        PrintToFile(outFile, "\n");
        PrintToFile(outFile, "\n");
        PrintToFile(outFile, "#define SIZEOF_MSG_MAX (%d)\n", totalMaxMsgSize);
        PrintToFile(outFile, "#define SIZEOF_NUMBER_MAX (%d)\n", maxNumberSize);
        PrintToFile(outFile, "#define SIZEOF_TEXT_MAX (%d)\n", maxTextSize);
    }

    PrintToFile(outFile, "\n");
    PrintToFile(outFile, "#pragma pack()\n");
    PrintToFile(outFile, "\n");

    free(spaceBuff);
    free(printfFormat);
    free(printfArgs);

    return maxWrite;
}
