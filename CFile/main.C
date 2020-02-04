#include <ConvertString.H>
#include <FileGlobList.H>
#include <CFile_Cksum_C.h>

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <io.h>
#include <fcntl.h>
#include <fcntl.h>
#include <direct.h>
#else
#include <unistd.h>
#endif

#include <sys/stat.h>

typedef struct _FileList
{
    char packageContents[1024];
    char packageCFiles[1024];
    char packageHFiles[1024];
    char packageOFiles[1024];
    char packageNFiles[1024];
    size_t crc;

} FileList,
*PFileList;

int followLinks = 0;

static int verbose = 0;
static int outputLines = 0;
static int outputBlocks = 0;
static int decode = 0;
static int mainProcedure = 0;
static int autoName = 0;
static int useDefines = 1;
static int useGlobbing = 0;
static char outputDir[1024] = "";
static char currentDir[1024] = "";
static char* packageName = NULL;
static FileList* fileList = NULL;

const static size_t blockSize = 256;

extern "C" int StrCmp(const void* _Item1, const void* _Item2)
{
    PFileList Item1 = (PFileList)_Item1;
    PFileList Item2 = (PFileList)_Item2;

    return strcmp(Item1->packageContents, Item2->packageContents);
}

typedef struct
{
    size_t FileSize;
    size_t LineCount;
    size_t MaxLine;
    size_t Crc;
} CFileInfo, *PCFileInfo;

const char* Header[] =
{
    "#ifndef __%s_%s_H__",
    "#define __%s_%s_H__",
    "",
    "#ifdef  __cplusplus",
    "extern \"C\" {",
    "#endif",
    "",
};
const char* Footer[] =
{
    "",
    "#ifdef  __cplusplus",
    "}",
    "#endif",
    "",
    "#endif /* __%s_%s_H__ */",
};

const char* CFileHeaderBody[] =
{
    "#include <stdio.h>",
    "#include <string.h>",
    "",
    "#ifdef WIN32",
    "#include <direct.h>",
    "#endif",
    "",
    "#include <sys/types.h>",
    "#include <sys/stat.h>",
    "",
    "extern const size_t %s_%s_FileSize;",
    "extern const size_t %s_%s_LineCount;",
    "extern const size_t %s_%s_MaxLine;",
    "extern const size_t %s_%s_Crc;",
    "extern const char %s_%s_Name[];",
    "extern const int %s_%s_Mode;",
};

const char* CMakfileHead[] =
{
    "#",
    "# Inital declarations",
    "#",
    "D_SRC			= .",
    "D_INC			= .",
    "D_OBJ			= obj",
    "D_BIN			= bin",
    "INCLUDE			= -I$(D_INC)",
    "NOM_PROC		= $(D_BIN)/%s",
    "COMPILOC		= gcc 2>er",
    "",
    "#",
    "# Object list",
    "#",
    "LIST_OBJ		= \\",
};

const char* CMakfileFoot[] =
{
    "				  $(D_OBJ)/%s_cksum.o \\",
    "				  $(D_OBJ)/%s_Main.o",
    "#",
    "# For all",
    "#",
    "all: .INIT $(NOM_PROC)",
    "",
    "nocygwin:",
    "	make all \"COMPILOC=gcc -mno-cygwin 2>er\"",
    "",
    "$(NOM_PROC): $(LIST_OBJ)",
    "	$(COMPILOC) $(LIST_OBJ) \\",
    "		$(DOPTLNK) -o $(NOM_PROC)",
    "",
    ".INIT:",
    "	rm -f er",
    "	if [ ! -d $(D_OBJ) ] ; then   \\",
    "		mkdir -p $(D_OBJ) ;  \\",
    "	fi",
    "	if [ ! -d $(D_BIN) ] ; then   \\",
    "		mkdir -p $(D_BIN) ;  \\",
    "	fi",
    "",
    "#",
    "# Compilation rules",
    "#",
    "${D_OBJ}/%%.o : ${D_SRC}/%%.c",
    "	@echo Compiling $(?F)",
    "	@${COMPILOC} ${INCLUDE} -c $? -o $@",
    "",
    "${D_OBJ}/%%.o : ${D_SRC}/%%.cpp",
    "	@echo Compiling $(?F)",
    "	@${COMPILOC} ${INCLUDE} -c $? -o $@",
    "",
    "#",
    "# Cleaning rules",
    "#",
    "vclean:",
    "	rm -f $(D_OBJ)/*.o $(NOM_PROC)",
    "clean:",
    "	rm -f $(D_OBJ)/*.o",
    "",
};

const char* CMainHead[] =
{
    "#include <ctype.h>",
    "#include <stdlib.h>",
    "#include <stdio.h>",
    "#include <string.h>",
    "#include <stdlib.h>",
    "#include <sys/stat.h>",
    "",
    "static int useGlobbing = 0;",
    "static int caseSensitive = 1;",
    "static char outputDir[1024] = \"\";",
    "static char currentDir[1024] = \"\";",
    "",
    "void DisplayDetailedUsage(FILE * file)",
    "{",
    "    if(file)",
    "    {",
    "        fprintf(file, \"\\nUsage: %s {OPTIONS}\\n\");",
    "        fprintf(file, \"\\n\");",
    "        fprintf(file, \"  Options:\\n\");",
    "        fprintf(file, \"           --install\\n\");",
    "        fprintf(file, \"           --uninstall\\n\");",
    "        fprintf(file, \"           --extract\\n\");",
    "        fprintf(file, \"           --delete\\n\");",
    "        fprintf(file, \"           --view\\n\");",
    "        fprintf(file, \"           --list\\n\");",
    "        fprintf(file, \"           --touch\\n\");",
    "        fprintf(file, \"           --verifySelf\\n\");",
    "        fprintf(file, \"           --verifyInstall\\n\");",
    "        fprintf(file, \"           --glob\\n\");",
    "        fprintf(file, \"           --ignoreCase\\n\");",
    "        fprintf(file, \"           --dir [output path]\\n\");",
    "        fprintf(file, \"           --help\\n\");",
    "        fprintf(file, \"\\n\");",
    "        fflush(file);",
    "    }",
    "}",
    "",
    "int WildMatch( const char* pattern, const char *string, int caseSensitive )",
    "{",
    "    const char* mp = NULL;",
    "    const char* cp = NULL;",
    "",
    "    // Handle all the letters of the pattern and the string.",
    "    while ( *string != 0  &&  *pattern != '*' )",
    "    {",
    "        if ( *pattern != '?' )",
    "        {",
    "            if ( caseSensitive )",
    "            {",
    "                if ( *pattern != *string )",
    "                    return 0;",
    "            }",
    "            else",
    "            {",
    "                if ( toupper( *pattern ) != toupper( *string ) )",
    "                    return 0;",
    "            }",
    "        }",
    "",
    "        pattern++;",
    "        string++;",
    "    }",
    "",
    "    while ( *string != 0 )",
    "    {",
    "        if (*pattern == '*')",
    "        {",
    "            // It's a match if the wildcard is at the end.",
    "            if ( *++pattern == 0 )",
    "            {",
    "                return 1;",
    "            }",
    "",
    "            mp = pattern;",
    "            cp = string + 1;",
    "        }",
    "        else",
    "        {",
    "            if ( caseSensitive )",
    "            {",
    "                if ( *pattern == *string  ||  *pattern == '?' )",
    "                {",
    "                    pattern++;",
    "                    string++;",
    "                }",
    "                else",
    "                {",
    "                    pattern = mp;",
    "                    string = cp++;",
    "                }",
    "            }",
    "            else",
    "            {",
    "                if ( toupper( *pattern ) == toupper( *string )  ||  *pattern == '?' )",
    "                {",
    "                    pattern++;",
    "                    string++;",
    "                }",
    "                else",
    "                {",
    "                    pattern = mp;",
    "                    string = cp++;",
    "                }",
    "            }",
    "        }",
    "    }",
    "",
    "    // Collapse remaining wildcards.",
    "    while ( *pattern == '*' )",
    "        pattern++;",
    "",
    "    return (!*pattern ? 1 : 0);",
    "}",
    "",
    "typedef size_t (*DumpContentsToFile)(FILE* file);",
    "typedef size_t (*DumpContentsToNewFile)(const char* fileName);",
    "typedef size_t (*DumpContents)();",
    "typedef size_t (*Touch)(int forceWrite);",
    "typedef size_t (*Unlink)();",
    "typedef size_t (*Verify)();",
    "",
    "typedef struct _%s_List",
    "{",
    "    char*                   name;",
    "    size_t                  crc;",
    "    DumpContentsToFile      dumpContentsToFile;",
    "    DumpContentsToNewFile   dumpContentsToNewFile;",
    "    DumpContents            dumpContents;",
    "    Touch                   touch;",
    "    Unlink                  unlink;",
    "    Unlink                  verify;",
    "",
    "} %s_List,",
    "*P%s_List;",
    "",
    "int __stricmp(const char* s, const char* t)",
    "{",
    "    for(; tolower(*s) == tolower(*t); s++, t++)",
    "        if(*s == '\\0')",
    "            return(0);",
    "    return(tolower(*s) - tolower(*t));",
    "}",
    "",
    "int StrCmp(const void* _Item1,",
    "           const void* _Item2)",
    "{",
    "    P%s_List Item1 = ",
    "        (P%s_List)_Item1;",
    "    P%s_List Item2 = ",
    "        (P%s_List)_Item2;",
    "",
    "    if(caseSensitive)",
    "        return strcmp(Item1->name, Item2->name);",
    "    else",
    "        return __stricmp(Item1->name, Item2->name);",
    "}",
    "",
    "#ifdef  __cplusplus",
    "extern \"C\" {",
    "#endif",
    "    unsigned long sum(unsigned long filecrc, unsigned char *bp, unsigned long n);",
    "#ifdef  __cplusplus",
    "}",
    "#endif",
    "",
    "unsigned long SumFile(char *filename)",
    "{",
    "    FILE *file;",
    "    unsigned long crc;",
    "    unsigned long len;",
    "    unsigned long size;",
    "    unsigned char buffer[1024];",
    "",
    "    if((file = fopen (filename, \"rb\")) == NULL) return 0;",
    "",
    "    crc = size = 0;",
    "",
    "    while(len = (unsigned long)fread(buffer, 1, sizeof(buffer), file))",
    "    {",
    "        size += len;",
    "        crc = sum(crc, buffer, len);",
    "    }",
    "",
    "    crc = sum(crc, 0, size);",
    "    fclose (file);",
    "",
    "    return crc;",
    "}",
    "",
};

