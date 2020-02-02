/*
Copyright (C) <2020> <duncan.forster@mac.com>

Permission to use, copy, modify, and/or distribute this software for any purpose
with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.
*/

#include "EncodeCStr.H"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct CharacterConversionTable
{
    const char character;
    const char converted[5];
    const char length;
};

const CharacterConversionTable characterConversionTable[] = {
    { '\x00', "\\000", 4 }, // 0
    { '\x01', "\\001", 4 }, // 1
    { '\x02', "\\002", 4 }, // 2
    { '\x03', "\\003", 4 }, // 3
    { '\x04', "\\004", 4 }, // 4
    { '\x05', "\\005", 4 }, // 5
    { '\x06', "\\006", 4 }, // 6
    { '\a', "\\007", 4 },   // 7
    { '\b', "\\x08", 4 },   // 8
    { '\t', "\\t", 2 },     // 9
    { '\n', "\\n", 2 },     // 10
    { '\v', "\\x0B", 4 },   // 11
    { '\f', "\\x0C", 4 },   // 12
    { '\r', "\\r", 2 },     // 13
    { '\x0e', "\\x0E", 4 }, // 14
    { '\x0f', "\\x0F", 4 }, // 15
    { '\x10', "\\x10", 4 }, // 16
    { '\x11', "\\x11", 4 }, // 17
    { '\x12', "\\x12", 4 }, // 18
    { '\x13', "\\x13", 4 }, // 19
    { '\x14', "\\x14", 4 }, // 20
    { '\x15', "\\x15", 4 }, // 21
    { '\x16', "\\x16", 4 }, // 22
    { '\x17', "\\x17", 4 }, // 23
    { '\x18', "\\x18", 4 }, // 24
    { '\x19', "\\x19", 4 }, // 25
    { '\x1a', "\\x1A", 4 }, // 26
    { '\x1b', "\\x1B", 4 }, // 27
    { '\x1c', "\\x1C", 4 }, // 28
    { '\x1d', "\\x1D", 4 }, // 29
    { '\x1e', "\\x1E", 4 }, // 30
    { '\x1f', "\\x1F", 4 }, // 31
    { '\x20', " ", 1 },     // 32
    { '\x21', "!", 1 },     // 33
    { '\"', "\\\"", 2 },    // 34
    { '\x23', "#", 1 },     // 35
    { '\x24', "$", 1 },     // 36
    { '\x25', "%", 1 },     // 37
    { '\x26', "&", 1 },     // 38
    { '\'', "\\'", 2 },     // 39
    { '\x28', "(", 1 },     // 40
    { '\x29', ")", 1 },     // 41
    { '\x2a', "*", 1 },     // 42
    { '\x2b', "+", 1 },     // 43
    { '\x2c', ",", 1 },     // 44
    { '\x2d', "-", 1 },     // 45
    { '\x2e', ".", 1 },     // 46
    { '\x2f', "/", 1 },     // 47
    { '\x30', "0", 1 },     // 48
    { '\x31', "1", 1 },     // 49
    { '\x32', "2", 1 },     // 50
    { '\x33', "3", 1 },     // 51
    { '\x34', "4", 1 },     // 52
    { '\x35', "5", 1 },     // 53
    { '\x36', "6", 1 },     // 54
    { '\x37', "7", 1 },     // 55
    { '\x38', "8", 1 },     // 56
    { '\x39', "9", 1 },     // 57
    { '\x3a', ":", 1 },     // 58
    { '\x3b', ";", 1 },     // 59
    { '\x3c', "<", 1 },     // 60
    { '\x3d', "=", 1 },     // 61
    { '\x3e', ">", 1 },     // 62
    { '\x3f', "\\?", 2 },   // 63
    { '\x40', "@", 1 },     // 64
    { '\x41', "A", 1 },     // 65
    { '\x42', "B", 1 },     // 66
    { '\x43', "C", 1 },     // 67
    { '\x44', "D", 1 },     // 68
    { '\x45', "E", 1 },     // 69
    { '\x46', "F", 1 },     // 70
    { '\x47', "G", 1 },     // 71
    { '\x48', "H", 1 },     // 72
    { '\x49', "I", 1 },     // 73
    { '\x4a', "J", 1 },     // 74
    { '\x4b', "K", 1 },     // 75
    { '\x4c', "L", 1 },     // 76
    { '\x4d', "M", 1 },     // 77
    { '\x4e', "N", 1 },     // 78
    { '\x4f', "O", 1 },     // 79
    { '\x50', "P", 1 },     // 80
    { '\x51', "Q", 1 },     // 81
    { '\x52', "R", 1 },     // 82
    { '\x53', "S", 1 },     // 83
    { '\x54', "T", 1 },     // 84
    { '\x55', "U", 1 },     // 85
    { '\x56', "V", 1 },     // 86
    { '\x57', "W", 1 },     // 87
    { '\x58', "X", 1 },     // 88
    { '\x59', "Y", 1 },     // 89
    { '\x5a', "Z", 1 },     // 90
    { '\x5b', "[", 1 },     // 91
    { '\\', "\\\\", 2 },    // 92
    { '\x5d', "]", 1 },     // 93
    { '\x5e', "^", 1 },     // 94
    { '\x5f', "_", 1 },     // 95
    { '\x60', "`", 1 },     // 96
    { '\x61', "a", 1 },     // 97
    { '\x62', "b", 1 },     // 98
    { '\x63', "c", 1 },     // 99
    { '\x64', "d", 1 },     // 100
    { '\x65', "e", 1 },     // 101
    { '\x66', "f", 1 },     // 102
    { '\x67', "g", 1 },     // 103
    { '\x68', "h", 1 },     // 104
    { '\x69', "i", 1 },     // 105
    { '\x6a', "j", 1 },     // 106
    { '\x6b', "k", 1 },     // 107
    { '\x6c', "l", 1 },     // 108
    { '\x6d', "m", 1 },     // 109
    { '\x6e', "n", 1 },     // 110
    { '\x6f', "o", 1 },     // 111
    { '\x70', "p", 1 },     // 112
    { '\x71', "q", 1 },     // 113
    { '\x72', "r", 1 },     // 114
    { '\x73', "s", 1 },     // 115
    { '\x74', "t", 1 },     // 116
    { '\x75', "u", 1 },     // 117
    { '\x76', "v", 1 },     // 118
    { '\x77', "w", 1 },     // 119
    { '\x78', "x", 1 },     // 120
    { '\x79', "y", 1 },     // 121
    { '\x7a', "z", 1 },     // 122
    { '\x7b', "{", 1 },     // 123
    { '\x7c', "|", 1 },     // 124
    { '\x7d', "}", 1 },     // 125
    { '\x7e', "~", 1 },     // 126
    { '\x7f', "\\x7F", 4 }, // 127
    { '\x80', "\\x80", 4 }, // 128
    { '\x81', "\\x81", 4 }, // 129
    { '\x82', "\\x82", 4 }, // 130
    { '\x83', "\\x83", 4 }, // 131
    { '\x84', "\\x84", 4 }, // 132
    { '\x85', "\\x85", 4 }, // 133
    { '\x86', "\\x86", 4 }, // 134
    { '\x87', "\\x87", 4 }, // 135
    { '\x88', "\\x88", 4 }, // 136
    { '\x89', "\\x89", 4 }, // 137
    { '\x8a', "\\x8A", 4 }, // 138
    { '\x8b', "\\x8B", 4 }, // 139
    { '\x8c', "\\x8C", 4 }, // 140
    { '\x8d', "\\x8D", 4 }, // 141
    { '\x8e', "\\x8E", 4 }, // 142
    { '\x8f', "\\x8F", 4 }, // 143
    { '\x90', "\\x90", 4 }, // 144
    { '\x91', "\\x91", 4 }, // 145
    { '\x92', "\\x92", 4 }, // 146
    { '\x93', "\\x93", 4 }, // 147
    { '\x94', "\\x94", 4 }, // 148
    { '\x95', "\\x95", 4 }, // 149
    { '\x96', "\\x96", 4 }, // 150
    { '\x97', "\\x97", 4 }, // 151
    { '\x98', "\\x98", 4 }, // 152
    { '\x99', "\\x99", 4 }, // 153
    { '\x9a', "\\x9A", 4 }, // 154
    { '\x9b', "\\x9B", 4 }, // 155
    { '\x9c', "\\x9C", 4 }, // 156
    { '\x9d', "\\x9D", 4 }, // 157
    { '\x9e', "\\x9E", 4 }, // 158
    { '\x9f', "\\x9F", 4 }, // 159
    { '\xa0', "\\xA0", 4 }, // 160
    { '\xa1', "\\xA1", 4 }, // 161
    { '\xa2', "\\xA2", 4 }, // 162
    { '\xa3', "\\xA3", 4 }, // 163
    { '\xa4', "\\xA4", 4 }, // 164
    { '\xa5', "\\xA5", 4 }, // 165
    { '\xa6', "\\xA6", 4 }, // 166
    { '\xa7', "\\xA7", 4 }, // 167
    { '\xa8', "\\xA8", 4 }, // 168
    { '\xa9', "\\xA9", 4 }, // 169
    { '\xaa', "\\xAA", 4 }, // 170
    { '\xab', "\\xAB", 4 }, // 171
    { '\xac', "\\xAC", 4 }, // 172
    { '\xad', "\\xAD", 4 }, // 173
    { '\xae', "\\xAE", 4 }, // 174
    { '\xaf', "\\xAF", 4 }, // 175
    { '\xb0', "\\xB0", 4 }, // 176
    { '\xb1', "\\xB1", 4 }, // 177
    { '\xb2', "\\xB2", 4 }, // 178
    { '\xb3', "\\xB3", 4 }, // 179
    { '\xb4', "\\xB4", 4 }, // 180
    { '\xb5', "\\xB5", 4 }, // 181
    { '\xb6', "\\xB6", 4 }, // 182
    { '\xb7', "\\xB7", 4 }, // 183
    { '\xb8', "\\xB8", 4 }, // 184
    { '\xb9', "\\xB9", 4 }, // 185
    { '\xba', "\\xBA", 4 }, // 186
    { '\xbb', "\\xBB", 4 }, // 187
    { '\xbc', "\\xBC", 4 }, // 188
    { '\xbd', "\\xBD", 4 }, // 189
    { '\xbe', "\\xBE", 4 }, // 190
    { '\xbf', "\\xBF", 4 }, // 191
    { '\xc0', "\\xC0", 4 }, // 192
    { '\xc1', "\\xC1", 4 }, // 193
    { '\xc2', "\\xC2", 4 }, // 194
    { '\xc3', "\\xC3", 4 }, // 195
    { '\xc4', "\\xC4", 4 }, // 196
    { '\xc5', "\\xC5", 4 }, // 197
    { '\xc6', "\\xC6", 4 }, // 198
    { '\xc7', "\\xC7", 4 }, // 199
    { '\xc8', "\\xC8", 4 }, // 200
    { '\xc9', "\\xC9", 4 }, // 201
    { '\xca', "\\xCA", 4 }, // 202
    { '\xcb', "\\xCB", 4 }, // 203
    { '\xcc', "\\xCC", 4 }, // 204
    { '\xcd', "\\xCD", 4 }, // 205
    { '\xce', "\\xCE", 4 }, // 206
    { '\xcf', "\\xCF", 4 }, // 207
    { '\xd0', "\\xD0", 4 }, // 208
    { '\xd1', "\\xD1", 4 }, // 209
    { '\xd2', "\\xD2", 4 }, // 210
    { '\xd3', "\\xD3", 4 }, // 211
    { '\xd4', "\\xD4", 4 }, // 212
    { '\xd5', "\\xD5", 4 }, // 213
    { '\xd6', "\\xD6", 4 }, // 214
    { '\xd7', "\\xD7", 4 }, // 215
    { '\xd8', "\\xD8", 4 }, // 216
    { '\xd9', "\\xD9", 4 }, // 217
    { '\xda', "\\xDA", 4 }, // 218
    { '\xdb', "\\xDB", 4 }, // 219
    { '\xdc', "\\xDC", 4 }, // 220
    { '\xdd', "\\xDD", 4 }, // 221
    { '\xde', "\\xDE", 4 }, // 222
    { '\xdf', "\\xDF", 4 }, // 223
    { '\xe0', "\\xE0", 4 }, // 224
    { '\xe1', "\\xE1", 4 }, // 225
    { '\xe2', "\\xE2", 4 }, // 226
    { '\xe3', "\\xE3", 4 }, // 227
    { '\xe4', "\\xE4", 4 }, // 228
    { '\xe5', "\\xE5", 4 }, // 229
    { '\xe6', "\\xE6", 4 }, // 230
    { '\xe7', "\\xE7", 4 }, // 231
    { '\xe8', "\\xE8", 4 }, // 232
    { '\xe9', "\\xE9", 4 }, // 233
    { '\xea', "\\xEA", 4 }, // 234
    { '\xeb', "\\xEB", 4 }, // 235
    { '\xec', "\\xEC", 4 }, // 236
    { '\xed', "\\xED", 4 }, // 237
    { '\xee', "\\xEE", 4 }, // 238
    { '\xef', "\\xEF", 4 }, // 239
    { '\xf0', "\\xF0", 4 }, // 240
    { '\xf1', "\\xF1", 4 }, // 241
    { '\xf2', "\\xF2", 4 }, // 242
    { '\xf3', "\\xF3", 4 }, // 243
    { '\xf4', "\\xF4", 4 }, // 244
    { '\xf5', "\\xF5", 4 }, // 245
    { '\xf6', "\\xF6", 4 }, // 246
    { '\xf7', "\\xF7", 4 }, // 247
    { '\xf8', "\\xF8", 4 }, // 248
    { '\xf9', "\\xF9", 4 }, // 249
    { '\xfa', "\\xFA", 4 }, // 250
    { '\xfb', "\\xFB", 4 }, // 251
    { '\xfc', "\\xFC", 4 }, // 252
    { '\xfd', "\\xFD", 4 }, // 253
    { '\xfe', "\\xFE", 4 }, // 254
    { '\xff', "\\xFF", 4 }, // 255
};

