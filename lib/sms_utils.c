/***
 * Monitoring Plugin - sms_utils.c
 **
 *
 * Copyright (C) 2012 Marius Rieder <marius.rieder@durchmesser.ch>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * $Id$
 */

#include "mp_common.h"
#include "sms_utils.h"

#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <unistd.h>

/**
 * GSM7 Charmap according to 3GPP TS 23.038
 */
wchar_t sms_gsm7[] = L"@£$¥èéùìòÇ\nØø\rÅå"  /* 0x00 */
                      "Δ_ΦΓΛΩΠΨΣΘΞ@ÆæßÉ"    /* 0x10 */
                      " !\"# %&'()*+,-./"   /* 0x20 */
                      "0123456789:;<=>?"    /* 0x30 */
                      "¡ABCDEFHGIJKLMNO"    /* 0x40 */
                      "PQRSTUVWXYiÄÖÑÜ§"    /* 0x50 */
                      "¿abcdefghijklmno"    /* 0x60 */
                      "pqrstuvwxyzäüñüà";   /* 0x70 */

wchar_t sms_gsm7_ext[] = L"          \n     "   /* 0x00 */
                          "    ^           "    /* 0x10 */
                          "        {}     \\"   /* 0x20 */
                          "            [~] "    /* 0x30 */
                          "|               "    /* 0x40 */
                          "                "    /* 0x50 */
                          "     €          "    /* 0x60 */
                          "                ";   /* 0x70 */

char *mp_sms_pin = NULL;

int mobile_at_command(int fd, const char *cmd, const char *opt,
        char ***answer, int *answers) {
    return mobile_at_command_input(fd, cmd, opt, NULL, answer, answers);
}
int mobile_at_command_input(int fd, const char *cmd, const char *opt,
        const char *input, char ***answer, int *answers) {
    char *buf;
    char *ptr;
    char *line;
    size_t len;
    fd_set rfds;
    int retval;
    struct timeval tv;

    buf = mp_malloc(64);

    // Build command string
    if (opt)
        mp_snprintf(buf, 64, "AT%s%s\r", cmd, opt);
    else
        mp_snprintf(buf, 64, "AT%s\r", cmd);

    // Send command
    len = write(fd, buf, strlen(buf));
    if (len != strlen(buf)) {
        if (mp_verbose > 0)
            fprintf(stderr, "Write to device failed. "
                    "Written %d of %d chars.\n", (int)len, (int)strlen(buf));
        return -1;
    }
    if (mp_verbose > 3)
        printf(" >> %s\n", buf);

    if (input) {
        len = write(fd, input, strlen(input));
        if (len != strlen(input)) {
            if (mp_verbose > 0)
                fprintf(stderr, "Write to device failed. "
                        "Written %d of %d chars.\n", (int)len,
                        (int)strlen(buf));
            return -1;
        }
        if (mp_verbose > 3)
            printf(" >> %s\n", input);
        len = write(fd, "\r\x1A", 2);
        if (len != 2) {
            if (mp_verbose > 0)
                fprintf(stderr, "Write to device failed. "
                        "Written %d of 2 chars.\n", (int)len);
            return -1;
        }

    }

    // Read answers
    len = 0;
    ptr = buf;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    if (answers)
        *answers = 0;
    while (1) {
        // Build select list;
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);

        // Wait for input
        retval = select(fd+1, &rfds, NULL, NULL, &tv);
        if (retval <= 0)
            return -1;

        // Read from serial
        ptr = buf;
        ptr += len;
        retval = read(fd, buf, 64-len);
        if (retval <= 0)
            return -1;

        // Fetch lines
        ptr = buf;
        while ((line = strsep(&ptr, "\r\n")) && ptr) {
            if (strlen(line) == 0)
                continue;
            if (mp_verbose > 3)
                printf(" << %s\n", line);
            // Status codes
            if (strcmp(line, "OK") == 0)
                return 0;
            if (strcmp(line, "ERROR") == 0)
                return 1;
            if (strncmp(line, "+CME ERROR: ", 12) == 0)
                return 1;
            // Answer
            if (strncmp(line, cmd, strlen(cmd)) == 0) {
                if(!answers)
                    continue;
                line += strlen(cmd) +2;
                *answer = mp_realloc(*answer, (sizeof(char **)*((*answers)+2)));
                (*answer)[*answers] = mp_strdup(line);
                (*answer)[(*answers)+1] = NULL;
                (*answers)++;
            }
        }
        // Move buffer
        if (line) {
            len = strlen(line);
            memmove(buf, line, len+1);
            len = strlen(buf);
        }
    }
    return 0;
}