const char* CVCProjectHead[] =
{
    "<?xml version=\"1.0\"?>",
    "<VisualStudioProject",
    "	ProjectType=\"Visual C++\"",
    "	Version=\"9.00\"",
    "	Name=\"%s\"",
    "	RootNamespace=\"Test\"",
    "	Keyword=\"Win32Proj\"",
    "	>",
    "	<Platforms>",
    "		<Platform Name=\"Win32\"/>",
    "	</Platforms>",
    "	<ToolFiles>",
    "	</ToolFiles>",
    "	<Configurations>",
    "		<Configuration",
    "			Name=\"Debug|Win32\"",
    "			OutputDirectory=\"$(SolutionDir)$(ConfigurationName)\"",
    "			IntermediateDirectory=\"$(ConfigurationName)\"",
    "			ConfigurationType=\"1\"",
    "			CharacterSet=\"1\"",
    "			>",
    "			<Tool",
    "				Name=\"VCCLCompilerTool\"",
    "				Optimization=\"0\"",
    "				PreprocessorDefinitions=\"WIN32;_DEBUG;_CONSOLE;_CRT_SECURE_NO_WARNINGS\"",
    "				MinimalRebuild=\"true\"",
    "				BasicRuntimeChecks=\"3\"",
    "				RuntimeLibrary=\"1\"",
    "				UsePrecompiledHeader=\"0\"",
    "				WarningLevel=\"3\"",
    "				Detect64BitPortabilityProblems=\"true\"",
    "				DebugInformationFormat=\"4\"",
    "			/>",
    "			<Tool",
    "				Name=\"VCLinkerTool\"",
    "				LinkIncremental=\"2\"",
    "				GenerateDebugInformation=\"true\"",
    "				SubSystem=\"1\"",
    "				TargetMachine=\"1\"",
    "			/>",
    "		</Configuration>",
    "		<Configuration",
    "			Name=\"Release|Win32\"",
    "			OutputDirectory=\"$(SolutionDir)$(ConfigurationName)\"",
    "			IntermediateDirectory=\"$(ConfigurationName)\"",
    "			ConfigurationType=\"1\"",
    "			CharacterSet=\"1\"",
    "			WholeProgramOptimization=\"1\"",
    "			>",
    "			<Tool",
    "				Name=\"VCCLCompilerTool\"",
    "				PreprocessorDefinitions=\"WIN32;NDEBUG;_CONSOLE;_CRT_SECURE_NO_WARNINGS\"",
    "				RuntimeLibrary=\"0\"",
    "				UsePrecompiledHeader=\"0\"",
    "				WarningLevel=\"3\"",
    "				Detect64BitPortabilityProblems=\"true\"",
    "				DebugInformationFormat=\"3\"",
    "			/>",
    "			<Tool",
    "				Name=\"VCLinkerTool\"",
    "				LinkIncremental=\"1\"",
    "				GenerateDebugInformation=\"true\"",
    "				SubSystem=\"1\"",
    "				OptimizeReferences=\"2\"",
    "				EnableCOMDATFolding=\"2\"",
    "				TargetMachine=\"1\"",
    "			/>",
    "		</Configuration>",
    "	</Configurations>",
    "	<Files>",
};

size_t ToSentenceCase(char* in, char* out)
{
    size_t idx = 0, jdx = 0;
    if(strchr("\\/", in[idx]))
    {
        out[jdx++] = '_';
        out[jdx++] = '_';
        idx++;
    }
    else if(strchr("~@#%^&*()+-={}|[]:;<>\?.`!$.,'\" ", in[idx]))
    {
        out[jdx++] = '_';
        idx++;
    }
    else
    {
        out[jdx++] = toupper(in[idx++]);
    }

    while(in[idx])
    {
        if(strchr("\\/", in[idx]))
        {
            out[jdx++] = '_';
            out[jdx++] = '_';
            idx++;
        }
        else if(strchr("~@#%^&*()+-={}|[]:;<>\?.`!$.,'\" ", in[idx]))
        {
            out[jdx++] = '_';
            idx++;
        }
        else if(strchr("_1234567890", out[jdx-1]))
            out[jdx++] = toupper(in[idx++]);
        else
            out[jdx++] = tolower(in[idx++]);
    }

    out[jdx] = 0;

    return jdx;
}

size_t FileGetLine(char* s, int n, FILE *fp)
{
    int         ch;
    char*       cs = s;
    int         count = 0;

    while((--n > 0) && (ch = fgetc(fp)) != EOF)
    {
        count++;
        if((*cs++ = ch) == '\n') break;
    }

    if(ch == EOF && (cs == s || ferror( fp )))
        count = 0;
    else
        *cs = '\0';

    return count;
}

extern "C" unsigned long sum(unsigned long filecrc, unsigned char *bp, unsigned long n);

CFileInfo CountLinesInFile(const char* fileName)
{
    CFileInfo info;
    FILE* file = fopen(fileName, "rb");
    size_t len, lineLen;
    char buffer[1024+1];
    memset(&info, 0, sizeof(info));
    if(file)
    {
        lineLen = 0;
        while(feof(file) != EOF)
        {
            if(len = FileGetLine(buffer, sizeof(buffer), file))
            {
                lineLen += len;
                info.FileSize += len;
                info.Crc = (size_t)sum((unsigned long)info.Crc, (unsigned char*)buffer, (unsigned long)len);
                if(buffer[len-1] == '\r' || buffer[len-1] == '\n')
                {
                    if(lineLen > info.MaxLine) info.MaxLine = lineLen;
                    lineLen = 0;
                    info.LineCount++;
                }
            }
            else break;
        }
        info.Crc = (size_t)sum((unsigned long)info.Crc, NULL, (unsigned long)info.FileSize);

        if(lineLen) info.LineCount++;
        fclose(file);
    }
    return info;
}

typedef int (*GeneratorChainRoutine)(char* projectName, char* moduleName, FILE* file, void* userData1, void* userData2);

int GenerateMakefile(char* packageName, FILE* file, int argc, FileList* fileList)
{
    int idx, last, count = 0;

    for(idx = 0; idx < sizeof(CMakfileHead) / sizeof(CMakfileHead[0]); idx++)
    {
        count += last = fprintf(file, CMakfileHead[idx], packageName);
        fputs("\n", file); count++;
    }

    for(idx = 0; idx < argc; idx++)
    {
        count += last = fprintf(file, "				  $(D_OBJ)/%s \\", fileList[idx].packageOFiles, packageName);
        fputs("\n", file); count++;
    }

    for(idx = 0; idx < sizeof(CMakfileFoot) / sizeof(CMakfileFoot[0]); idx++)
    {
        count += last = fprintf(file, CMakfileFoot[idx], packageName);
        fputs("\n", file); count++;
    }

    return count;
}

