#include "ConvertString.h"
#include <stdlib.h>

typedef struct _character_conversion
{
    char    character;
    char    converted[5];
    char    length;
} CHARACTER_CONVERSION, *PCHARACTER_CONVERSION;

CHARACTER_CONVERSION
charConvTableFullDisplay[] =
{
    '\x00', "\\x00", 4, // 0
    '\x01', "\\x01", 4, // 1
    '\x02', "\\x02", 4, // 2
    '\x03', "\\x03", 4, // 3
    '\x04', "\\x04", 4, // 4
    '\x05', "\\x05", 4, // 5
    '\x06', "\\x06", 4, // 6
    '\x07', "\\x07", 4, // 7
    '\x08', "\\x08", 4, // 8
    '\x09', "\\x09", 4, // 9
    '\x0a', "\\x0a", 4, // 10
    '\x0b', "\\x0b", 4, // 11
    '\x0c', "\\x0c", 4, // 12
    '\x0d', "\\x0d", 4, // 13
    '\x0e', "\\x0e", 4, // 14
    '\x0f', "\\x0f", 4, // 15
    '\x10', "\\x10", 4, // 16
    '\x11', "\\x11", 4, // 17
    '\x12', "\\x12", 4, // 18
    '\x13', "\\x13", 4, // 19
    '\x14', "\\x14", 4, // 20
    '\x15', "\\x15", 4, // 21
    '\x16', "\\x16", 4, // 22
    '\x17', "\\x17", 4, // 23
    '\x18', "\\x18", 4, // 24
    '\x19', "\\x19", 4, // 25
    '\x1a', "\\x1a", 4, // 26
    '\x1b', "\\x1b", 4, // 27
    '\x1c', "\\x1c", 4, // 28
    '\x1d', "\\x1d", 4, // 29
    '\x1e', "\\x1e", 4, // 30
    '\x1f', "\\x1f", 4, // 31
    '\x20', "\\x20", 4, // 32
    '\x21', "\\x21", 4, // 33
    '\x22', "\\x22", 4, // 34
    '\x23', "\\x23", 4, // 35
    '\x24', "\\x24", 4, // 36
    '\x25', "\\x25", 4, // 37
    '\x26', "\\x26", 4, // 38
    '\x27', "\\x27", 4, // 39
    '\x28', "\\x28", 4, // 40
    '\x29', "\\x29", 4, // 41
    '\x2a', "\\x2a", 4, // 42
    '\x2b', "\\x2b", 4, // 43
    '\x2c', "\\x2c", 4, // 44
    '\x2d', "\\x2d", 4, // 45
    '\x2e', "\\x2e", 4, // 46
    '\x2f', "\\x2f", 4, // 47
    '\x30', "\\x30", 4, // 48
    '\x31', "\\x31", 4, // 49
    '\x32', "\\x32", 4, // 50
    '\x33', "\\x33", 4, // 51
    '\x34', "\\x34", 4, // 52
    '\x35', "\\x35", 4, // 53
    '\x36', "\\x36", 4, // 54
    '\x37', "\\x37", 4, // 55
    '\x38', "\\x38", 4, // 56
    '\x39', "\\x39", 4, // 57
    '\x3a', "\\x3a", 4, // 58
    '\x3b', "\\x3b", 4, // 59
    '\x3c', "\\x3c", 4, // 60
    '\x3d', "\\x3d", 4, // 61
    '\x3e', "\\x3e", 4, // 62
    '\x3f', "\\x3f", 4, // 63
    '\x40', "\\x40", 4, // 64
    '\x41', "\\x41", 4, // 65
    '\x42', "\\x42", 4, // 66
    '\x43', "\\x43", 4, // 67
    '\x44', "\\x44", 4, // 68
    '\x45', "\\x45", 4, // 69
    '\x46', "\\x46", 4, // 70
    '\x47', "\\x47", 4, // 71
    '\x48', "\\x48", 4, // 72
    '\x49', "\\x49", 4, // 73
    '\x4a', "\\x4a", 4, // 74
    '\x4b', "\\x4b", 4, // 75
    '\x4c', "\\x4c", 4, // 76
    '\x4d', "\\x4d", 4, // 77
    '\x4e', "\\x4e", 4, // 78
    '\x4f', "\\x4f", 4, // 79
    '\x50', "\\x50", 4, // 80
    '\x51', "\\x51", 4, // 81
    '\x52', "\\x52", 4, // 82
    '\x53', "\\x53", 4, // 83
    '\x54', "\\x54", 4, // 84
    '\x55', "\\x55", 4, // 85
    '\x56', "\\x56", 4, // 86
    '\x57', "\\x57", 4, // 87
    '\x58', "\\x58", 4, // 88
    '\x59', "\\x59", 4, // 89
    '\x5a', "\\x5a", 4, // 90
    '\x5b', "\\x5b", 4, // 91
    '\x5c', "\\x5c", 4, // 92
    '\x5d', "\\x5d", 4, // 93
    '\x5e', "\\x5e", 4, // 94
    '\x5f', "\\x5f", 4, // 95
    '\x60', "\\x60", 4, // 96
    '\x61', "\\x61", 4, // 97
    '\x62', "\\x62", 4, // 98
    '\x63', "\\x63", 4, // 99
    '\x64', "\\x64", 4, // 100
    '\x65', "\\x65", 4, // 101
    '\x66', "\\x66", 4, // 102
    '\x67', "\\x67", 4, // 103
    '\x68', "\\x68", 4, // 104
    '\x69', "\\x69", 4, // 105
    '\x6a', "\\x6a", 4, // 106
    '\x6b', "\\x6b", 4, // 107
    '\x6c', "\\x6c", 4, // 108
    '\x6d', "\\x6d", 4, // 109
    '\x6e', "\\x6e", 4, // 110
    '\x6f', "\\x6f", 4, // 111
    '\x70', "\\x70", 4, // 112
    '\x71', "\\x71", 4, // 113
    '\x72', "\\x72", 4, // 114
    '\x73', "\\x73", 4, // 115
    '\x74', "\\x74", 4, // 116
    '\x75', "\\x75", 4, // 117
    '\x76', "\\x76", 4, // 118
    '\x77', "\\x77", 4, // 119
    '\x78', "\\x78", 4, // 120
    '\x79', "\\x79", 4, // 121
    '\x7a', "\\x7a", 4, // 122
    '\x7b', "\\x7b", 4, // 123
    '\x7c', "\\x7c", 4, // 124
    '\x7d', "\\x7d", 4, // 125
    '\x7e', "\\x7e", 4, // 126
    '\x7f', "\\x7f", 4, // 127
    '\x80', "\\x80", 4, // 128
    '\x81', "\\x81", 4, // 129
    '\x82', "\\x82", 4, // 130
    '\x83', "\\x83", 4, // 131
    '\x84', "\\x84", 4, // 132
    '\x85', "\\x85", 4, // 133
    '\x86', "\\x86", 4, // 134
    '\x87', "\\x87", 4, // 135
    '\x88', "\\x88", 4, // 136
    '\x89', "\\x89", 4, // 137
    '\x8a', "\\x8a", 4, // 138
    '\x8b', "\\x8b", 4, // 139
    '\x8c', "\\x8c", 4, // 140
    '\x8d', "\\x8d", 4, // 141
    '\x8e', "\\x8e", 4, // 142
    '\x8f', "\\x8f", 4, // 143
    '\x90', "\\x90", 4, // 144
    '\x91', "\\x91", 4, // 145
    '\x92', "\\x92", 4, // 146
    '\x93', "\\x93", 4, // 147
    '\x94', "\\x94", 4, // 148
    '\x95', "\\x95", 4, // 149
    '\x96', "\\x96", 4, // 150
    '\x97', "\\x97", 4, // 151
    '\x98', "\\x98", 4, // 152
    '\x99', "\\x99", 4, // 153
    '\x9a', "\\x9a", 4, // 154
    '\x9b', "\\x9b", 4, // 155
    '\x9c', "\\x9c", 4, // 156
    '\x9d', "\\x9d", 4, // 157
    '\x9e', "\\x9e", 4, // 158
    '\x9f', "\\x9f", 4, // 159
    '\xa0', "\\xa0", 4, // 160
    '\xa1', "\\xa1", 4, // 161
    '\xa2', "\\xa2", 4, // 162
    '\xa3', "\\xa3", 4, // 163
    '\xa4', "\\xa4", 4, // 164
    '\xa5', "\\xa5", 4, // 165
    '\xa6', "\\xa6", 4, // 166
    '\xa7', "\\xa7", 4, // 167
    '\xa8', "\\xa8", 4, // 168
    '\xa9', "\\xa9", 4, // 169
    '\xaa', "\\xaa", 4, // 170
    '\xab', "\\xab", 4, // 171
    '\xac', "\\xac", 4, // 172
    '\xad', "\\xad", 4, // 173
    '\xae', "\\xae", 4, // 174
    '\xaf', "\\xaf", 4, // 175
    '\xb0', "\\xb0", 4, // 176
    '\xb1', "\\xb1", 4, // 177
    '\xb2', "\\xb2", 4, // 178
    '\xb3', "\\xb3", 4, // 179
    '\xb4', "\\xb4", 4, // 180
    '\xb5', "\\xb5", 4, // 181
    '\xb6', "\\xb6", 4, // 182
    '\xb7', "\\xb7", 4, // 183
    '\xb8', "\\xb8", 4, // 184
    '\xb9', "\\xb9", 4, // 185
    '\xba', "\\xba", 4, // 186
    '\xbb', "\\xbb", 4, // 187
    '\xbc', "\\xbc", 4, // 188
    '\xbd', "\\xbd", 4, // 189
    '\xbe', "\\xbe", 4, // 190
    '\xbf', "\\xbf", 4, // 191
    '\xc0', "\\xc0", 4, // 192
    '\xc1', "\\xc1", 4, // 193
    '\xc2', "\\xc2", 4, // 194
    '\xc3', "\\xc3", 4, // 195
    '\xc4', "\\xc4", 4, // 196
    '\xc5', "\\xc5", 4, // 197
    '\xc6', "\\xc6", 4, // 198
    '\xc7', "\\xc7", 4, // 199
    '\xc8', "\\xc8", 4, // 200
    '\xc9', "\\xc9", 4, // 201
    '\xca', "\\xca", 4, // 202
    '\xcb', "\\xcb", 4, // 203
    '\xcc', "\\xcc", 4, // 204
    '\xcd', "\\xcd", 4, // 205
    '\xce', "\\xce", 4, // 206
    '\xcf', "\\xcf", 4, // 207
    '\xd0', "\\xd0", 4, // 208
    '\xd1', "\\xd1", 4, // 209
    '\xd2', "\\xd2", 4, // 210
    '\xd3', "\\xd3", 4, // 211
    '\xd4', "\\xd4", 4, // 212
    '\xd5', "\\xd5", 4, // 213
    '\xd6', "\\xd6", 4, // 214
    '\xd7', "\\xd7", 4, // 215
    '\xd8', "\\xd8", 4, // 216
    '\xd9', "\\xd9", 4, // 217
    '\xda', "\\xda", 4, // 218
    '\xdb', "\\xdb", 4, // 219
    '\xdc', "\\xdc", 4, // 220
    '\xdd', "\\xdd", 4, // 221
    '\xde', "\\xde", 4, // 222
    '\xdf', "\\xdf", 4, // 223
    '\xe0', "\\xe0", 4, // 224
    '\xe1', "\\xe1", 4, // 225
    '\xe2', "\\xe2", 4, // 226
    '\xe3', "\\xe3", 4, // 227
    '\xe4', "\\xe4", 4, // 228
    '\xe5', "\\xe5", 4, // 229
    '\xe6', "\\xe6", 4, // 230
    '\xe7', "\\xe7", 4, // 231
    '\xe8', "\\xe8", 4, // 232
    '\xe9', "\\xe9", 4, // 233
    '\xea', "\\xea", 4, // 234
    '\xeb', "\\xeb", 4, // 235
    '\xec', "\\xec", 4, // 236
    '\xed', "\\xed", 4, // 237
    '\xee', "\\xee", 4, // 238
    '\xef', "\\xef", 4, // 239
    '\xf0', "\\xf0", 4, // 240
    '\xf1', "\\xf1", 4, // 241
    '\xf2', "\\xf2", 4, // 242
    '\xf3', "\\xf3", 4, // 243
    '\xf4', "\\xf4", 4, // 244
    '\xf5', "\\xf5", 4, // 245
    '\xf6', "\\xf6", 4, // 246
    '\xf7', "\\xf7", 4, // 247
    '\xf8', "\\xf8", 4, // 248
    '\xf9', "\\xf9", 4, // 249
    '\xfa', "\\xfa", 4, // 250
    '\xfb', "\\xfb", 4, // 251
    '\xfc', "\\xfc", 4, // 252
    '\xfd', "\\xfd", 4, // 253
    '\xfe', "\\xfe", 4, // 254
    '\xff', "\\xff", 4, // 255
};