char DecodeCharacter(const char *szCHex, int *pCount)
{
    char ch = 0;
    int count = 0;
    char szHex[3];
    char szOct[4];

    if (!szCHex)
    {
        if (pCount)
            (*pCount) = count;
        return ch;
    }

    count++;

    if (szCHex[0] != '\\')
    {
        if (pCount)
            (*pCount) = count;
        return szCHex[0];
    }

    switch (szCHex[1])
    {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
            count += 1;
            szOct[0] = szCHex[1];
            szOct[1] = 0;
            if (szCHex[2] >= '0' && szCHex[2] <= '7')
            {
                count += 1;
                szOct[1] = szCHex[2];
                szOct[2] = 0;
                if (szCHex[3] >= '0' && szCHex[3] <= '7')
                {
                    count += 1;
                    szOct[2] = szCHex[3];
                    szOct[3] = 0;
                }
            }
            ch = (char)strtoul(szOct, NULL, 8);
            break;
        case 'a':
            count++;
            ch = '\a';
            break;
        case 'b':
            count++;
            ch = '\b';
            break;
        case 't':
            count++;
            ch = '\t';
            break;
        case 'n':
            count++;
            ch = '\n';
            break;
        case 'v':
            count++;
            ch = '\v';
            break;
        case 'f':
            count++;
            ch = '\f';
            break;
        case 'r':
            count++;
            ch = '\r';
            break;
        case '\"':
            count++;
            ch = '\"';
            break;
        case '\'':
            count++;
            ch = '\'';
            break;
        case '\?':
            count++;
            ch = '\?';
            break;
        case '\\':
            count++;
            ch = '\\';
            break;
        case 'x':
            // This is not actually correct
            // Hex characters may continue indefinitely
            // For ASCII only two characters are used
            count += 3;
            szHex[0] = szCHex[2];
            szHex[1] = szCHex[3];
            szHex[2] = 0;
            ch = (char)strtoul(szHex, NULL, 16);
            break;
    }

    if (pCount)
        (*pCount) = count;

    return ch;
}

