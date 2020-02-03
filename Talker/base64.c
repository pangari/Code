static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static char cd64[] =
{
    -9,-9,-9,-9,-9,-9,-9,-9,-9,                 // Decimal  0 -  8
    -5,-5,                                      // Whitespace: Tab and Linefeed
    -9,-9,                                      // Decimal 11 - 12
    -5,                                         // Whitespace: Carriage Return
    -9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,     // Decimal 14 - 26
    -9,-9,-9,-9,-9,                             // Decimal 27 - 31
    -5,                                         // Whitespace: Space
    -9,-9,-9,-9,-9,-9,-9,-9,-9,-9,              // Decimal 33 - 42
    62,                                         // Plus sign at decimal 43
    -9,                                         // Decimal 44
    0,                                          // Minus sign at decimal 45
    -9,                                         // Decimal 46
    63,                                         // Slash at decimal 47
    52,53,54,55,56,57,58,59,60,61,              // Numbers zero through nine
    -9,-9,-9,                                   // Decimal 58 - 60
    -1,                                         // Equals sign at decimal 61
    -9,-9,-9,                                   // Decimal 62 - 64
    0,1,2,3,4,5,6,7,8,9,10,11,12,               // Letters 'A' through 'M'
    13,14,15,16,17,18,19,20,21,22,23,24,25,     // Letters 'N' through 'Z'
    -9,-9,-9,-9,                                // Decimal 91 - 94
    -9,                                         // Underscore at decimal 95
    -9,                                         // Decimal 96
    26,27,28,29,30,31,32,33,34,35,36,37,38,     // Letters 'a' through 'm'
    39,40,41,42,43,44,45,46,47,48,49,50,51,     // Letters 'n' through 'z'
    -9,-9,-9,-9,                                // Decimal 123 - 126
    -9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,     // Decimal 127 - 139
    -9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,     // Decimal 140 - 152
    -9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,     // Decimal 153 - 165
    -9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,     // Decimal 166 - 178
    -9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,     // Decimal 179 - 191
    -9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,     // Decimal 192 - 204
    -9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,     // Decimal 205 - 217
    -9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,     // Decimal 218 - 230
    -9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,     // Decimal 231 - 243
    -9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9         // Decimal 244 - 255
};

void encodeBase64( unsigned char in[3], unsigned char out[4], int len )
{
    out[0] = cb64[ in[0] >> 2 ];
    out[1] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xF0) >> 4) ];
    out[2] = (unsigned char) (len > 1 ? cb64[ ((in[1] & 0x0F) << 2) | ((in[2] & 0xC0) >> 6) ] : '=');
    out[3] = (unsigned char) (len > 2 ? cb64[ in[2] & 0x3F ] : '=');
}

void decodeBase64( unsigned char in[4], unsigned char out[3] )
{
    out[0] = (unsigned char) ((cd64[in[0]] << 2) | ((cd64[in[1]] >> 4) & 0x03));
    out[1] = (unsigned char) (cd64[in[2]] < 0) ? 0 : (cd64[in[1]] << 4 | (cd64[in[2]] >> 2));
    out[2] = (unsigned char) (cd64[in[3]] < 0) ? 0 : ((cd64[in[2]] << 6) | (cd64[in[3]] & 0x3F));
}