CHARACTER_CONVERSION
charConvTableDisplay[] =
{
    '\x00', "\\000", 4, // 0
    '\x01', "\\001", 4, // 1
    '\x02', "\\002", 4, // 2
    '\x03', "\\003", 4, // 3
    '\x04', "\\004", 4, // 4
    '\x05', "\\005", 4, // 5
    '\x06', "\\006", 4, // 6
    '\a',   "\\a",   2, // 7
    '\b',   "\\b",   2, // 8
    '\t',   "\\t",   2, // 9
    '\n',   "\\n",   2, // 10
    '\v',   "\\v",   2, // 11
    '\f',   "\\f",   2, // 12
    '\r',   "\\r",   2, // 13
    '\x0e', "\\x0e", 4, // 14
    '\x0f', "\\x0f", 4, // 15
    '\x10', "\\x10", 4, // 16
    '\x11', "\\x11", 4, // 17
    '\x12', "\\x12", 4, // 18
    '\x13', "\\x13", 4, // 19
    '\x14', "\\x14", 4, // 20
    '\x15', "\\x15", 4, // 21
    '\x16', "\\x16", 4, // 22
    '\x17', "\\x17", 4, // 23
    '\x18', "\\x18", 4, // 24
    '\x19', "\\x19", 4, // 25
    '\x1a', "\\x1a", 4, // 26
    '\x1b', "\\x1b", 4, // 27
    '\x1c', "\\x1c", 4, // 28
    '\x1d', "\\x1d", 4, // 29
    '\x1e', "\\x1e", 4, // 30
    '\x1f', "\\x1f", 4, // 31
    '\x20', " ",     1, // 32
    '\x21', "!",     1, // 33
    '\"',   "\\\"",  2, // 34
    '\x23', "#",     1, // 35
    '\x24', "$",     1, // 36
    '\x25', "%",     1, // 37
    '\x26', "&",     1, // 38
    '\'',   "\\'",   2, // 39
    '\x28', "(",     1, // 40
    '\x29', ")",     1, // 41
    '\x2a', "*",     1, // 42
    '\x2b', "+",     1, // 43
    '\x2c', ",",     1, // 44
    '\x2d', "-",     1, // 45
    '\x2e', ".",     1, // 46
    '\x2f', "/",     1, // 47
    '\x30', "0",     1, // 48
    '\x31', "1",     1, // 49
    '\x32', "2",     1, // 50
    '\x33', "3",     1, // 51
    '\x34', "4",     1, // 52
    '\x35', "5",     1, // 53
    '\x36', "6",     1, // 54
    '\x37', "7",     1, // 55
    '\x38', "8",     1, // 56
    '\x39', "9",     1, // 57
    '\x3a', ":",     1, // 58
    '\x3b', ";",     1, // 59
    '\x3c', "<",     1, // 60
    '\x3d', "=",     1, // 61
    '\x3e', ">",     1, // 62
    '\x3f', "\\?",   2, // 63
    '\x40', "@",     1, // 64
    '\x41', "A",     1, // 65
    '\x42', "B",     1, // 66
    '\x43', "C",     1, // 67
    '\x44', "D",     1, // 68
    '\x45', "E",     1, // 69
    '\x46', "F",     1, // 70
    '\x47', "G",     1, // 71
    '\x48', "H",     1, // 72
    '\x49', "I",     1, // 73
    '\x4a', "J",     1, // 74
    '\x4b', "K",     1, // 75
    '\x4c', "L",     1, // 76
    '\x4d', "M",     1, // 77
    '\x4e', "N",     1, // 78
    '\x4f', "O",     1, // 79
    '\x50', "P",     1, // 80
    '\x51', "Q",     1, // 81
    '\x52', "R",     1, // 82
    '\x53', "S",     1, // 83
    '\x54', "T",     1, // 84
    '\x55', "U",     1, // 85
    '\x56', "V",     1, // 86
    '\x57', "W",     1, // 87
    '\x58', "X",     1, // 88
    '\x59', "Y",     1, // 89
    '\x5a', "Z",     1, // 90
    '\x5b', "[",     1, // 91
    '\\',   "\\\\",  2, // 92
    '\x5d', "]",     1, // 93
    '\x5e', "^",     1, // 94
    '\x5f', "_",     1, // 95
    '\x60', "`",     1, // 96
    '\x61', "a",     1, // 97
    '\x62', "b",     1, // 98
    '\x63', "c",     1, // 99
    '\x64', "d",     1, // 100
    '\x65', "e",     1, // 101
    '\x66', "f",     1, // 102
    '\x67', "g",     1, // 103
    '\x68', "h",     1, // 104
    '\x69', "i",     1, // 105
    '\x6a', "j",     1, // 106
    '\x6b', "k",     1, // 107
    '\x6c', "l",     1, // 108
    '\x6d', "m",     1, // 109
    '\x6e', "n",     1, // 110
    '\x6f', "o",     1, // 111
    '\x70', "p",     1, // 112
    '\x71', "q",     1, // 113
    '\x72', "r",     1, // 114
    '\x73', "s",     1, // 115
    '\x74', "t",     1, // 116
    '\x75', "u",     1, // 117
    '\x76', "v",     1, // 118
    '\x77', "w",     1, // 119
    '\x78', "x",     1, // 120
    '\x79', "y",     1, // 121
    '\x7a', "z",     1, // 122
    '\x7b', "{",     1, // 123
    '\x7c', "|",     1, // 124
    '\x7d', "}",     1, // 125
    '\x7e', "~",     1, // 126
    '\x7f', "",     1, // 127
    '\x80', "\x80",  1, // 128
    '\x81', "\x81",  1, // 129
    '\x82', "\x82",  1, // 130
    '\x83', "\x83",  1, // 131
    '\x84', "\x84",  1, // 132
    '\x85', "\x85",  1, // 133
    '\x86', "\x86",  1, // 134
    '\x87', "\x87",  1, // 135
    '\x88', "\x88",  1, // 136
    '\x89', "\x89",  1, // 137
    '\x8a', "\x8a",  1, // 138
    '\x8b', "\x8b",  1, // 139
    '\x8c', "\x8c",  1, // 140
    '\x8d', "\x8d",  1, // 141
    '\x8e', "\x8e",  1, // 142
    '\x8f', "\x8f",  1, // 143
    '\x90', "\x90",  1, // 144
    '\x91', "\x91",  1, // 145
    '\x92', "\x92",  1, // 146
    '\x93', "\x93",  1, // 147
    '\x94', "\x94",  1, // 148
    '\x95', "\x95",  1, // 149
    '\x96', "\x96",  1, // 150
    '\x97', "\x97",  1, // 151
    '\x98', "\x98",  1, // 152
    '\x99', "\x99",  1, // 153
    '\x9a', "\x9a",  1, // 154
    '\x9b', "\x9b",  1, // 155
    '\x9c', "\x9c",  1, // 156
    '\x9d', "\x9d",  1, // 157
    '\x9e', "\x9e",  1, // 158
    '\x9f', "\x9f",  1, // 159
    '\xa0', "\xa0",  1, // 160
    '\xa1', "\xa1",  1, // 161
    '\xa2', "\xa2",  1, // 162
    '\xa3', "\xa3",  1, // 163
    '\xa4', "\xa4",  1, // 164
    '\xa5', "\xa5",  1, // 165
    '\xa6', "\xa6",  1, // 166
    '\xa7', "\xa7",  1, // 167
    '\xa8', "\xa8",  1, // 168
    '\xa9', "\xa9",  1, // 169
    '\xaa', "\xaa",  1, // 170
    '\xab', "\xab",  1, // 171
    '\xac', "\xac",  1, // 172
    '\xad', "\xad",  1, // 173
    '\xae', "\xae",  1, // 174
    '\xaf', "\xaf",  1, // 175
    '\xb0', "\xb0",  1, // 176
    '\xb1', "\xb1",  1, // 177
    '\xb2', "\xb2",  1, // 178
    '\xb3', "\xb3",  1, // 179
    '\xb4', "\xb4",  1, // 180
    '\xb5', "\xb5",  1, // 181
    '\xb6', "\xb6",  1, // 182
    '\xb7', "\xb7",  1, // 183
    '\xb8', "\xb8",  1, // 184
    '\xb9', "\xb9",  1, // 185
    '\xba', "\xba",  1, // 186
    '\xbb', "\xbb",  1, // 187
    '\xbc', "\xbc",  1, // 188
    '\xbd', "\xbd",  1, // 189
    '\xbe', "\xbe",  1, // 190
    '\xbf', "\xbf",  1, // 191
    '\xc0', "\xc0",  1, // 192
    '\xc1', "\xc1",  1, // 193
    '\xc2', "\xc2",  1, // 194
    '\xc3', "\xc3",  1, // 195
    '\xc4', "\xc4",  1, // 196
    '\xc5', "\xc5",  1, // 197
    '\xc6', "\xc6",  1, // 198
    '\xc7', "\xc7",  1, // 199
    '\xc8', "\xc8",  1, // 200
    '\xc9', "\xc9",  1, // 201
    '\xca', "\xca",  1, // 202
    '\xcb', "\xcb",  1, // 203
    '\xcc', "\xcc",  1, // 204
    '\xcd', "\xcd",  1, // 205
    '\xce', "\xce",  1, // 206
    '\xcf', "\xcf",  1, // 207
    '\xd0', "\xd0",  1, // 208
    '\xd1', "\xd1",  1, // 209
    '\xd2', "\xd2",  1, // 210
    '\xd3', "\xd3",  1, // 211
    '\xd4', "\xd4",  1, // 212
    '\xd5', "\xd5",  1, // 213
    '\xd6', "\xd6",  1, // 214
    '\xd7', "\xd7",  1, // 215
    '\xd8', "\xd8",  1, // 216
    '\xd9', "\xd9",  1, // 217
    '\xda', "\xda",  1, // 218
    '\xdb', "\xdb",  1, // 219
    '\xdc', "\xdc",  1, // 220
    '\xdd', "\xdd",  1, // 221
    '\xde', "\xde",  1, // 222
    '\xdf', "\xdf",  1, // 223
    '\xe0', "\xe0",  1, // 224
    '\xe1', "\xe1",  1, // 225
    '\xe2', "\xe2",  1, // 226
    '\xe3', "\xe3",  1, // 227
    '\xe4', "\xe4",  1, // 228
    '\xe5', "\xe5",  1, // 229
    '\xe6', "\xe6",  1, // 230
    '\xe7', "\xe7",  1, // 231
    '\xe8', "\xe8",  1, // 232
    '\xe9', "\xe9",  1, // 233
    '\xea', "\xea",  1, // 234
    '\xeb', "\xeb",  1, // 235
    '\xec', "\xec",  1, // 236
    '\xed', "\xed",  1, // 237
    '\xee', "\xee",  1, // 238
    '\xef', "\xef",  1, // 239
    '\xf0', "\xf0",  1, // 240
    '\xf1', "\xf1",  1, // 241
    '\xf2', "\xf2",  1, // 242
    '\xf3', "\xf3",  1, // 243
    '\xf4', "\xf4",  1, // 244
    '\xf5', "\xf5",  1, // 245
    '\xf6', "\xf6",  1, // 246
    '\xf7', "\xf7",  1, // 247
    '\xf8', "\xf8",  1, // 248
    '\xf9', "\xf9",  1, // 249
    '\xfa', "\xfa",  1, // 250
    '\xfb', "\xfb",  1, // 251
    '\xfc', "\xfc",  1, // 252
    '\xfd', "\xfd",  1, // 253
    '\xfe', "\xfe",  1, // 254
    '\xff', "\\xff", 4, // 255
};


