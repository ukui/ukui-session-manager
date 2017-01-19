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

#ifndef __UIDM_SIGNAL_HANDLER_H
#define __UIDM_SIGNAL_HANDLER_H

#include <glib-object.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UIDM_TYPE_SIGNAL_HANDLER \
	(uidm_signal_handler_get_type())

#define UIDM_SIGNAL_HANDLER(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), UIDM_TYPE_SIGNAL_HANDLER, UIdmSignalHandler))

#define UIDM_SIGNAL_HANDLER_CLASS(k) \
	(G_TYPE_CHECK_CLASS_CAST((k), UIDM_TYPE_SIGNAL_HANDLER, UIdmSignalHandlerClass))

#define UIDM_IS_SIGNAL_HANDLER(o) \
	(G_TYPE_CHECK_INSTANCE_TYPE((o), UIDM_TYPE_SIGNAL_HANDLER))

#define UIDM_IS_SIGNAL_HANDLER_CLASS(k) \
	(G_TYPE_CHECK_CLASS_TYPE((k), UIDM_TYPE_SIGNAL_HANDLER))

#define UIDM_SIGNAL_HANDLER_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), UIDM_TYPE_SIGNAL_HANDLER, UIdmSignalHandlerClass))


typedef gboolean (*UIdmSignalHandlerFunc)(int signal, gpointer data);

typedef void (*UIdmShutdownHandlerFunc)(gpointer data);

typedef struct UIdmSignalHandlerPrivate UIdmSignalHandlerPrivate;

typedef struct {
	GObject parent;
	UIdmSignalHandlerPrivate* priv;
} UIdmSignalHandler;

typedef struct {
	GObjectClass parent_class;
} UIdmSignalHandlerClass;

GType uidm_signal_handler_get_type(void);

UIdmSignalHandler* uidm_signal_handler_new(void);
void uidm_signal_handler_set_fatal_func(UIdmSignalHandler* handler, UIdmShutdownHandlerFunc func, gpointer user_data);

void uidm_signal_handler_add_fatal(UIdmSignalHandler* handler);
guint uidm_signal_handler_add(UIdmSignalHandler* handler, int signal_number, UIdmSignalHandlerFunc callback, gpointer data);
void uidm_signal_handler_remove(UIdmSignalHandler* handler, guint id);
void uidm_signal_handler_remove_func(UIdmSignalHandler* handler, guint signal_number, UIdmSignalHandlerFunc callback, gpointer data);

#ifdef __cplusplus
}
#endif

#endif /* __UIDM_SIGNAL_HANDLER_H */
