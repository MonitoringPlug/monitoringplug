/***
 * Monitoring Plugin - mp_serial.h
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

#ifndef MP_SERIAL_H_
#define MP_SERIAL_H_

#include <termios.h>

/* The global serial vars. */
extern char *mp_serial_device;  /**< Hold the Serial device name. */
extern int   mp_serial_speed;   /**< Hold the Serial device speed. */

/** Serial specific short option string. */
#define MP_SERIAL_OPTSTR "S:s:"
/** Serial specific longopt struct. */
#define MP_SERIAL_LONGOPTS {"serial", required_argument, NULL, (int)'S'}, \
                      {"speed", required_argument, NULL, (int)'s'}

/**
 * Serial Speed infos
 */
typedef struct {
    char *name; /**< Human speed representation */
    int flag;   /**< Termios representation. */
} speed_map;

/**
 * Open serial port.
 * \param[in] device Filename of the device.
 * \param[in] speed Termios speed flag.
 * \return File descriptor.
 */
int mp_serial_open(const char *device, int speed);

int mp_serial_write(int fd, const char *buf);
int mp_serial_readline(int fd, char *buf, int count);
char *mp_serial_reply(int fd, char *command);

/**
 * Close serial port
 * \param[in] fd File descriptor of the serial port to close.
 * \return Return 0 on success and -1 on error.
 */
int mp_serial_close(int fd);

/**
 * Handle Serial related command line options.
 * \param[in] c Command line option to handle.
 */
void getopt_serial(int c);

/**
 * Print the help for the Serial related command line options.
 */
void print_help_serial(void);

#endif /* MP_SERIAL_H_ */

/* vim: set ts=4 sw=4 et syn=c : */