CHARACTER_CONVERSION
charConvTable[] =
{
    '\x00', "\\000", 4, // 0
    '\x01', "\\001", 4, // 1
    '\x02', "\\002", 4, // 2
    '\x03', "\\003", 4, // 3
    '\x04', "\\004", 4, // 4
    '\x05', "\\005", 4, // 5
    '\x06', "\\006", 4, // 6
    '\a',   "\\a",   2, // 7
    '\b',   "\\b",   2, // 8
    '\t',   "\\t",   2, // 9
    '\n',   "\\n",   2, // 10
    '\v',   "\\v",   2, // 11
    '\f',   "\\f",   2, // 12
    '\r',   "\\r",   2, // 13
    '\x0e', "\\x0e", 4, // 14
    '\x0f', "\\x0f", 4, // 15
    '\x10', "\\x10", 4, // 16
    '\x11', "\\x11", 4, // 17
    '\x12', "\\x12", 4, // 18
    '\x13', "\\x13", 4, // 19
    '\x14', "\\x14", 4, // 20
    '\x15', "\\x15", 4, // 21
    '\x16', "\\x16", 4, // 22
    '\x17', "\\x17", 4, // 23
    '\x18', "\\x18", 4, // 24
    '\x19', "\\x19", 4, // 25
    '\x1a', "\\x1a", 4, // 26
    '\x1b', "\\x1b", 4, // 27
    '\x1c', "\\x1c", 4, // 28
    '\x1d', "\\x1d", 4, // 29
    '\x1e', "\\x1e", 4, // 30
    '\x1f', "\\x1f", 4, // 31
    '\x20', " ",     1, // 32
    '\x21', "!",     1, // 33
    '\"',   "\\\"",  2, // 34
    '\x23', "#",     1, // 35
    '\x24', "$",     1, // 36
    '\x25', "%",     1, // 37
    '\x26', "&",     1, // 38
    '\'',   "\\'",   2, // 39
    '\x28', "(",     1, // 40
    '\x29', ")",     1, // 41
    '\x2a', "*",     1, // 42
    '\x2b', "+",     1, // 43
    '\x2c', ",",     1, // 44
    '\x2d', "-",     1, // 45
    '\x2e', ".",     1, // 46
    '\x2f', "/",     1, // 47
    '\x30', "0",     1, // 48
    '\x31', "1",     1, // 49
    '\x32', "2",     1, // 50
    '\x33', "3",     1, // 51
    '\x34', "4",     1, // 52
    '\x35', "5",     1, // 53
    '\x36', "6",     1, // 54
    '\x37', "7",     1, // 55
    '\x38', "8",     1, // 56
    '\x39', "9",     1, // 57
    '\x3a', ":",     1, // 58
    '\x3b', ";",     1, // 59
    '\x3c', "<",     1, // 60
    '\x3d', "=",     1, // 61
    '\x3e', ">",     1, // 62
    '\x3f', "\\?",   2, // 63
    '\x40', "@",     1, // 64
    '\x41', "A",     1, // 65
    '\x42', "B",     1, // 66
    '\x43', "C",     1, // 67
    '\x44', "D",     1, // 68
    '\x45', "E",     1, // 69
    '\x46', "F",     1, // 70
    '\x47', "G",     1, // 71
    '\x48', "H",     1, // 72
    '\x49', "I",     1, // 73
    '\x4a', "J",     1, // 74
    '\x4b', "K",     1, // 75
    '\x4c', "L",     1, // 76
    '\x4d', "M",     1, // 77
    '\x4e', "N",     1, // 78
    '\x4f', "O",     1, // 79
    '\x50', "P",     1, // 80
    '\x51', "Q",     1, // 81
    '\x52', "R",     1, // 82
    '\x53', "S",     1, // 83
    '\x54', "T",     1, // 84
    '\x55', "U",     1, // 85
    '\x56', "V",     1, // 86
    '\x57', "W",     1, // 87
    '\x58', "X",     1, // 88
    '\x59', "Y",     1, // 89
    '\x5a', "Z",     1, // 90
    '\x5b', "[",     1, // 91
    '\\',   "\\\\",  2, // 92
    '\x5d', "]",     1, // 93
    '\x5e', "^",     1, // 94
    '\x5f', "_",     1, // 95
    '\x60', "`",     1, // 96
    '\x61', "a",     1, // 97
    '\x62', "b",     1, // 98
    '\x63', "c",     1, // 99
    '\x64', "d",     1, // 100
    '\x65', "e",     1, // 101
    '\x66', "f",     1, // 102
    '\x67', "g",     1, // 103
    '\x68', "h",     1, // 104
    '\x69', "i",     1, // 105
    '\x6a', "j",     1, // 106
    '\x6b', "k",     1, // 107
    '\x6c', "l",     1, // 108
    '\x6d', "m",     1, // 109
    '\x6e', "n",     1, // 110
    '\x6f', "o",     1, // 111
    '\x70', "p",     1, // 112
    '\x71', "q",     1, // 113
    '\x72', "r",     1, // 114
    '\x73', "s",     1, // 115
    '\x74', "t",     1, // 116
    '\x75', "u",     1, // 117
    '\x76', "v",     1, // 118
    '\x77', "w",     1, // 119
    '\x78', "x",     1, // 120
    '\x79', "y",     1, // 121
    '\x7a', "z",     1, // 122
    '\x7b', "{",     1, // 123
    '\x7c', "|",     1, // 124
    '\x7d', "}",     1, // 125
    '\x7e', "~",     1, // 126
    '\x7f', "\\x7f", 4, // 127
    '\x80', "\\x80", 4, // 128
    '\x81', "\\x81", 4, // 129
    '\x82', "\\x82", 4, // 130
    '\x83', "\\x83", 4, // 131
    '\x84', "\\x84", 4, // 132
    '\x85', "\\x85", 4, // 133
    '\x86', "\\x86", 4, // 134
    '\x87', "\\x87", 4, // 135
    '\x88', "\\x88", 4, // 136
    '\x89', "\\x89", 4, // 137
    '\x8a', "\\x8a", 4, // 138
    '\x8b', "\\x8b", 4, // 139
    '\x8c', "\\x8c", 4, // 140
    '\x8d', "\\x8d", 4, // 141
    '\x8e', "\\x8e", 4, // 142
    '\x8f', "\\x8f", 4, // 143
    '\x90', "\\x90", 4, // 144
    '\x91', "\\x91", 4, // 145
    '\x92', "\\x92", 4, // 146
    '\x93', "\\x93", 4, // 147
    '\x94', "\\x94", 4, // 148
    '\x95', "\\x95", 4, // 149
    '\x96', "\\x96", 4, // 150
    '\x97', "\\x97", 4, // 151
    '\x98', "\\x98", 4, // 152
    '\x99', "\\x99", 4, // 153
    '\x9a', "\\x9a", 4, // 154
    '\x9b', "\\x9b", 4, // 155
    '\x9c', "\\x9c", 4, // 156
    '\x9d', "\\x9d", 4, // 157
    '\x9e', "\\x9e", 4, // 158
    '\x9f', "\\x9f", 4, // 159
    '\xa0', "\\xa0", 4, // 160
    '\xa1', "\\xa1", 4, // 161
    '\xa2', "\\xa2", 4, // 162
    '\xa3', "\\xa3", 4, // 163
    '\xa4', "\\xa4", 4, // 164
    '\xa5', "\\xa5", 4, // 165
    '\xa6', "\\xa6", 4, // 166
    '\xa7', "\\xa7", 4, // 167
    '\xa8', "\\xa8", 4, // 168
    '\xa9', "\\xa9", 4, // 169
    '\xaa', "\\xaa", 4, // 170
    '\xab', "\\xab", 4, // 171
    '\xac', "\\xac", 4, // 172
    '\xad', "\\xad", 4, // 173
    '\xae', "\\xae", 4, // 174
    '\xaf', "\\xaf", 4, // 175
    '\xb0', "\\xb0", 4, // 176
    '\xb1', "\\xb1", 4, // 177
    '\xb2', "\\xb2", 4, // 178
    '\xb3', "\\xb3", 4, // 179
    '\xb4', "\\xb4", 4, // 180
    '\xb5', "\\xb5", 4, // 181
    '\xb6', "\\xb6", 4, // 182
    '\xb7', "\\xb7", 4, // 183
    '\xb8', "\\xb8", 4, // 184
    '\xb9', "\\xb9", 4, // 185
    '\xba', "\\xba", 4, // 186
    '\xbb', "\\xbb", 4, // 187
    '\xbc', "\\xbc", 4, // 188
    '\xbd', "\\xbd", 4, // 189
    '\xbe', "\\xbe", 4, // 190
    '\xbf', "\\xbf", 4, // 191
    '\xc0', "\\xc0", 4, // 192
    '\xc1', "\\xc1", 4, // 193
    '\xc2', "\\xc2", 4, // 194
    '\xc3', "\\xc3", 4, // 195
    '\xc4', "\\xc4", 4, // 196
    '\xc5', "\\xc5", 4, // 197
    '\xc6', "\\xc6", 4, // 198
    '\xc7', "\\xc7", 4, // 199
    '\xc8', "\\xc8", 4, // 200
    '\xc9', "\\xc9", 4, // 201
    '\xca', "\\xca", 4, // 202
    '\xcb', "\\xcb", 4, // 203
    '\xcc', "\\xcc", 4, // 204
    '\xcd', "\\xcd", 4, // 205
    '\xce', "\\xce", 4, // 206
    '\xcf', "\\xcf", 4, // 207
    '\xd0', "\\xd0", 4, // 208
    '\xd1', "\\xd1", 4, // 209
    '\xd2', "\\xd2", 4, // 210
    '\xd3', "\\xd3", 4, // 211
    '\xd4', "\\xd4", 4, // 212
    '\xd5', "\\xd5", 4, // 213
    '\xd6', "\\xd6", 4, // 214
    '\xd7', "\\xd7", 4, // 215
    '\xd8', "\\xd8", 4, // 216
    '\xd9', "\\xd9", 4, // 217
    '\xda', "\\xda", 4, // 218
    '\xdb', "\\xdb", 4, // 219
    '\xdc', "\\xdc", 4, // 220
    '\xdd', "\\xdd", 4, // 221
    '\xde', "\\xde", 4, // 222
    '\xdf', "\\xdf", 4, // 223
    '\xe0', "\\xe0", 4, // 224
    '\xe1', "\\xe1", 4, // 225
    '\xe2', "\\xe2", 4, // 226
    '\xe3', "\\xe3", 4, // 227
    '\xe4', "\\xe4", 4, // 228
    '\xe5', "\\xe5", 4, // 229
    '\xe6', "\\xe6", 4, // 230
    '\xe7', "\\xe7", 4, // 231
    '\xe8', "\\xe8", 4, // 232
    '\xe9', "\\xe9", 4, // 233
    '\xea', "\\xea", 4, // 234
    '\xeb', "\\xeb", 4, // 235
    '\xec', "\\xec", 4, // 236
    '\xed', "\\xed", 4, // 237
    '\xee', "\\xee", 4, // 238
    '\xef', "\\xef", 4, // 239
    '\xf0', "\\xf0", 4, // 240
    '\xf1', "\\xf1", 4, // 241
    '\xf2', "\\xf2", 4, // 242
    '\xf3', "\\xf3", 4, // 243
    '\xf4', "\\xf4", 4, // 244
    '\xf5', "\\xf5", 4, // 245
    '\xf6', "\\xf6", 4, // 246
    '\xf7', "\\xf7", 4, // 247
    '\xf8', "\\xf8", 4, // 248
    '\xf9', "\\xf9", 4, // 249
    '\xfa', "\\xfa", 4, // 250
    '\xfb', "\\xfb", 4, // 251
    '\xfc', "\\xfc", 4, // 252
    '\xfd', "\\xfd", 4, // 253
    '\xfe', "\\xfe", 4, // 254
    '\xff', "\\xff", 4, // 255
};

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

