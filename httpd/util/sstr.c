
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

uint8_t *str_nstr(uint8_t *s1, char *s2, size_t len)
{
    uint8_t  c1, c2;
    size_t  n;

    c2 = *(uint8_t *) s2++;
    n = strlen(s2);
    do {
        do {
            if (len-- == 0) {
                return NULL;
            }
            c1 = *s1++;

            if (c1 == 0) {
                return NULL;
            }

        } while (c1 != c2);

        if (n > len) {
            return NULL;
        }

    } while (strncmp((char *)s1,  s2, n) != 0);
    return --s1;
}