int GenerateCMain(char* packageName, FILE* file, int argc, FileList* fileList)
{
    int idx, last, count = 0;

    for(idx = 0; idx < sizeof(CMainHead) / sizeof(CMainHead[0]); idx++)
    {
        count += last = fprintf(file, CMainHead[idx], packageName);
        fputs("\n", file); count++;
    }

    for(idx = 0; idx < argc; idx++)
    {
        count += last = fprintf(file, "#include \"%s\"", fileList[idx].packageHFiles, packageName);
        fputs("\n", file); count++;
    }
    count += last = fprintf(file, "\n");

    count += last = fprintf(file, "static const %s_List %s_Files[] = \n", packageName, packageName);
    count += last = fprintf(file, "{\n");
    for(idx = 0; idx < argc; idx++)
    {
        count += last = fprintf(file, "    \"%s\", %uUL, %s_DumpContentsToFile, %s_DumpContentsToNewFile, %s_DumpContents, %s_Touch, %s_Unlink, %s_Verify, \n",
            fileList[idx].packageContents, fileList[idx].crc, fileList[idx].packageNFiles, fileList[idx].packageNFiles, fileList[idx].packageNFiles, fileList[idx].packageNFiles, fileList[idx].packageNFiles, fileList[idx].packageNFiles, fileList[idx].packageNFiles, fileList[idx].packageNFiles);
    }
    count += last = fprintf(file, "};\n");
    count += last = fprintf(file, "static const size_t %s_FileCount = sizeof(%s_Files) / sizeof(%s_Files[0]);\n", packageName, packageName, packageName);

    count += last = fprintf(file, "\n");
    count += last = fprintf(file, "int main(int argc, char** argv)\n");
    count += last = fprintf(file, "{\n");
    count += last = fprintf(file, "    int flag = 0;\n");

    count += last = fprintf(file, "    while(argc > 1 && argv[1][0] == '-' && argv[1][1] == '-')\n");
    count += last = fprintf(file, "    {\n");
    count += last = fprintf(file, "\n");
    //glob
    count += last = fprintf(file, "        if(!strcmp(\"glob\", &argv[1][2]))\n");
    count += last = fprintf(file, "        {\n");
    count += last = fprintf(file, "            useGlobbing = 1;\n");
    count += last = fprintf(file, "        }\n");
    //ignoreCase
    count += last = fprintf(file, "        else if(!strcmp(\"ignoreCase\", &argv[1][2]))\n");
    count += last = fprintf(file, "        {\n");
    count += last = fprintf(file, "            caseSensitive = 0;\n");
    count += last = fprintf(file, "        }\n");
    //dir
    count += last = fprintf(file, "        else if(!strcmp(\"dir\", &argv[1][2]))\n");
    count += last = fprintf(file, "        {\n");
    count += last = fprintf(file, "            if(argc > 2 && argv[2][0] != '-')\n");
    count += last = fprintf(file, "            {\n");
    count += last = fprintf(file, "                strcpy(outputDir, argv[2]);\n");
    count += last = fprintf(file, "#ifdef WIN32\n");
    count += last = fprintf(file, "                if(outputDir[0]) _chdir(outputDir);\n");
    count += last = fprintf(file, "#else\n");
    count += last = fprintf(file, "                if(outputDir[0]) chdir(outputDir);\n");
    count += last = fprintf(file, "#endif\n");
    count += last = fprintf(file, "                argc--;\n");
    count += last = fprintf(file, "                argv++;\n");
    count += last = fprintf(file, "            }\n");
    count += last = fprintf(file, "        }\n");
    //dir
    count += last = fprintf(file, "        else if(argv[1][2] == 'd' &&\n");
    count += last = fprintf(file, "                argv[1][3] == 'i' &&\n");
    count += last = fprintf(file, "                argv[1][4] == 'r' &&\n");
    count += last = fprintf(file, "                argv[1][5] == '=' &&\n");
    count += last = fprintf(file, "                argv[1][6] != '\\0')\n");
    count += last = fprintf(file, "        {\n");
    count += last = fprintf(file, "            strcpy(outputDir, &argv[1][6]);\n");
    count += last = fprintf(file, "#ifdef WIN32\n");
    count += last = fprintf(file, "            if(outputDir[0]) _chdir(outputDir);\n");
    count += last = fprintf(file, "#else\n");
    count += last = fprintf(file, "            if(outputDir[0]) chdir(outputDir);\n");
    count += last = fprintf(file, "#endif\n");
    count += last = fprintf(file, "        }\n");
    //install
    count += last = fprintf(file, "        else if(!strcmp(\"install\", &argv[1][2]))\n");
    count += last = fprintf(file, "        {\n");
    count += last = fprintf(file, "            size_t idx;\n");
    count += last = fprintf(file, "            flag = 1;\n");
    count += last = fprintf(file, "            for(idx = 0; idx < %s_FileCount; idx++)\n", packageName);
    count += last = fprintf(file, "            {\n");
    count += last = fprintf(file, "                %s_Files[idx].dumpContents();\n", packageName);
    count += last = fprintf(file, "            }\n");
    count += last = fprintf(file, "#ifdef WIN32\n");
    count += last = fprintf(file, "            {\n");
    count += last = fprintf(file, "                struct stat st;\n");
    count += last = fprintf(file, "                if(!stat(\"autorun.cmd\", &st) && \n");
    count += last = fprintf(file, "                   (st.st_mode & _S_IREAD) && \n");
    count += last = fprintf(file, "                   (st.st_mode & _S_IEXEC))\n");
    count += last = fprintf(file, "                {\n");
    count += last = fprintf(file, "                    system(\"autorun.cmd install\");\n");
    count += last = fprintf(file, "                }\n");
    count += last = fprintf(file, "            }\n");
    count += last = fprintf(file, "#else\n");
    count += last = fprintf(file, "            {\n");
    count += last = fprintf(file, "                struct stat st;\n");
    count += last = fprintf(file, "                if(!stat(\"autorun.sh\", &st) && \n");
    count += last = fprintf(file, "                   (st.st_mode & S_IRUSR) && \n");
    count += last = fprintf(file, "                   (st.st_mode & S_IXUSR))\n");
    count += last = fprintf(file, "                {\n");
    count += last = fprintf(file, "                    system(\"./autorun.sh install\");\n");
    count += last = fprintf(file, "                }\n");
    count += last = fprintf(file, "            }\n");
    count += last = fprintf(file, "#endif\n");
    count += last = fprintf(file, "        }\n");
    //uninstall
    count += last = fprintf(file, "        else if(!strcmp(\"uninstall\", &argv[1][2]))\n");
    count += last = fprintf(file, "        {\n");
    count += last = fprintf(file, "            size_t idx;\n");
    count += last = fprintf(file, "            flag = 1;\n");
    count += last = fprintf(file, "#ifdef WIN32\n");
    count += last = fprintf(file, "            {\n");
    count += last = fprintf(file, "                struct stat st;\n");
    count += last = fprintf(file, "                if(!stat(\"autorun.cmd\", &st) && \n");
    count += last = fprintf(file, "                   (st.st_mode & _S_IREAD) && \n");
    count += last = fprintf(file, "                   (st.st_mode & _S_IEXEC))\n");
    count += last = fprintf(file, "                {\n");
    count += last = fprintf(file, "                    system(\"autorun.cmd uninstall\");\n");
    count += last = fprintf(file, "                }\n");
    count += last = fprintf(file, "            }\n");
    count += last = fprintf(file, "#else\n");
    count += last = fprintf(file, "            {\n");
    count += last = fprintf(file, "                struct stat st;\n");
    count += last = fprintf(file, "                if(!stat(\"autorun.sh\", &st) && \n");
    count += last = fprintf(file, "                   (st.st_mode & S_IRUSR) && \n");
    count += last = fprintf(file, "                   (st.st_mode & S_IXUSR))\n");
    count += last = fprintf(file, "                {\n");
    count += last = fprintf(file, "                    system(\"./autorun.sh uninstall\");\n");
    count += last = fprintf(file, "                }\n");
    count += last = fprintf(file, "            }\n");
    count += last = fprintf(file, "#endif\n");
    count += last = fprintf(file, "            for(idx = 0; idx < %s_FileCount; idx++)\n", packageName);
    count += last = fprintf(file, "            {\n");
    count += last = fprintf(file, "                %s_Files[idx].touch(1);\n", packageName);
    count += last = fprintf(file, "                %s_Files[idx].unlink();\n", packageName);
    count += last = fprintf(file, "            }\n");
    count += last = fprintf(file, "        }\n");
    //extract
    count += last = fprintf(file, "        else if(!strcmp(\"extract\", &argv[1][2]))\n");
    count += last = fprintf(file, "        {\n");
    count += last = fprintf(file, "            flag = 1;\n");
    count += last = fprintf(file, "            if(argc > 2 && argv[2][0] != '-')\n");
    count += last = fprintf(file, "            {\n");
    count += last = fprintf(file, "                while(argc > 2 && argv[2][0] != '-')\n");
    count += last = fprintf(file, "                {\n");
    count += last = fprintf(file, "                    if(useGlobbing)\n");
    count += last = fprintf(file, "                    {\n");
    count += last = fprintf(file, "                        size_t idx;\n");
    count += last = fprintf(file, "                        for(idx = 0; idx < %s_FileCount; idx++)\n", packageName);
    count += last = fprintf(file, "                        {\n");
    count += last = fprintf(file, "                            if(WildMatch(argv[2], %s_Files[idx].name, caseSensitive))\n", packageName);
    count += last = fprintf(file, "                                %s_Files[idx].dumpContents();\n", packageName);
    count += last = fprintf(file, "                        }\n");
    count += last = fprintf(file, "                    }\n");
    count += last = fprintf(file, "                    else\n");
    count += last = fprintf(file, "                    {\n");
    count += last = fprintf(file, "                        %s_List key;\n", packageName);
    count += last = fprintf(file, "                        P%s_List item;\n", packageName);
    count += last = fprintf(file, "                        key.name = argv[2];\n");
    count += last = fprintf(file, "                        if(item = (P%s_List)bsearch(&key, %s_Files, %s_FileCount, sizeof(%s_List), StrCmp))\n", packageName, packageName, packageName, packageName);
    count += last = fprintf(file, "                            item->dumpContents();\n");
    count += last = fprintf(file, "                    }\n");
    count += last = fprintf(file, "                    argc--;\n");
    count += last = fprintf(file, "                    argv++;\n");
    count += last = fprintf(file, "                }\n");
    count += last = fprintf(file, "            }\n");
    count += last = fprintf(file, "            else\n");
    count += last = fprintf(file, "            {\n");
    count += last = fprintf(file, "                size_t idx;\n");
    count += last = fprintf(file, "                for(idx = 0; idx < %s_FileCount; idx++)\n", packageName);
    count += last = fprintf(file, "                {\n");
    count += last = fprintf(file, "                    %s_Files[idx].dumpContents();\n", packageName);
    count += last = fprintf(file, "                }\n");
    count += last = fprintf(file, "            }\n");
    count += last = fprintf(file, "        }\n");
    //delete
    count += last = fprintf(file, "        else if(!strcmp(\"delete\", &argv[1][2]))\n");
    count += last = fprintf(file, "        {\n");
    count += last = fprintf(file, "            flag = 1;\n");
    count += last = fprintf(file, "            if(argc > 2 && argv[2][0] != '-')\n");
    count += last = fprintf(file, "            {\n");
    count += last = fprintf(file, "                while(argc > 2 && argv[2][0] != '-')\n");
    count += last = fprintf(file, "                {\n");
    count += last = fprintf(file, "                    if(useGlobbing)\n");
    count += last = fprintf(file, "                    {\n");
    count += last = fprintf(file, "                        size_t idx;\n");
    count += last = fprintf(file, "                        for(idx = 0; idx < %s_FileCount; idx++)\n", packageName);
    count += last = fprintf(file, "                        {\n");
    count += last = fprintf(file, "                            if(WildMatch(argv[2], %s_Files[idx].name, caseSensitive))\n", packageName);
    count += last = fprintf(file, "                            {\n");
    count += last = fprintf(file, "                                %s_Files[idx].touch(1);\n", packageName);
    count += last = fprintf(file, "                                %s_Files[idx].unlink();\n", packageName);
    count += last = fprintf(file, "                            }\n");
    count += last = fprintf(file, "                        }\n");
    count += last = fprintf(file, "                    }\n");
    count += last = fprintf(file, "                    else\n");
    count += last = fprintf(file, "                    {\n");
    count += last = fprintf(file, "                        %s_List key;\n", packageName);
    count += last = fprintf(file, "                        P%s_List item;\n", packageName);
    count += last = fprintf(file, "                        key.name = argv[2];\n");
    count += last = fprintf(file, "                        if(item = (P%s_List)bsearch(&key, %s_Files, %s_FileCount, sizeof(%s_List), StrCmp))\n", packageName, packageName, packageName, packageName);
    count += last = fprintf(file, "                        {\n");
    count += last = fprintf(file, "                            item->touch(1);\n");
    count += last = fprintf(file, "                            item->unlink();\n");
    count += last = fprintf(file, "                        }\n");
    count += last = fprintf(file, "                    }\n");
    count += last = fprintf(file, "                    argc--;\n");
    count += last = fprintf(file, "                    argv++;\n");
    count += last = fprintf(file, "                }\n");
    count += last = fprintf(file, "            }\n");
    count += last = fprintf(file, "            else\n");
    count += last = fprintf(file, "            {\n");
    count += last = fprintf(file, "                size_t idx;\n");
    count += last = fprintf(file, "                for(idx = 0; idx < %s_FileCount; idx++)\n", packageName);
    count += last = fprintf(file, "                {\n");
    count += last = fprintf(file, "                    %s_Files[idx].touch(1);\n", packageName);
    count += last = fprintf(file, "                    %s_Files[idx].unlink();\n", packageName);
    count += last = fprintf(file, "                }\n");
    count += last = fprintf(file, "            }\n");
    count += last = fprintf(file, "        }\n");
    //view
    count += last = fprintf(file, "        else if(!strcmp(\"view\", &argv[1][2]))\n");
    count += last = fprintf(file, "        {\n");
    count += last = fprintf(file, "            flag = 1;\n");
    count += last = fprintf(file, "            if(argc > 2 && argv[2][0] != '-')\n");
    count += last = fprintf(file, "            {\n");
    count += last = fprintf(file, "                while(argc > 2 && argv[2][0] != '-')\n");
    count += last = fprintf(file, "                {\n");
    count += last = fprintf(file, "                    if(useGlobbing)\n");
    count += last = fprintf(file, "                    {\n");
    count += last = fprintf(file, "                        size_t idx;\n");
    count += last = fprintf(file, "                        for(idx = 0; idx < %s_FileCount; idx++)\n", packageName);
    count += last = fprintf(file, "                        {\n");
    count += last = fprintf(file, "                            if(WildMatch(argv[2], %s_Files[idx].name, caseSensitive))\n", packageName);
    count += last = fprintf(file, "                                %s_Files[idx].dumpContentsToFile(stdout);\n", packageName);
    count += last = fprintf(file, "                        }\n");
    count += last = fprintf(file, "                    }\n");
    count += last = fprintf(file, "                    else\n");
    count += last = fprintf(file, "                    {\n");
    count += last = fprintf(file, "                        %s_List key;\n", packageName);
    count += last = fprintf(file, "                        P%s_List item;\n", packageName);
    count += last = fprintf(file, "                        key.name = argv[2];\n");
    count += last = fprintf(file, "                        if(item = (P%s_List)bsearch(&key, %s_Files, %s_FileCount, sizeof(%s_List), StrCmp))\n", packageName, packageName, packageName, packageName);
    count += last = fprintf(file, "                            item->dumpContentsToFile(stdout);\n", packageName);
    count += last = fprintf(file, "                    }\n");
    count += last = fprintf(file, "                    argc--;\n");
    count += last = fprintf(file, "                    argv++;\n");
    count += last = fprintf(file, "                }\n");
    count += last = fprintf(file, "            }\n");
    count += last = fprintf(file, "        }\n");
    //list
    count += last = fprintf(file, "        else if(!strcmp(\"list\", &argv[1][2]))\n");
    count += last = fprintf(file, "        {\n");
    count += last = fprintf(file, "            size_t count = 0;\n");
    count += last = fprintf(file, "            flag = 1;\n");
    count += last = fprintf(file, "            fprintf(stderr, \"\\n\");\n");
    count += last = fprintf(file, "            if(argc > 2 && argv[2][0] != '-')\n");
    count += last = fprintf(file, "            {\n");
    count += last = fprintf(file, "                while(argc > 2 && argv[2][0] != '-')\n");
    count += last = fprintf(file, "                {\n");
    count += last = fprintf(file, "                    if(useGlobbing)\n");
    count += last = fprintf(file, "                    {\n");
    count += last = fprintf(file, "                        size_t idx;\n");
    count += last = fprintf(file, "                        for(idx = 0; idx < %s_FileCount; idx++)\n", packageName);
    count += last = fprintf(file, "                        {\n");
    count += last = fprintf(file, "                            if(WildMatch(argv[2], %s_Files[idx].name, caseSensitive))\n", packageName);
    count += last = fprintf(file, "                            {\n");
    count += last = fprintf(file, "                                count++;\n");
    count += last = fprintf(file, "                                fprintf(stdout, \"%%s\\n\", %s_Files[idx].name);\n", packageName);
    count += last = fprintf(file, "                            }\n");
    count += last = fprintf(file, "                        }\n");
    count += last = fprintf(file, "                    }\n");
    count += last = fprintf(file, "                    else\n");
    count += last = fprintf(file, "                    {\n");
    count += last = fprintf(file, "                        %s_List key;\n", packageName);
    count += last = fprintf(file, "                        P%s_List item;\n", packageName);
    count += last = fprintf(file, "                        key.name = argv[2];\n");
    count += last = fprintf(file, "                        if(item = (P%s_List)bsearch(&key, %s_Files, %s_FileCount, sizeof(%s_List), StrCmp))\n", packageName, packageName, packageName, packageName);
    count += last = fprintf(file, "                        {\n");
    count += last = fprintf(file, "                            count++;\n");
    count += last = fprintf(file, "                            fprintf(stdout, \"%%s\\n\", item->name);\n", packageName);
    count += last = fprintf(file, "                        }\n");
    count += last = fprintf(file, "                    }\n");
    count += last = fprintf(file, "                    argc--;\n");
    count += last = fprintf(file, "                    argv++;\n");
    count += last = fprintf(file, "                }\n");
    count += last = fprintf(file, "            }\n");
    count += last = fprintf(file, "            else\n");
    count += last = fprintf(file, "            {\n");
    count += last = fprintf(file, "                size_t idx;\n");
    count += last = fprintf(file, "                for(idx = 0; idx < %s_FileCount; idx++)\n", packageName);
    count += last = fprintf(file, "                {\n");
    count += last = fprintf(file, "                    count++;\n");
    count += last = fprintf(file, "                    fprintf(stdout, \"%%s\\n\", %s_Files[idx].name);\n", packageName);
    count += last = fprintf(file, "                }\n");
    count += last = fprintf(file, "            }\n");
    count += last = fprintf(file, "            if(count) fprintf(stderr, \"\\nCount[%%d]\\n\\n\", count);\n");
    count += last = fprintf(file, "        }\n");
    //touch
    count += last = fprintf(file, "        else if(!strcmp(\"touch\", &argv[1][2]))\n");
    count += last = fprintf(file, "        {\n");
    count += last = fprintf(file, "            flag = 1;\n");
    count += last = fprintf(file, "            if(argc > 2 && argv[2][0] != '-')\n");
    count += last = fprintf(file, "            {\n");
    count += last = fprintf(file, "                while(argc > 2 && argv[2][0] != '-')\n");
    count += last = fprintf(file, "                {\n");
    count += last = fprintf(file, "                    if(useGlobbing)\n");
    count += last = fprintf(file, "                    {\n");
    count += last = fprintf(file, "                        size_t idx;\n");
    count += last = fprintf(file, "                        for(idx = 0; idx < %s_FileCount; idx++)\n", packageName);
    count += last = fprintf(file, "                        {\n");
    count += last = fprintf(file, "                            if(WildMatch(argv[2], %s_Files[idx].name, caseSensitive))\n", packageName);
    count += last = fprintf(file, "                                %s_Files[idx].touch(0);\n", packageName);
    count += last = fprintf(file, "                        }\n");
    count += last = fprintf(file, "                    }\n");
    count += last = fprintf(file, "                    else\n");
    count += last = fprintf(file, "                    {\n");
    count += last = fprintf(file, "                        %s_List key;\n", packageName);
    count += last = fprintf(file, "                        P%s_List item;\n", packageName);
    count += last = fprintf(file, "                        key.name = argv[2];\n");
    count += last = fprintf(file, "                        if(item = (P%s_List)bsearch(&key, %s_Files, %s_FileCount, sizeof(%s_List), StrCmp))\n", packageName, packageName, packageName, packageName);
    count += last = fprintf(file, "                            item->touch(0);\n");
    count += last = fprintf(file, "                    }\n");
    count += last = fprintf(file, "                    argc--;\n");
    count += last = fprintf(file, "                    argv++;\n");
    count += last = fprintf(file, "                }\n");
    count += last = fprintf(file, "            }\n");
    count += last = fprintf(file, "            else\n");
    count += last = fprintf(file, "            {\n");
    count += last = fprintf(file, "                size_t idx;\n");
    count += last = fprintf(file, "                for(idx = 0; idx < %s_FileCount; idx++)\n", packageName);
    count += last = fprintf(file, "                {\n");
    count += last = fprintf(file, "                    %s_Files[idx].touch(0);\n", packageName);
    count += last = fprintf(file, "                }\n");
    count += last = fprintf(file, "            }\n");
    count += last = fprintf(file, "        }\n");
    //verifySelf
    count += last = fprintf(file, "        else if(!strcmp(\"verifySelf\", &argv[1][2]))\n");
    count += last = fprintf(file, "        {\n");
    count += last = fprintf(file, "            size_t pass = 0;\n");
    count += last = fprintf(file, "            size_t fail = 0;\n");
    count += last = fprintf(file, "            size_t crc;\n");
    count += last = fprintf(file, "            flag = 1;\n");
    count += last = fprintf(file, "            fprintf(stderr, \"\\n\");\n");
    count += last = fprintf(file, "            if(argc > 2 && argv[2][0] != '-')\n");
    count += last = fprintf(file, "            {\n");
    count += last = fprintf(file, "                while(argc > 2 && argv[2][0] != '-')\n");
    count += last = fprintf(file, "                {\n");
    count += last = fprintf(file, "                    if(useGlobbing)\n");
    count += last = fprintf(file, "                    {\n");
    count += last = fprintf(file, "                        size_t idx;\n");
    count += last = fprintf(file, "                        for(idx = 0; idx < %s_FileCount; idx++)\n", packageName);
    count += last = fprintf(file, "                        {\n");
    count += last = fprintf(file, "                            if(WildMatch(argv[2], %s_Files[idx].name, caseSensitive))\n", packageName);
    count += last = fprintf(file, "                            {\n");
    count += last = fprintf(file, "                                crc = %s_Files[idx].verify();\n", packageName);
    count += last = fprintf(file, "                                if(crc == %s_Files[idx].crc)\n", packageName);
    count += last = fprintf(file, "                                {\n");
    count += last = fprintf(file, "                                    pass++;\n");
    count += last = fprintf(file, "                                    fprintf(stdout, \"[PASS] %%s\\n\", %s_Files[idx].name);\n", packageName);
    count += last = fprintf(file, "                                }\n");
    count += last = fprintf(file, "                                else\n");
    count += last = fprintf(file, "                                {\n");
    count += last = fprintf(file, "                                    fail++;\n");
    count += last = fprintf(file, "                                    fprintf(stdout, \"[FAIL] %%s\\n\", %s_Files[idx].name);\n", packageName);
    count += last = fprintf(file, "                                }\n");
    count += last = fprintf(file, "                            }\n");
    count += last = fprintf(file, "                        }\n");
    count += last = fprintf(file, "                    }\n");
    count += last = fprintf(file, "                    else\n");
    count += last = fprintf(file, "                    {\n");
    count += last = fprintf(file, "                        %s_List key;\n", packageName);
    count += last = fprintf(file, "                        P%s_List item;\n", packageName);
    count += last = fprintf(file, "                        key.name = argv[2];\n");
    count += last = fprintf(file, "                        if(item = (P%s_List)bsearch(&key, %s_Files, %s_FileCount, sizeof(%s_List), StrCmp))\n", packageName, packageName, packageName, packageName);
    count += last = fprintf(file, "                        {\n");
    count += last = fprintf(file, "                            crc = item->verify();\n");
    count += last = fprintf(file, "                            if(crc == item->crc)\n");
    count += last = fprintf(file, "                            {\n");
    count += last = fprintf(file, "                                pass++;\n");
    count += last = fprintf(file, "                                fprintf(stdout, \"[PASS] %%s\\n\", item->name);\n", packageName);
    count += last = fprintf(file, "                            }\n");
    count += last = fprintf(file, "                            else\n");
    count += last = fprintf(file, "                            {\n");
    count += last = fprintf(file, "                                fail++;\n");
    count += last = fprintf(file, "                                fprintf(stdout, \"[FAIL] %%s\\n\", item->name);\n", packageName);
    count += last = fprintf(file, "                            }\n");
    count += last = fprintf(file, "                        }\n");
    count += last = fprintf(file, "                    }\n");
    count += last = fprintf(file, "                    argc--;\n");
    count += last = fprintf(file, "                    argv++;\n");
    count += last = fprintf(file, "                }\n");
    count += last = fprintf(file, "            }\n");
    count += last = fprintf(file, "            else\n");
    count += last = fprintf(file, "            {\n");
    count += last = fprintf(file, "                size_t idx;\n");
    count += last = fprintf(file, "                for(idx = 0; idx < %s_FileCount; idx++)\n", packageName);
    count += last = fprintf(file, "                {\n");
    count += last = fprintf(file, "                    crc = %s_Files[idx].verify();\n", packageName);
    count += last = fprintf(file, "                    if(crc == %s_Files[idx].crc)\n", packageName);
    count += last = fprintf(file, "                    {\n");
    count += last = fprintf(file, "                        pass++;\n");
    count += last = fprintf(file, "                        fprintf(stdout, \"[PASS] %%s\\n\", %s_Files[idx].name);\n", packageName);
    count += last = fprintf(file, "                    }\n");
    count += last = fprintf(file, "                    else\n");
    count += last = fprintf(file, "                    {\n");
    count += last = fprintf(file, "                        fail++;\n");
    count += last = fprintf(file, "                        fprintf(stdout, \"[FAIL] %%s\\n\", %s_Files[idx].name);\n", packageName);
    count += last = fprintf(file, "                    }\n");
    count += last = fprintf(file, "                }\n");
    count += last = fprintf(file, "            }\n");
    count += last = fprintf(file, "            if(pass + fail) fprintf(stderr, \"\\nPass[%%d], Fail[%%d]\\n\\n\", pass, fail);\n");
    count += last = fprintf(file, "        }\n");
    //verifyInstall
    count += last = fprintf(file, "        else if(!strcmp(\"verifyInstall\", &argv[1][2]))\n");
    count += last = fprintf(file, "        {\n");
    count += last = fprintf(file, "            size_t pass = 0;\n");
    count += last = fprintf(file, "            size_t fail = 0;\n");
    count += last = fprintf(file, "            size_t crc;\n");
    count += last = fprintf(file, "            flag = 1;\n");
    count += last = fprintf(file, "            fprintf(stderr, \"\\n\");\n");
    count += last = fprintf(file, "            if(argc > 2 && argv[2][0] != '-')\n");
    count += last = fprintf(file, "            {\n");
    count += last = fprintf(file, "                while(argc > 2 && argv[2][0] != '-')\n");
    count += last = fprintf(file, "                {\n");
    count += last = fprintf(file, "                    if(useGlobbing)\n");
    count += last = fprintf(file, "                    {\n");
    count += last = fprintf(file, "                        size_t idx;\n");
    count += last = fprintf(file, "                        for(idx = 0; idx < %s_FileCount; idx++)\n", packageName);
    count += last = fprintf(file, "                        {\n");
    count += last = fprintf(file, "                            if(WildMatch(argv[2], %s_Files[idx].name, caseSensitive))\n", packageName);
    count += last = fprintf(file, "                            {\n");
    count += last = fprintf(file, "                                crc = SumFile(%s_Files[idx].name);\n", packageName);
    count += last = fprintf(file, "                                if(crc == %s_Files[idx].crc)\n", packageName);
    count += last = fprintf(file, "                                {\n");
    count += last = fprintf(file, "                                    pass++;\n");
    count += last = fprintf(file, "                                    fprintf(stdout, \"[PASS] %%s\\n\", %s_Files[idx].name);\n", packageName);
    count += last = fprintf(file, "                                }\n");
    count += last = fprintf(file, "                                else\n");
    count += last = fprintf(file, "                                {\n");
    count += last = fprintf(file, "                                    fail++;\n");
    count += last = fprintf(file, "                                    fprintf(stdout, \"[FAIL] %%s\\n\", %s_Files[idx].name);\n", packageName);
    count += last = fprintf(file, "                                }\n");
    count += last = fprintf(file, "                            }\n");
    count += last = fprintf(file, "                        }\n");
    count += last = fprintf(file, "                    }\n");
    count += last = fprintf(file, "                    else\n");
    count += last = fprintf(file, "                    {\n");
    count += last = fprintf(file, "                        %s_List key;\n", packageName);
    count += last = fprintf(file, "                        P%s_List item;\n", packageName);
    count += last = fprintf(file, "                        key.name = argv[2];\n");
    count += last = fprintf(file, "                        if(item = (P%s_List)bsearch(&key, %s_Files, %s_FileCount, sizeof(%s_List), StrCmp))\n", packageName, packageName, packageName, packageName);
    count += last = fprintf(file, "                        {\n");
    count += last = fprintf(file, "                            crc = SumFile(item->name);\n");
    count += last = fprintf(file, "                            if(crc == item->crc)\n");
    count += last = fprintf(file, "                            {\n");
    count += last = fprintf(file, "                                pass++;\n");
    count += last = fprintf(file, "                                fprintf(stdout, \"[PASS] %%s\\n\", item->name);\n", packageName);
    count += last = fprintf(file, "                            }\n");
    count += last = fprintf(file, "                            else\n");
    count += last = fprintf(file, "                            {\n");
    count += last = fprintf(file, "                                fail++;\n");
    count += last = fprintf(file, "                                fprintf(stdout, \"[FAIL] %%s\\n\", item->name);\n", packageName);
    count += last = fprintf(file, "                            }\n");
    count += last = fprintf(file, "                        }\n");
    count += last = fprintf(file, "                    }\n");
    count += last = fprintf(file, "                    argc--;\n");
    count += last = fprintf(file, "                    argv++;\n");
    count += last = fprintf(file, "                }\n");
    count += last = fprintf(file, "            }\n");
    count += last = fprintf(file, "            else\n");
    count += last = fprintf(file, "            {\n");
    count += last = fprintf(file, "                size_t idx;\n");
    count += last = fprintf(file, "                for(idx = 0; idx < %s_FileCount; idx++)\n", packageName);
    count += last = fprintf(file, "                {\n");
    count += last = fprintf(file, "                    crc = SumFile(%s_Files[idx].name);\n", packageName);
    count += last = fprintf(file, "                    if(crc == %s_Files[idx].crc)\n", packageName);
    count += last = fprintf(file, "                    {\n");
    count += last = fprintf(file, "                        pass++;\n");
    count += last = fprintf(file, "                        fprintf(stdout, \"[PASS] %%s\\n\", %s_Files[idx].name);\n", packageName);
    count += last = fprintf(file, "                    }\n");
    count += last = fprintf(file, "                    else\n");
    count += last = fprintf(file, "                    {\n");
    count += last = fprintf(file, "                        fail++;\n");
    count += last = fprintf(file, "                        fprintf(stdout, \"[FAIL] %%s\\n\", %s_Files[idx].name);\n", packageName);
    count += last = fprintf(file, "                    }\n");
    count += last = fprintf(file, "                }\n");
    count += last = fprintf(file, "            }\n");
    count += last = fprintf(file, "            if(pass + fail) fprintf(stderr, \"\\nPass[%%d], Fail[%%d]\\n\\n\", pass, fail);\n");
    count += last = fprintf(file, "        }\n");
    //help
    count += last = fprintf(file, "        else if(!strcmp(\"help\", &argv[1][2]))\n");
    count += last = fprintf(file, "        {\n");
    count += last = fprintf(file, "            flag = 1;\n");
    count += last = fprintf(file, "            DisplayDetailedUsage(stderr);\n");
    count += last = fprintf(file, "        }\n");
    count += last = fprintf(file, "\n");
    count += last = fprintf(file, "        argc--;\n");
    count += last = fprintf(file, "        argv++;\n");
    count += last = fprintf(file, "    }\n");

    count += last = fprintf(file, "\n");
    count += last = fprintf(file, "    if(!flag) fprintf(stderr, \"\\nUsage: %s {OPTIONS}\\n\\n\");\n", packageName);
    count += last = fprintf(file, "\n");
    count += last = fprintf(file, "    return 0;\n");
    count += last = fprintf(file, "}\n");
    count += last = fprintf(file, "\n");

    return count;
}