int ConvertStringForFullDisplay(char* szIn, char* szOut, int nCountIn)
{
    PCHARACTER_CONVERSION conv = (PCHARACTER_CONVERSION)0;
    int nCountOut = 0;
    int i;

    for(i = 0; i < nCountIn; i++)
    {
        conv = &charConvTableFullDisplay[(unsigned char)szIn[i]];
        memcpy(&szOut[nCountOut], &conv->converted[0], conv->length);
        nCountOut += conv->length;
    }

    return nCountOut;
}

int ConvertStringForDisplay(char* szIn, char* szOut, int nCountIn)
{
    PCHARACTER_CONVERSION conv = (PCHARACTER_CONVERSION)0;
    int nCountOut = 0;
    int i;

    for(i = 0; i < nCountIn; i++)
    {
        conv = &charConvTableDisplay[(unsigned char)szIn[i]];
        memcpy(&szOut[nCountOut], &conv->converted[0], conv->length);
        nCountOut += conv->length;
    }

    return nCountOut;
}

int PrintStringForFullDisplay(char* szIn, FILE* output, int nCountIn)
{
    PCHARACTER_CONVERSION conv = (PCHARACTER_CONVERSION)0;
    int nCountOut = 0;
    int i;

    for(i = 0; i < nCountIn; i++)
    {
        conv = &charConvTableFullDisplay[(unsigned char)szIn[i]];
        fwrite(&conv->converted[0], conv->length, 1, output);
        nCountOut += conv->length;
    }

    return nCountOut;
}

