#define UBYTE unsigned char
#define UWORD unsigned short
#define ULONG unsigned long
#define BYTE signed char
#define WORD short
#define LONG long

int LZXinit(int window);
int LZXdecompress(UBYTE *inbuf, UBYTE *outbuf, ULONG inlen, ULONG outlen);