int GenerateCVCProject(char* packageName, FILE* file, int argc, FileList* fileList)
{
    int idx, last, count = 0;

    for(idx = 0; idx < sizeof(CVCProjectHead) / sizeof(CVCProjectHead[0]); idx++)
    {
        count += last = fprintf(file, CVCProjectHead[idx], packageName);
        fputs("\n", file); count++;
    }

    count += last = fprintf(file, "\t\t<Filter Name=\"Source Files\" Filter=\"c\">", packageName); fputs("\n", file); count++;
    count += last = fprintf(file, "\t\t\t<File RelativePath=\"./%s_cksum.c\"></File>", packageName); fputs("\n", file); count++;
    count += last = fprintf(file, "\t\t\t<File RelativePath=\"./%s_Main.c\"></File>", packageName); fputs("\n", file); count++;
    for(idx = 0; idx < argc; idx++)
    {
        count += last = fprintf(file, "\t\t\t<File RelativePath=\"./%s\"></File>", fileList[idx].packageCFiles);
        fputs("\n", file); count++;
    }
    count += last = fprintf(file, "\t\t</Filter>", packageName); fputs("\n", file); count++;

    count += last = fprintf(file, "\t\t<Filter Name=\"Header Files\" Filter=\"h\">", packageName); fputs("\n", file); count++;
    for(idx = 0; idx < argc; idx++)
    {
        count += last = fprintf(file, "\t\t\t<File RelativePath=\"./%s\"></File>", fileList[idx].packageHFiles);
        fputs("\n", file); count++;
    }
    count += last = fprintf(file, "\t\t</Filter>", packageName); fputs("\n", file); count++;

    count += last = fprintf(file, "\t</Files>", packageName); fputs("\n", file); count++;
    count += last = fprintf(file, "</VisualStudioProject>", packageName); fputs("\n", file); count++;

    return count;
}

