/***
 * Monitoring Plugin - ipmi_sms.h
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

#ifndef _SMS_UTILS_H_
#define _SMS_UTILS_H_

#define SMS_OPTSTR "P:"
#define SMS_LONGOPTS {"pin", required_argument, NULL, (int)'P'}

extern char *mp_sms_pin;

/**
 * Run all AT command and collect it's output.
 * \param[in] fd File descriptor of the modem.
 * \param[in] cmd AT command to run.
 * \param[in] opt AT command options.
 * \param[out] answer Pointer where to put the array with the answer(s).
 * \param[out] answers Number of answers received.
 * \return Return 0 if command is finished by OK, 1 if command is finished by
 *          ERROR or -1 otherwise.
 */
int mobile_at_command(int fd, const char *cmd, const char *opt,
        char ***answer, int *answers);

/**
 * Encode a phone number according to TS GSM 04.11
 * \param[in] number The number to encode.
 * \return Return a new allocated string with the encoded number.
 */
char *sms_encode_number(const char *number);

/**
 * Encode a test as GSM7
 * \param[in] text The text to encode.
 * \return Return a new allocated string with the encoded text including
 *          the length field.
 */
char *sms_encode_text(const char *text);

/**
 * Encode a SMS PDU
 * \param[in] smsc The SMSC to use or NULL.
 * \param[in] number The SMS destination number.
 * \param[in] text The text to send.
 * \return  Return a new allocated string with the HEX representation of the PDU.
 */
char *sms_encode_pdu(const char *smsc, const char *number, const char *text);

#endif /* _SMS_UTILS_H_ */

/* vim: set ts=4 sw=4 et syn=c : */
