#ifdef BIG_ENDIAN
#define EREV32(x) ((((x)&0xFF000000)>>24) | \
                   (((x)&0x00FF0000)>> 8) | \
                   (((x)&0x0000FF00)<< 8) | \
                   (((x)&0x000000FF)<<24))
#define FIXENDIAN32(x) (x)=EREV32((ulong)x)
#define FIXENDIAN16(x) (x)= ((((ushort)(x))>>8) | ((ushort)((x)<<8)))
#define COPYENDIAN32(x) EREV32((ulong)x)
#define COPYENDIAN16(x) ((((ushort)(x))>>8) | ((ushort)((x)<<8)))
#else
#define FIXENDIAN32(x)
#define FIXENDIAN16(x)
#define COPYENDIAN32(x) x
#define COPYENDIAN16(x) x
#endif