int PrintStringForDisplay(char* szIn, FILE* output, int nCountIn)
{
    PCHARACTER_CONVERSION conv = (PCHARACTER_CONVERSION)0;
    int nCountOut = 0;
    int i;

    for(i = 0; i < nCountIn; i++)
    {
        conv = &charConvTableDisplay[(unsigned char)szIn[i]];
        fwrite(&conv->converted[0], conv->length, 1, output);
        nCountOut += conv->length;
    }

    return nCountOut;
}

int ConvertString(char* szIn, char* szOut, int nCountIn)
{
    PCHARACTER_CONVERSION conv = (PCHARACTER_CONVERSION)0;
    int nCountOut = 0;
    int i;
    char ch;

    if(!szIn) return nCountOut;
    if(!szOut) return nCountOut;
    if(!nCountIn) return nCountOut;

    for(i = 0; i < (nCountIn-1); i++)
    {
        if(szIn[i] >= 0 && szIn[i] <= 6)
        {
            if(szIn[i+1] < '0' || szIn[i+1] > '7')
            {
                ch = (char)(szIn[i] + '0');
                memcpy(&szOut[nCountOut++], "\\", 1);
                memcpy(&szOut[nCountOut++], &ch, 1);
                continue;
            }
        }

        conv = &charConvTable[(unsigned char)szIn[i]];
        if(conv->length == 4)
        {
            if((szIn[i+1] >= '0' && szIn[i+1] <= '9') ||
                (szIn[i+1] >= 'a' && szIn[i+1] <= 'f') ||
                (szIn[i+1] >= 'A' && szIn[i+1] <= 'F'))
            {
                char tmpBuf[5];
                sprintf(tmpBuf, "\\%03o", (int)(unsigned char)szIn[i]);
                memcpy(&szOut[nCountOut], tmpBuf, 4);
                nCountOut += 4;
                continue;
            }
        }

        memcpy(&szOut[nCountOut], &conv->converted[0], conv->length);
        nCountOut += conv->length;
    }

    if(szIn[i] >= 0 && szIn[i] <= 6)
    {
        ch = (char)(szIn[i] + '0');
        memcpy(&szOut[nCountOut++], "\\", 1);
        memcpy(&szOut[nCountOut++], &ch, 1);
    }
    else
    {
        conv = &charConvTable[(unsigned char)szIn[i]];
        memcpy(&szOut[nCountOut], &conv->converted[0], conv->length);
        nCountOut += conv->length;
    }

    return nCountOut;
}
int PrintString(char* szIn, FILE* output, int nCountIn)
{
    PCHARACTER_CONVERSION conv = (PCHARACTER_CONVERSION)0;
    int nCountOut = 0;
    int i;
    char ch;

    if(!szIn) return nCountOut;
    if(!output) return nCountOut;
    if(!nCountIn) return nCountOut;

    for(i = 0; i < (nCountIn-1); i++)
    {
        if(szIn[i] >= 0 && szIn[i] <= 6)
        {
            if(szIn[i+1] < '0' || szIn[i+1] > '7')
            {
                ch = (char)(szIn[i] + '0');
                fwrite("\\", 1, 1, output);
                fwrite(&ch, 1, 1, output);
                nCountOut += 2;
                continue;
            }
        }

        conv = &charConvTable[(unsigned char)szIn[i]];
        if(conv->length == 4)
        {
            if((szIn[i+1] >= '0' && szIn[i+1] <= '9') ||
                (szIn[i+1] >= 'a' && szIn[i+1] <= 'f') ||
                (szIn[i+1] >= 'A' && szIn[i+1] <= 'F'))
            {
                fprintf(output, "\\%03o", (int)(unsigned char)szIn[i]);
                nCountOut += 4;
                continue;
            }
        }

        fwrite(&conv->converted[0], conv->length, 1, output);
        nCountOut += conv->length;
    }

    if(szIn[i] >= 0 && szIn[i] <= 6)
    {
        ch = (char)(szIn[i] + '0');
        fwrite("\\", 1, 1, output);
        fwrite(&ch, 1, 1, output);
        nCountOut += 2;
    }
    else
    {
        conv = &charConvTable[(unsigned char)szIn[i]];
        fwrite(&conv->converted[0], conv->length, 1, output);
        nCountOut += conv->length;
    }

    return nCountOut;
}

size_t ConvertCString(char *str)
{
    size_t len = 0;
    int idx = 0;
    size_t inPos = 0;
    size_t outPos = 0;

    if(!str) return 0;
    len = strlen(str);

    while(inPos < len)
    {
        str[outPos++] = DecodeCString(&str[inPos], &idx);
        inPos += idx;
    }
    str[outPos] = 0;

    return (size_t)outPos;
}
