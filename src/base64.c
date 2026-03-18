#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "base64.h"


size_t b64encode(char **dst, const char *src, size_t srclen)
{
    static char b64map[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t i = 0, j = 0;
    size_t dstlen = 4 * ceil(srclen / 3.0);
    *dst = (char *)calloc(dstlen + 1, sizeof(char));
    while (i < srclen) {
        (*dst)[j++] = b64map[((unsigned char)src[i] >> 2) & 0x3f];
        (*dst)[j++] = b64map[(((unsigned char)src[i] << 4) +
                              (i < srclen - 1
                                  ? ((unsigned char)src[i + 1] >> 4)
                                  : 0)) & 0x3f];
        (*dst)[j++] = i < srclen - 1
                          ? b64map[(((unsigned char)src[i + 1] << 2) +
                                    (i < srclen - 2
                                        ? (unsigned char)src[i + 2] >> 6
                                        : 0)) & 0x3f]
                          : '=';
        (*dst)[j++] = i < srclen - 2
                          ? b64map[(unsigned char)src[i + 2] & 0x3f]
                          : '=';
        i += 3;
    }
    return dstlen;
}

#ifdef TEST_BASE64
int main(int argc, char *argv[])
{
    int i;
    for (i = 1; i < argc; i++) {
        if (strlen(argv[i])) {
            char *s = NULL;
            if (!b64encode(&s, argv[i], strlen(argv[i]))) {
                fprintf(stderr, "b64encode(\"%s\")\n", argv[i]);
            } else {
                printf("%s => %s\n", argv[i], s);
                free(s);
            }
        }
    }
}
#endif  /* TEST_BASE64 */
