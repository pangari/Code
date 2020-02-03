#ifndef __BASE64_H__
#define __BASE64_H__

void encodeBase64( unsigned char in[3], unsigned char out[4], int len );
void decodeBase64( unsigned char in[4], unsigned char out[3] );

#endif /* __BASE64_H__ */