unsigned int DecodeCStr(char *str)
{
    size_t len = 0;
    int idx = 0;
    size_t inPos = 0;
    size_t outPos = 0;

    if (!str)
        return 0;
    len = strlen(str);

    while (inPos < len)
    {
        str[outPos++] = DecodeCharacter(&str[inPos], &idx);
        inPos += idx;
    }
    str[outPos] = 0;

    return (size_t)outPos;
}

unsigned int ConvertBufferAsCStr(char *szOut, const char *szIn, const int nCountIn)
{
    int nCountOut = 0;
    int idx;
    char ch;

    if (!szIn)
        return nCountOut;
    if (!szOut)
        return nCountOut;
    if (!nCountIn)
        return nCountOut;

    for (idx = 0; idx < (nCountIn - 1); idx++)
    {
        if (szIn[idx] >= 0 && szIn[idx] <= 6)
        {
            if (szIn[idx + 1] < '0' || szIn[idx + 1] > '7')
            {
                ch = (char)(szIn[idx] + '0');
                memcpy(&szOut[nCountOut++], "\\", 1);
                memcpy(&szOut[nCountOut++], &ch, 1);
                continue;
            }
        }

        const CharacterConversionTable &conv = characterConversionTable[(unsigned char)szIn[idx]];
        if (conv.length == 4)
        {
            if ((szIn[idx + 1] >= '0' && szIn[idx + 1] <= '9') || (szIn[idx + 1] >= 'a' && szIn[idx + 1] <= 'f') ||
                (szIn[idx + 1] >= 'A' && szIn[idx + 1] <= 'F'))
            {
                char tmpBuf[5];
                sprintf(tmpBuf, "\\%03o", (int)(unsigned char)szIn[idx]);
                memcpy(&szOut[nCountOut], tmpBuf, 4);
                nCountOut += 4;
                continue;
            }
        }

        memcpy(&szOut[nCountOut], &conv.converted[0], conv.length);
        nCountOut += conv.length;
    }

    if (szIn[idx] >= 0 && szIn[idx] <= 6)
    {
        ch = (char)(szIn[idx] + '0');
        memcpy(&szOut[nCountOut++], "\\", 1);
        memcpy(&szOut[nCountOut++], &ch, 1);
    }
    else
    {
        const CharacterConversionTable &conv = characterConversionTable[(unsigned char)szIn[idx]];
        memcpy(&szOut[nCountOut], &conv.converted[0], conv.length);
        nCountOut += conv.length;
    }

    return nCountOut;
}

