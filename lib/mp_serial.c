/***
 * Monitoring Plugin - mp_serial.c
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
#include "mp_serial.h"

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>

/*
 * Global Vars
 */
char *mp_serial_device = NULL;
int   mp_serial_speed  = B9600;
struct termios save_tio;
speed_map speeds[] = {
    {"1200", B1200},    {"2400", B2400},    {"4800", B4800},
    {"9600", B9600},    {"19200", B19200},  {"38400", B38400},
    {"57600", B57600},  {"115200", B115200},{NULL, 0}
};

int mp_serial_open(const char *device, int speed) {
    int fd;
    struct termios tio;

    fd = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0)
        critical("Can't open Serial Port '%s'", device);

    // Save serial port setting
    tcgetattr(fd,&save_tio);
    // Setup Serial Port
    tio.c_cflag = mp_serial_speed | CS8 | CLOCAL | CREAD;
    tio.c_iflag = IGNPAR;
    tio.c_oflag = 0;
    tio.c_lflag = ICANON;
    tio.c_cc[VMIN]=1;
    tio.c_cc[VTIME]=0;
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd,TCSANOW,&tio);

    return fd;
}

int mp_serial_write(int fd, const char *buf) {
    fd_set fds;
    int ret;

    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    ret = select(fd+1, NULL, &fds, NULL, NULL);

    if(ret == -1)
        unknown("select failed");

    if(FD_ISSET(fd, &fds)) {
        ret = write(fd, buf, strlen(buf));
        write(fd, "\r\n", 2);

        if (mp_verbose > 3)
            printf(">> %s\n", buf);

        return ret;
    }

    return -1;
}

int mp_serial_readline(int fd, char *buf, int count) {
    fd_set fds;
    int ret;
    char *ptr = NULL;
    int num = 0;

    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    ret = select(fd+1, &fds, NULL, NULL, NULL);

    if(ret == -1)
        unknown("select failed");

    if(FD_ISSET(fd, &fds)) {
        ptr = buf;
        for (ptr = buf; *(ptr-1) != '\n'; ptr++) {
            num += read(fd, ptr, 1);
            if (num == count)
                break;
        }
    }

    printf("<< {%d} '%s'\n", num, buf);

    do {
        *ptr = '\0';
        ptr--;
        num--;
    } while (*ptr == '\r' || *ptr == '\n');

    if (mp_verbose > 3)
        printf("<< [%d] '%s'\n", num, buf);
    return num;
}

char *mp_serial_reply(int fd, char *command) {
    char *buf;

    buf = mp_malloc(64);

    mp_serial_write(fd, command);

    do {
        mp_serial_readline(fd, buf, 63);
    } while (strcmp(buf, command) != 0);
    mp_serial_readline(fd, buf, 63);

    return buf;
}

int mp_serial_close(int fd) {
    tcsetattr(fd,TCSANOW,&save_tio);
    return close(fd);
}

void getopt_serial(int c) {
    switch (c) {
        case 'S':
            mp_serial_device = optarg;
            break;
        case 's' :
            {
                speed_map *m;
                for(m = speeds; m->name; m++) {
                    if(strcmp(m->name, optarg) == 0) {
                        mp_serial_speed = m->flag;
                        break;
                    }
                }
                if (m->name == NULL)
                    usage("Unknow speed value '%s'", optarg);
                break;
            }
    }
}

void print_help_serial(void) {
    printf(" -S, --serial=DEV\n");
    printf("      Servial device to use.\n");
    printf(" -s, --speed=SPEED\n");
    printf("      Connection speed. (Default to 9600)\n");
}

/* vim: set ts=4 sw=4 et syn=c : */