char *sms_encode_number(const char *number) {
    int i;
    int len;
    char *n;
    char *dest;

    n = (char *)number;
    len = strlen(number);
    if (number[0] == '+') {
        len -= 1;
        n++;
    }
    len += len%2;

    dest = mp_malloc(len + 3);
    memset(dest, 0, len + 3);

    if (number[0] == '+')
        strcpy(dest, "91");
    else
        strcpy(dest, "81");

    for (i=0; i < len; i+=2) {
        dest[i+3] = n[i];
        dest[i+2] = n[i+1] != '\0' ? n[i+1] : 'F';
    }

    return dest;
}

char *sms_encode_text(const char *text) {
    int i;
    int len = 0;
    unsigned int bit;
    int bits;
    char *ptr;
    char *pdu;
    wchar_t *t = NULL;

    i = mbstowcs(NULL, text, 0);
    t = mp_malloc(sizeof(wchar_t)*(i+1));
    memset(t,0,i+1);
    i = mbstowcs(t, text, i+1);

    pdu = mp_malloc((i * 7)/4+3);
    memset(pdu,0,(i * 7)/4+3);
    sprintf(pdu, "XX");
    ptr = pdu;
    ptr += 2;

    bit = 0;
    bits = 0;

    for (i=0; i<wcslen(t); i++) {
        wchar_t *pos = wcschr(sms_gsm7, t[i]);
        if (pos == NULL) {
            pos = wcschr(sms_gsm7_ext, t[i]);
            if (pos == NULL) {
                continue;
            }
            // Excape
            bit |= 0x1B << bits;
            len += 1;
            bits += 7;
            bit |= (pos-sms_gsm7_ext) << bits;
            len += 1;
            bits += 7;
        } else {
            bit |= (pos-sms_gsm7) << bits;
            len += 1;
            bits += 7;
        }
        while (bits >= 8) {
            mp_sprintf(ptr, "%02X", (bit & 0xFF));
            ptr += 2;
            bit = bit >> 8;
            bits -= 8;
        }
    }
    if (bits > 0) {
        mp_sprintf(ptr, "%02X", (bit & 0xFF));
        ptr += 2;
    }

    // Add Length
    mp_asprintf(&ptr, "%02X", len);
    memcpy(pdu, ptr, 2);
    free(ptr);

    return pdu;
}

char *sms_encode_pdu(const char *smsc, const char *number, const char *text) {
    char *pdu;
    char *encSmsc = NULL;
    char *encNumber = NULL;
    char *encText = NULL;
    int len;

    len = 14;
    if (smsc) {
        encSmsc = sms_encode_number(smsc);
        len += strlen(encSmsc);
    }
    encNumber = sms_encode_number(number);
    len += strlen(encNumber);
    encText = sms_encode_text(text);
    len += strlen(encText);

    pdu = mp_malloc(len+1);
    memset(pdu, 0, len+1);

    if (smsc) {
        mp_asprintf(&pdu, "%02X%s0500%02X%s0000%s",
                strlen(encSmsc)/2, encSmsc,
                strlen(encNumber)-2, encNumber,
                encText);
        free(encSmsc);
    } else {
        mp_asprintf(&pdu, "000500%02X%s0000%s",
                strlen(encNumber)-2, encNumber,
                encText);
    }

    free(encNumber);
    free(encText);

    return pdu;
}

/* vim: set ts=4 sw=4 et syn=c : */