int GenerateCHeader(char* projectName, char* moduleName, FILE* file, GeneratorChainRoutine handler, void* userData1, void* userData2)
{
    int idx, last, count = 0;
    for(idx = 0; idx < sizeof(Header) / sizeof(Header[0]); idx++)
    {
        count += last = fprintf(file, Header[idx], projectName, moduleName);
        fputs("\n", file); count++;
    }
    if(handler) count += last = handler(projectName, moduleName, file, userData1, userData2);
    for(idx = 0; idx < sizeof(Footer) / sizeof(Footer[0]); idx++)
    {
        count += last = fprintf(file, Footer[idx], projectName, moduleName);
        fputs("\n", file); count++;
    }
    return count;
}

int GenerateCFileHeader(char* projectName, char* moduleName, FILE* file, void* userData1, void* userData2)
{
    int idx, last, count = 0;

    for(idx = 0; idx < sizeof(CFileHeaderBody) / sizeof(CFileHeaderBody[0]); idx++)
    {
        count += last = fprintf(file, CFileHeaderBody[idx], projectName, moduleName);
        fputs("\n", file); count++;
    }

    if(outputLines)
    {
        count += last = fprintf(file, "", projectName, moduleName); fputs("\n", file); count++;
        if(useDefines){
            count += last = fprintf(file, "#ifdef %s_%s_LOAD_LINES", projectName, moduleName); fputs("\n", file); count++;}
        count += last = fprintf(file, "extern char* %s_%s_LineContent[];", projectName, moduleName); fputs("\n", file); count++;
        if(useDefines){
            count += last = fprintf(file, "#endif", projectName, moduleName); fputs("\n", file); count++;}
    }
    if(outputBlocks)
    {
        count += last = fprintf(file, "", projectName, moduleName); fputs("\n", file); count++;
        if(useDefines){
            count += last = fprintf(file, "#ifdef %s_%s_LOAD_BLOCK", projectName, moduleName); fputs("\n", file); count++;}
        count += last = fprintf(file, "extern char %s_%s_BlockContent[][%d];", projectName, moduleName, blockSize); fputs("\n", file); count++;
        if(useDefines){
            count += last = fprintf(file, "#endif", projectName, moduleName); fputs("\n", file); count++;}
    }
    if(outputBlocks || outputLines)
    {
        count += last = fprintf(file, "", projectName, moduleName); fputs("\n", file); count++;
        if(useDefines){
            count += last = fprintf(file, "#if defined(%s_%s_LOAD_LINES) || defined(%s_%s_LOAD_BLOCK)", projectName, moduleName, projectName, moduleName); fputs("\n", file); count++;}
        count += last = fprintf(file, "size_t %s_%s_DumpContentsToFile(FILE* file);", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "size_t %s_%s_DumpContentsToNewFile(const char* fileName);", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "size_t %s_%s_DumpContents();", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "size_t %s_%s_Touch(int forceWrite);", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "size_t %s_%s_Unlink();", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "size_t %s_%s_Verify();", projectName, moduleName); fputs("\n", file); count++;
        if(useDefines){
            count += last = fprintf(file, "#endif", projectName, moduleName); fputs("\n", file); count++;}
    }

    return count;
}

