#ifndef TYPES_H
#define TYPES_H

#define SWAP_16(x) (((x) & 0x00ff) << 8 | ((x) & 0xff00) >> 8)
#define SWAP_32(x) \
     ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) | \
      (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))
#define SWAP_64(x) ((SWAP_32((x) & 0xFFFFFFFF) << 32) | SWAP_32((x) >> 32))

#define XX_16(x) (*(unsigned short*)(x))
#define XX_32(x) (*(unsigned int*)(x))
#define XX_64(x) (*(unsigned long long int*)(x))
#define DEREF(x) (*(unsigned char*)(x))

#define LE_16(x) XX_16(x)
#define LE_32(x) XX_32(x)
#define LE_64(x) XX_64(x)
#define BE_16(x) SWAP_16(XX_16(x))
#define BE_32(x) SWAP_32(XX_32(x))
#define BE_64(x) ((void*)0)

#define BE_16_BUF(x) (((unsigned short )DEREF(x+0) <<  8) | ((unsigned short )DEREF(x+1)))
#define BE_32_BUF(x) (((unsigned int)DEREF(x+0) << 24) | ((unsigned int)DEREF(x+1) << 16) | \
                      ((unsigned int)DEREF(x+2) <<  8) | ((unsigned int)DEREF(x+3)))
#define BE_64_BUF(x) (((unsigned long long int)DEREF(x+0) << 56) | ((unsigned long long int)DEREF(x+1) << 48) | \
                      ((unsigned long long int)DEREF(x+2) << 40) | ((unsigned long long int)DEREF(x+3) << 32) | \
                      ((unsigned long long int)DEREF(x+4) << 24) | ((unsigned long long int)DEREF(x+5) << 16) | \
                      ((unsigned long long int)DEREF(x+6) <<  8) | ((unsigned long long int)DEREF(x+7)) )
#define LE_16_BUF(x) LE_16(x)
#define LE_32_BUF(x) LE_32(x)
#define LE_64_BUF(x) LE_64(x)

#endif