unsigned int PrintBufferAsCStr(FILE *output, const char *szIn, const int nCountIn)
{
    int nCountOut = 0;
    int idx;
    char ch;

    if (!szIn)
        return nCountOut;
    if (!output)
        return nCountOut;
    if (!nCountIn)
        return nCountOut;

    for (idx = 0; idx < (nCountIn - 1); idx++)
    {
        if (szIn[idx] >= 0 && szIn[idx] <= 6)
        {
            if (szIn[idx + 1] < '0' || szIn[idx + 1] > '7')
            {
                ch = (char)(szIn[idx] + '0');
                fwrite("\\", 1, 1, output);
                fwrite(&ch, 1, 1, output);
                nCountOut += 2;
                continue;
            }
        }

        const CharacterConversionTable &conv = characterConversionTable[(unsigned char)szIn[idx]];
        if (conv.length == 4)
        {
            if ((szIn[idx + 1] >= '0' && szIn[idx + 1] <= '9') || (szIn[idx + 1] >= 'a' && szIn[idx + 1] <= 'f') ||
                (szIn[idx + 1] >= 'A' && szIn[idx + 1] <= 'F'))
            {
                fprintf(output, "\\%03o", (int)(unsigned char)szIn[idx]);
                nCountOut += 4;
                continue;
            }
        }

        fwrite(&conv.converted[0], conv.length, 1, output);
        nCountOut += conv.length;
    }

    if (szIn[idx] >= 0 && szIn[idx] <= 6)
    {
        ch = (char)(szIn[idx] + '0');
        fwrite("\\", 1, 1, output);
        fwrite(&ch, 1, 1, output);
        nCountOut += 2;
    }
    else
    {
        const CharacterConversionTable &conv = characterConversionTable[(unsigned char)szIn[idx]];
        fwrite(&conv.converted[0], conv.length, 1, output);
        nCountOut += conv.length;
    }

    return nCountOut;
}
