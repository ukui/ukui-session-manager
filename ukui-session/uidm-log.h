/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2007 William Jon McCann <mccann@jhu.edu>
 * Copyright (C) 2016 Tianjin KYLIN Information Technology Co., Ltd.
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Authors: William Jon McCann <mccann@jhu.edu>
 *
 */

#ifndef __UIDM_LOG_H
#define __UIDM_LOG_H

#include <stdarg.h>
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

void      uidm_log_default_handler (const gchar   *log_domain,
                                   GLogLevelFlags log_level,
                                   const gchar   *message,
                                   gpointer      unused_data);
void      uidm_log_set_debug       (gboolean       debug);
void      uidm_log_toggle_debug    (void);
void      uidm_log_init            (void);
void      uidm_log_shutdown        (void);

/* compatibility */
#define   uidm_fail               g_critical
#define   uidm_error              g_warning
#define   uidm_info               g_message
#define   uidm_debug              g_debug

#define   uidm_assert             g_assert
#define   uidm_assert_not_reached g_assert_not_reached

#ifdef __cplusplus
}
#endif

#endif /* __UIDM_LOG_H */
