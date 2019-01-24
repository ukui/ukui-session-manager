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
 */

#ifndef __UKDM_SIGNAL_HANDLER_H
#define __UKDM_SIGNAL_HANDLER_H

#include <glib-object.h>

G_BEGIN_DECLS

#define UKDM_TYPE_SIGNAL_HANDLER (ukdm_signal_handler_get_type())
G_DECLARE_FINAL_TYPE (UKdmSignalHandler, ukdm_signal_handler, UKDM, SIGNAL_HANDLER, GObject)

typedef gboolean (*UKdmSignalHandlerFunc)(int signal, gpointer data);

typedef void (*UKdmShutdownHandlerFunc)(gpointer data);

typedef struct _UKdmSignalHandler _UKdmSignalHandler;

UKdmSignalHandler* ukdm_signal_handler_new(void);
void ukdm_signal_handler_set_fatal_func(UKdmSignalHandler* handler, UKdmShutdownHandlerFunc func, gpointer user_data);

void ukdm_signal_handler_add_fatal(UKdmSignalHandler* handler);
guint ukdm_signal_handler_add(UKdmSignalHandler* handler, int signal_number, UKdmSignalHandlerFunc callback, gpointer data);
void ukdm_signal_handler_remove(UKdmSignalHandler* handler, guint id);
void ukdm_signal_handler_remove_func(UKdmSignalHandler* handler, guint signal_number, UKdmSignalHandlerFunc callback, gpointer data);

G_END_DECLS

#endif /* __UKDM_SIGNAL_HANDLER_H */
