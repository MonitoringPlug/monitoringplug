/***
 * Monitoring Plugin - mp_check.h
 **
 *
 * Copyright (C) 2011 Marius Rieder <marius.rieder@durchmesser.ch>
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id$
 */

#ifndef _MP_CHECK_H_
#define _MP_CHECK_H_

/**
 * Check if string is a integer.
 * \param[in] number string to check
 * \return return 1 if string is a integer 0 otherwise.
 */
int is_integer(const char *number);

/**
 * Check if string is a hostname/domainname.
 * \param[in] address string to check
 * \return return 1 if string is a hostname 0 otherwise.
 */
int is_hostname(const char *address);

/**
 * Check if string is a ip addr.
 * \param[in] address string to check
 * \return return 1 if string is a ip 0 otherwise.
 */
int is_hostaddr(const char *address);

/**
 * Check if string is a url.
 * \param[in] url string to check
 * \return return 1 if string is a url 0 otherwise.
 **/
int is_url(const char *url);

/**
 * Check if url is of given schema
 * \param[in] url string to check
 * \param[in] schema schema to check
 * \return return 1 if url is of given schema 0 otherwise.
 **/
int is_url_scheme(const char *url, const char *schema);

#endif /* _MP_CHECK_H_ */