int GenerateCFile(char* fileName, int mode, char* projectName, char* moduleName, FILE* file, void* userData1, void* userData2)
{
    FILE* inFile = (FILE*)userData1;
    PCFileInfo info = (PCFileInfo)userData2;
    int /*idx,*/ last, count = 0;

    count += last = fprintf(file, "#ifdef  __cplusplus", projectName, moduleName); fputs("\n", file); count++;
    count += last = fprintf(file, "extern \"C\" {", projectName, moduleName); fputs("\n", file); count++;
    count += last = fprintf(file, "#endif", projectName, moduleName); fputs("\n", file); count++;
    count += last = fprintf(file, "", projectName, moduleName); fputs("\n", file); count++;
    count += last = fprintf(file, "#include \"%s_%s.h\"", projectName, moduleName); fputs("\n", file); count++;
    count += last = fprintf(file, "#ifdef WIN32", projectName, moduleName); fputs("\n", file); count++;
    count += last = fprintf(file, "#include <io.h>", projectName, moduleName); fputs("\n", file); count++;
    count += last = fprintf(file, "#else", projectName, moduleName); fputs("\n", file); count++;
    count += last = fprintf(file, "#include <unistd.h>", projectName, moduleName); fputs("\n", file); count++;
    count += last = fprintf(file, "#endif", projectName, moduleName); fputs("\n", file); count++;
    count += last = fprintf(file, "", projectName, moduleName); fputs("\n", file); count++;
    count += last = fprintf(file, "#include <sys/stat.h>", projectName, moduleName); fputs("\n", file); count++;
    count += last = fprintf(file, "", projectName, moduleName); fputs("\n", file); count++;
    count += last = fprintf(file, "const size_t %s_%s_FileSize = %uUL;", projectName, moduleName, info->FileSize); fputs("\n", file); count++;
    count += last = fprintf(file, "const size_t %s_%s_LineCount = %uUL;", projectName, moduleName, info->LineCount); fputs("\n", file); count++;
    count += last = fprintf(file, "const size_t %s_%s_MaxLine = %uUL;", projectName, moduleName, info->MaxLine); fputs("\n", file); count++;
    count += last = fprintf(file, "const size_t %s_%s_Crc = %uUL;", projectName, moduleName, info->Crc); fputs("\n", file); count++;
    count += last = fprintf(file, "const char %s_%s_Name[] =\"%s\";", projectName, moduleName, fileName); fputs("\n", file); count++;
    count += last = fprintf(file, "const int %s_%s_Mode = %d;", projectName, moduleName, mode); fputs("\n", file); count++;

    if(outputLines)
    {
        count += last = fprintf(file, "", projectName, moduleName); fputs("\n", file); count++;
        if(useDefines){
            count += last = fprintf(file, "#ifdef %s_%s_LOAD_LINES", projectName, moduleName); fputs("\n", file); count++;}
        count += last = fprintf(file, "char* %s_%s_LineContent[] =", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "{", projectName, moduleName); fputs("\n", file); count++;

        {
            size_t len, lineLen;
            char buffer[1024+1];

            lineLen = 0;
            fseek(inFile, 0, SEEK_SET);
            while(feof(inFile) != EOF)
            {
                if(len = FileGetLine(buffer, sizeof(buffer), inFile))
                {
                    if(lineLen == 0) count += last = fprintf(file, "\t\"");
                    lineLen += len;
                    PrintString(buffer, file, (int)len);
                    if(buffer[len-1] == '\r' || buffer[len-1] == '\n')
                    {
                        lineLen = 0;
                        count += last = fprintf(file, "\",\n");
                    }
                }
                else break;
            }
            if(lineLen)
            {
                count += last = fprintf(file, "\",\n");
            }
        }

        count += last = fprintf(file, "};", projectName, moduleName); fputs("\n", file); count++;
        if(useDefines){
            count += last = fprintf(file, "#endif", projectName, moduleName); fputs("\n", file); count++;}
    }

    if(outputBlocks)
    {
        count += last = fprintf(file, "", projectName, moduleName); fputs("\n", file); count++;
        if(useDefines){
            count += last = fprintf(file, "#ifdef %s_%s_LOAD_BLOCK", projectName, moduleName); fputs("\n", file); count++;}
        count += last = fprintf(file, "char %s_%s_BlockContent[][%d] =", projectName, moduleName, blockSize); fputs("\n", file); count++;
        count += last = fprintf(file, "{", projectName, moduleName); fputs("\n", file); count++;

        {
            size_t len;
            char buffer[blockSize];

            fseek(inFile, 0, SEEK_SET);
            while(feof(inFile) != EOF)
            {
                if(len = fread(buffer, 1, sizeof(buffer), inFile))
                {
                    count += last = fprintf(file, "\t\"");
                    PrintString(buffer, file, (int)len);
                    count += last = fprintf(file, "\",\n");
                }
                else break;
            }
        }

        count += last = fprintf(file, "};", projectName, moduleName); fputs("\n", file); count++;
        if(useDefines){
            count += last = fprintf(file, "#endif", projectName, moduleName); fputs("\n", file); count++;}
    }

    if(outputBlocks || outputLines)
    {
        count += last = fprintf(file, "", projectName, moduleName); fputs("\n", file); count++;
        if(useDefines){
            count += last = fprintf(file, "#if defined(%s_%s_LOAD_LINES) || defined(%s_%s_LOAD_BLOCK)", projectName, moduleName, projectName, moduleName); fputs("\n", file); count++;}
        count += last = fprintf(file, "size_t %s_%s_DumpContentsToFile(FILE* file)", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "{", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "    if(!file) return 0;", projectName, moduleName); fputs("\n", file); count++;
        if(useDefines){
            count += last = fprintf(file, "    #if defined(%s_%s_LOAD_BLOCK)", projectName, moduleName); fputs("\n", file); count++;}
        if(useDefines || (!useDefines && outputBlocks)){
            count += last = fprintf(file, "        fwrite(%s_%s_BlockContent, %s_%s_FileSize, 1, file);", projectName, moduleName, projectName, moduleName); fputs("\n", file); count++;
            count += last = fprintf(file, "        return %s_%s_FileSize;", projectName, moduleName); fputs("\n", file); count++;}
        if(useDefines){
            count += last = fprintf(file, "    #elif defined(%s_%s_LOAD_LINES)", projectName, moduleName); fputs("\n", file); count++;}
        if(useDefines || (!useDefines && !outputBlocks && outputLines)){
            count += last = fprintf(file, "        {", projectName, moduleName); fputs("\n", file); count++;
            count += last = fprintf(file, "            size_t idx;", projectName, moduleName); fputs("\n", file); count++;
            count += last = fprintf(file, "            for(idx = 0; idx < %s_%s_LineCount; idx++)", projectName, moduleName); fputs("\n", file); count++;
            count += last = fprintf(file, "                fputs(%s_%s_LineContent[idx], file);", projectName, moduleName); fputs("\n", file); count++;
            count += last = fprintf(file, "        }", projectName, moduleName); fputs("\n", file); count++;
            count += last = fprintf(file, "        return %s_%s_FileSize;", projectName, moduleName); fputs("\n", file); count++;}
        if(useDefines){
            count += last = fprintf(file, "    #else", projectName, moduleName); fputs("\n", file); count++;
            count += last = fprintf(file, "        return 0;", projectName, moduleName); fputs("\n", file); count++;
            count += last = fprintf(file, "    #endif", projectName, moduleName); fputs("\n", file); count++;}
        count += last = fprintf(file, "}", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "size_t %s_%s_DumpContentsToNewFile(const char* fileName)", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "{", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "    FILE* file = NULL;", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "    if(!fileName) return 0;", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "    file = fopen(fileName, \"wb\");", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "    if(file)", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "    {", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "        size_t count = %s_%s_DumpContentsToFile(file);", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "#ifdef WIN32", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "        _chmod(fileName, %s_%s_Mode);", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "#else", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "        chmod(fileName, %s_%s_Mode);", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "#endif", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "        fclose(file);", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "        return count;", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "    }", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "    return 0;", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "}", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "size_t %s_%s_DumpContents()", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "{", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "    if(%s_%s_Touch(1))", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "    {", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "        size_t size = %s_%s_DumpContentsToNewFile(", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "                                           %s_%s_Name", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "                                           );", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "        %s_%s_Touch(0);", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "        return size;", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "    }", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "    else", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "        return 0;", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "}", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "size_t %s_%s_Touch(int forceWrite)", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "{", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "    char NameCopy[sizeof(%s_%s_Name)];", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "    size_t nameLen = sizeof(NameCopy) - 1;", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "    size_t idx;", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "    char tmp;", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "    FILE* file;", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "    memcpy(NameCopy, %s_%s_Name, sizeof(NameCopy));", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "    for(idx = 0; idx < nameLen; idx++)", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "    {", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "        if(strchr(\"\\\\/\", NameCopy[idx]))", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "        {", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "            tmp = NameCopy[idx];", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "            NameCopy[idx] = 0;", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "#ifdef WIN32", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "            _mkdir(NameCopy);", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "#else", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "            mkdir(NameCopy, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "#endif", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "            NameCopy[idx] = tmp;", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "        }", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "    }", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "    if(forceWrite)", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "#ifdef WIN32", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "        _chmod(NameCopy, _S_IREAD | _S_IWRITE);", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "#else", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "        chmod(NameCopy, S_IRUSR | S_IWUSR);", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "#endif", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "    if(file = fopen(NameCopy, \"ab+\"))", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "    {", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "        fclose(file);", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "        if(!forceWrite)", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "#ifdef WIN32", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "            _chmod(NameCopy, %s_%s_Mode);", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "#else", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "            chmod(NameCopy, %s_%s_Mode);", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "#endif", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "        return 1;", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "    }", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "    else", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "        return 0;", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "}", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "size_t %s_%s_Unlink()", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "{", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "#ifdef WIN32", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "    return (size_t)_unlink(%s_%s_Name);", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "#else", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "    return (size_t)unlink(%s_%s_Name);", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "#endif", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "}", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "#ifdef  __cplusplus", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "extern \"C\" {", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "#endif", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "    unsigned long sum(unsigned long filecrc, unsigned char *bp, unsigned long n);", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "#ifdef  __cplusplus", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "}", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "#endif", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "size_t %s_%s_Verify()", projectName, moduleName); fputs("\n", file); count++;
        count += last = fprintf(file, "{", projectName, moduleName); fputs("\n", file); count++;
        if(useDefines){
            count += last = fprintf(file, "#if defined(%s_%s_LOAD_BLOCK)", projectName, moduleName); fputs("\n", file); count++;}
        if(useDefines || (!useDefines && outputBlocks)){
            count += last = fprintf(file, "    unsigned long crc = sum(0, (unsigned char*)%s_%s_BlockContent,", projectName, moduleName); fputs("\n", file); count++;
            count += last = fprintf(file, "                               (unsigned long)%s_%s_FileSize);", projectName, moduleName); fputs("\n", file); count++;
            count += last = fprintf(file, "    return (size_t)sum(crc, NULL, (unsigned long)%s_%s_FileSize);", projectName, moduleName); fputs("\n", file); count++;}
        if(useDefines){
            count += last = fprintf(file, "#elif defined(%s_%s_LOAD_LINES)", projectName, moduleName); fputs("\n", file); count++;}
        if(useDefines || (!useDefines && !outputBlocks && outputLines)){
            count += last = fprintf(file, "    {", projectName, moduleName); fputs("\n", file); count++;
            count += last = fprintf(file, "        unsigned long crc = 0;", projectName, moduleName); fputs("\n", file); count++;
            count += last = fprintf(file, "        unsigned long count = 0;", projectName, moduleName); fputs("\n", file); count++;
            count += last = fprintf(file, "        size_t idx;", projectName, moduleName); fputs("\n", file); count++;
            count += last = fprintf(file, "        for(idx = crc = 0; idx < %s_%s_LineCount; idx++)", projectName, moduleName); fputs("\n", file); count++;
            count += last = fprintf(file, "        {", projectName, moduleName); fputs("\n", file); count++;
            count += last = fprintf(file, "            count += (unsigned long)strlen(%s_%s_LineContent[idx]);", projectName, moduleName); fputs("\n", file); count++;
            count += last = fprintf(file, "            crc = sum((unsigned long)crc, (unsigned char*)%s_%s_LineContent[idx], count);", projectName, moduleName); fputs("\n", file); count++;
            count += last = fprintf(file, "        }", projectName, moduleName); fputs("\n", file); count++;
            count += last = fprintf(file, "        return (size_t)sum(crc, NULL, count);", projectName, moduleName); fputs("\n", file); count++;
            count += last = fprintf(file, "    }", projectName, moduleName); fputs("\n", file); count++;}
        if(useDefines){
            count += last = fprintf(file, "#else", projectName, moduleName); fputs("\n", file); count++;
            count += last = fprintf(file, "    return 0;", projectName, moduleName); fputs("\n", file); count++;
            count += last = fprintf(file, "#endif", projectName, moduleName); fputs("\n", file); count++;}
        count += last = fprintf(file, "}", projectName, moduleName); fputs("\n", file); count++;
        if(useDefines){
            count += last = fprintf(file, "#endif", projectName, moduleName); fputs("\n", file); count++;}
    }

    count += last = fprintf(file, "", projectName, moduleName); fputs("\n", file); count++;
    count += last = fprintf(file, "#ifdef  __cplusplus", projectName, moduleName); fputs("\n", file); count++;
    count += last = fprintf(file, "}", projectName, moduleName); fputs("\n", file); count++;
    count += last = fprintf(file, "#endif", projectName, moduleName); fputs("\n", file); count++;

    return count;
}

int ProcessStreamToBlocks(FILE* inFile, FILE* outFile)
{
    char buffer[blockSize];
    int last, count = 0;
    size_t len, lineLen;

    count += last = fprintf(outFile, "char BlockContent[][%d] = \n", blockSize);
    count += last = fprintf(outFile, "{\n");

    lineLen = 0;
    while(feof(inFile) != EOF)
    {
        if(len = fread(buffer, 1, sizeof(buffer), inFile))
        {
            count += last = fprintf(outFile, "\t\"");
            PrintString(buffer, outFile, (int)len);
            count += last = fprintf(outFile, "\",\n");
        }
        else break;
    }

    count += last = fprintf(outFile, "}\n");

    return count;
}
int ProcessStreamToLines(FILE* inFile, FILE* outFile)
{
    char buffer[1024+1];
    int last, count = 0;
    size_t len, lineLen;

    count += last = fprintf(outFile, "char* LineContent[] = \n");
    count += last = fprintf(outFile, "{\n");

    lineLen = 0;
    while(feof(inFile) != EOF)
    {
        if(len = FileGetLine(buffer, sizeof(buffer), inFile))
        {
            if(lineLen == 0) count += last = fprintf(outFile, "\t\"");
            lineLen += len;
            PrintString(buffer, outFile, (int)len);
            if(buffer[len-1] == '\r' || buffer[len-1] == '\n')
            {
                lineLen = 0;
                count += last = fprintf(outFile, "\",\n");
            }
        }
        else break;
    }
    if(lineLen)
    {
        count += last = fprintf(outFile, "\",\n");
    }

    count += last = fprintf(outFile, "}\n");

    return count;
}

int ProcessStreamToFile(FILE* inFile, FILE* outFile)
{
    char buffer[1024+1+3];
    char* stBuff = NULL;
    int count = 0;
    int startFlag, endFlag;
    size_t len, lineLen, endLen;
    int inPos, outPos, idx;

    lineLen = 0;
    startFlag = endFlag = 0;
    while(feof(inFile) != EOF)
    {
        if(len = FileGetLine(buffer, sizeof(buffer)-3, inFile))
        {
            endLen = len;
            stBuff = &buffer[0];

            if(lineLen == 0)
            {
                if(!startFlag && buffer[0] == '{')
                {
                    startFlag = 1;
                    stBuff = NULL;
                }
                else if(!endFlag && buffer[0] == '}')
                {
                    endFlag = 1;
                    stBuff = NULL;
                }
                else if(startFlag && !endFlag && buffer[0] == '\t' && buffer[1] == '\"')
                {
                    endLen -= 2;
                    stBuff = &buffer[2];
                }
            }

            lineLen += len;

            if(stBuff && startFlag && !endFlag)
            {
                inPos = 0;
                outPos = 0;

                stBuff[len + 0] = 0;
                stBuff[len + 1] = 0;
                stBuff[len + 2] = 0;


                if(stBuff[endLen-1] == '\r' || stBuff[endLen-1] == '\n')
                {
                    while(endLen && stBuff[(endLen--)-1] != '\"');
                }

                while(inPos < (int)endLen)
                {
                    stBuff[outPos++] = DecodeCString(&stBuff[inPos], &idx);
                    inPos += idx;
                }
                count += outPos;
                fwrite(stBuff, outPos, 1, outFile);
                fflush(outFile);
            }

            if(buffer[len-1] == '\r' || buffer[len-1] == '\n')
            {
                lineLen = 0;
            }
        }
        else break;
    }

    return count;
}

int main(int argc, char** argv)
{
    FileGlobList glob;

#ifdef WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
    _setmode(_fileno(stderr), _O_BINARY);
#endif

    while(argc > 1 && argv[1][0] == '-')
    {
        size_t idx;
        char* arg = &argv[1][1];
        size_t len = strlen(arg);

        if(arg[0] == '-')
        {
            if(arg[1] == 'd' &&
                arg[2] == 'i' &&
                arg[3] == 'r' &&
                arg[4] == '=' &&
                arg[5] != '\0')
            {
                strcpy(outputDir, &arg[5]);
#ifdef WIN32
                _getcwd(currentDir, sizeof(currentDir));
#else
                getcwd(currentDir, sizeof(currentDir));
#endif
            }
        }
        else
        {
            for(idx = 0; idx < len; idx++)
            {
                switch(arg[0])
                {
                case 'l':
                case 'L':
                    outputLines = 1;
                    break;
                case 'b':
                case 'B':
                    outputBlocks = 1;
                    break;
                case 'd':
                case 'D':
                    decode = 1;
                    break;
                case 'a':
                case 'A':
                    autoName = 1;
                    break;
                case 'm':
                case 'M':
                    mainProcedure = 1;
                    break;
                case 'n':
                case 'N':
                    useDefines = 0;
                    break;
                case 'g':
                case 'G':
                    useGlobbing = 1;
                    break;
                case 'v':
                case 'V':
                    verbose = 1;
                    break;
                case 'f':
                case 'F':
                    followLinks = 1;
                    break;
                }
                arg++;
            }
        }
        argc--;
        argv++;
    }

    if(useGlobbing && !outputLines && !outputBlocks && !decode) useGlobbing = 2;
    if(!outputLines && !outputBlocks && !decode)
    {
        outputLines = 1;
        outputBlocks = 1;
    }
    if(mainProcedure) autoName = 1;
    if(mainProcedure) useDefines = 0;

    if(useGlobbing == 2)
    {
        FileGlobPrint glob(stdout);
        size_t idx;

        fprintf(stderr, "\n");

        for(idx = 0; idx < (size_t)(argc-1); idx++)
            glob.MatchPattern(argv[idx+1]);

        if(glob.Count()) fprintf(stderr, "\nCount[%u]\n\n", glob.Count());
    }
    else if(!mainProcedure && (argc != (autoName ? 2 : 3)))
    {
        if(argc == 1 && ((outputLines + outputBlocks + decode) == 1))
        {
            if(outputLines)
                return ProcessStreamToLines(stdin, stdout);
            else if(outputBlocks)
                return ProcessStreamToBlocks(stdin, stdout);
            else if(decode)
                return ProcessStreamToFile(stdin, stdout);
        }
        else
        {
            fprintf(stderr, "\nUsage: CFile [Options] {Name} [FileName]\n"
                "\n"
                "   Options:\n"
                "  ----------\n"
                "   -L          Generate line output\n"
                "   -B          Generate block output\n"
                "   -D          Decode output\n"
                "   -A          Auto generate name\n"
                "   -M          Generate make project\n"
                "   -G          Use globbing\n"
                "   -N          Don't use hash defines\n"
                "   -F          Follow links\n"
                "   -V          Verbose output\n"
                "   --dir=?     Output directory\n"
                "\n"
                );
            return 0;
        }
    }
    else
    {
        int idx, jdx = 0;

        if(!useGlobbing) fileList = (FileList*)malloc(sizeof(FileList) * (argc-1));

        if(mainProcedure)
        {
            packageName = argv[1];
            argc--;
            argv++;
        }

        if(verbose) fprintf(stderr, "\n");

        for(idx = 0; idx < (autoName ? (argc-1) : 1); idx++)
        {
            CFileInfo info;
            FILE* inFile;
            FILE* outHeaderFile;
            FILE* outCFileFile;
            char projName[] = "CFile";
            char modName[1024+1];
            char inFilename[1024+1];
#ifdef WIN32
            struct _stat mode;
#else
            struct stat mode;
#endif

            memset(&mode, 0, sizeof(mode));

            if(useGlobbing)
            {
                glob.MatchPattern(argv[idx+1]);
            }
            else
            {
                ToSentenceCase(argv[idx+1], modName);
                if(!autoName)
                {
                    argc--;
                    argv++;
                }
                sprintf(inFilename, "%s", argv[idx+1]);

                sprintf(fileList[jdx].packageHFiles, "%s_%s.h", projName, modName);
                sprintf(fileList[jdx].packageCFiles, "%s_%s.c", projName, modName);
                sprintf(fileList[jdx].packageOFiles, "%s_%s.o", projName, modName);
                sprintf(fileList[jdx].packageNFiles, "%s_%s", projName, modName);
                strcpy(fileList[jdx].packageContents, inFilename);

                info = CountLinesInFile(inFilename);
                fileList[jdx].crc = info.Crc;
                if(info.FileSize)
                {
                    inFile = fopen(inFilename, "rb");
#ifdef WIN32
                    _stat(inFilename, &mode);
#else
                    stat(inFilename, &mode);
#endif

#ifdef WIN32
                    if(outputDir[0]) _chdir(outputDir);
#else
                    if(outputDir[0]) chdir(outputDir);
#endif
                    outHeaderFile = fopen(fileList[jdx].packageHFiles, "wb");
                    outCFileFile = fopen(fileList[jdx].packageCFiles, "wb");
#ifdef WIN32
                    if(outputDir[0]) _chdir(currentDir);
#else
                    if(outputDir[0]) chdir(currentDir);
#endif

                    if(!inFile || !outHeaderFile || !outCFileFile)
                    {
                        if(inFile) fclose(inFile);
                        if(outHeaderFile) fclose(outHeaderFile);
                        if(outCFileFile) fclose(outCFileFile);
                        return 0;
                    }

                    GenerateCHeader(projName, modName, outHeaderFile, GenerateCFileHeader, (void*)inFile, (void*)&info);
                    GenerateCFile(inFilename, mode.st_mode, projName, modName, outCFileFile, (void*)inFile, (void*)&info);

                    if(verbose) printf("%s\n", fileList[jdx].packageContents);

                    fclose(inFile);
                    fclose(outHeaderFile);
                    fclose(outCFileFile);
                    jdx++;
                }
            }
        }
        if(useGlobbing)
        {
            CFileInfo info;
            FILE* inFile;
            FILE* outHeaderFile;
            FILE* outCFileFile;
            char projName[] = "CFile";
            char modName[1024+1];
            char inFilename[1024+1];
#ifdef WIN32
            struct _stat mode;
#else
            struct stat mode;
#endif

            fileList = (FileList*)malloc(sizeof(FileList) * glob.size());

            for(FileGlobList::Iterator it = glob.begin(); it != glob.end(); ++it)
            {
                strcpy(inFilename, it->c_str());

                ToSentenceCase(inFilename, modName);

                sprintf(fileList[jdx].packageHFiles, "%s_%s.h", projName, modName);
                sprintf(fileList[jdx].packageCFiles, "%s_%s.c", projName, modName);
                sprintf(fileList[jdx].packageOFiles, "%s_%s.o", projName, modName);
                sprintf(fileList[jdx].packageNFiles, "%s_%s", projName, modName);

                strcpy(fileList[jdx].packageContents, inFilename);

                info = CountLinesInFile(inFilename);
                fileList[jdx].crc = info.Crc;
                if(info.FileSize)
                {
                    inFile = fopen(inFilename, "rb");
#ifdef WIN32
                    _stat(inFilename, &mode);
#else
                    stat(inFilename, &mode);
#endif

#ifdef WIN32
                    if(outputDir[0]) _chdir(outputDir);
#else
                    if(outputDir[0]) chdir(outputDir);
#endif
                    outHeaderFile = fopen(fileList[jdx].packageHFiles, "wb");
                    outCFileFile = fopen(fileList[jdx].packageCFiles, "wb");
#ifdef WIN32
                    if(outputDir[0]) _chdir(currentDir);
#else
                    if(outputDir[0]) chdir(currentDir);
#endif

                    if(!inFile || !outHeaderFile || !outCFileFile)
                    {
                        if(inFile) fclose(inFile);
                        if(outHeaderFile) fclose(outHeaderFile);
                        if(outCFileFile) fclose(outCFileFile);
                        return 0;
                    }

                    GenerateCHeader(projName, modName, outHeaderFile, GenerateCFileHeader, (void*)inFile, (void*)&info);
                    GenerateCFile(inFilename, mode.st_mode, projName, modName, outCFileFile, (void*)inFile, (void*)&info);

                    if(verbose) printf("%s\n", fileList[jdx].packageContents);

                    fclose(inFile);
                    fclose(outHeaderFile);
                    fclose(outCFileFile);
                    jdx++;
                }
            }
        }

        if(verbose && jdx) fprintf(stderr, "\nCount[%d]\n\n", jdx);

        if(mainProcedure && jdx)
        {
            FILE* makeFileFile;
            FILE* mainFile;
            FILE* cksumFile;
            FILE* projectFile;

            char makeFileFilename[1024+1];
            char mainFilename[1024+1];
            char cksumFilename[1024+1];
            char projectFilename[1024+1];

            sprintf(makeFileFilename, "makefile", packageName);
            sprintf(mainFilename, "%s_Main.c", packageName);
            sprintf(cksumFilename, "%s_cksum.c", packageName);
            sprintf(projectFilename, "%s.vcproj", packageName);

#ifdef WIN32
            if(outputDir[0]) _chdir(outputDir);
#else
            if(outputDir[0]) chdir(outputDir);
#endif

            makeFileFile = fopen(makeFileFilename, "wb");
            mainFile = fopen(mainFilename, "wb");
            cksumFile = fopen(cksumFilename, "wb");
            projectFile = fopen(projectFilename, "wb");

#ifdef WIN32
            if(outputDir[0]) _chdir(currentDir);
#else
            if(outputDir[0]) chdir(currentDir);
#endif

            qsort(fileList, jdx, sizeof(FileList), StrCmp);

            if(makeFileFile)
            {
                GenerateMakefile(packageName, makeFileFile, jdx, fileList);
                fclose(makeFileFile);
            }
            if(mainFile)
            {
                GenerateCMain(packageName, mainFile, jdx, fileList);
                fclose(mainFile);
            }
            if(projectFile)
            {
                GenerateCVCProject(packageName, projectFile, jdx, fileList);
                fclose(projectFile);
            }
            if(cksumFile)
            {
                CFile_Cksum_C_DumpContentsToFile(cksumFile);
                fclose(cksumFile);
            }
        }
    }
    return 0;
}
