#ifndef __CFile_Cksum_C_H__
#define __CFile_Cksum_C_H__

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <direct.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

extern const size_t CFile_Cksum_C_FileSize;
extern const size_t CFile_Cksum_C_LineCount;
extern const size_t CFile_Cksum_C_MaxLine;
extern const size_t CFile_Cksum_C_Crc;
extern const char CFile_Cksum_C_Name[];
extern const int CFile_Cksum_C_Mode;

extern char CFile_Cksum_C_BlockContent[][256];

size_t CFile_Cksum_C_DumpContentsToFile(FILE* file);
size_t CFile_Cksum_C_DumpContentsToNewFile(const char* fileName);
size_t CFile_Cksum_C_DumpContents();
size_t CFile_Cksum_C_Touch(int forceWrite);
size_t CFile_Cksum_C_Unlink();
size_t CFile_Cksum_C_Verify();

#ifdef  __cplusplus
}
#endif

#endif /* __CFile_Cksum_C_H__ */
