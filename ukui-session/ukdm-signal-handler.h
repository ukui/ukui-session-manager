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

#ifdef __cplusplus
extern "C" {
#endif

#define UKDM_TYPE_SIGNAL_HANDLER \
	(ukdm_signal_handler_get_type())

#define UKDM_SIGNAL_HANDLER(o) \
	(G_TYPE_CHECK_INSTANCE_CAST((o), UKDM_TYPE_SIGNAL_HANDLER, UKdmSignalHandler))

#define UKDM_SIGNAL_HANDLER_CLASS(k) \
	(G_TYPE_CHECK_CLASS_CAST((k), UKDM_TYPE_SIGNAL_HANDLER, UKdmSignalHandlerClass))

#define UKDM_IS_SIGNAL_HANDLER(o) \
	(G_TYPE_CHECK_INSTANCE_TYPE((o), UKDM_TYPE_SIGNAL_HANDLER))

#define UKDM_IS_SIGNAL_HANDLER_CLASS(k) \
	(G_TYPE_CHECK_CLASS_TYPE((k), UKDM_TYPE_SIGNAL_HANDLER))

#define UKDM_SIGNAL_HANDLER_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS((o), UKDM_TYPE_SIGNAL_HANDLER, UKdmSignalHandlerClass))


typedef gboolean (*UKdmSignalHandlerFunc)(int signal, gpointer data);

typedef void (*UKdmShutdownHandlerFunc)(gpointer data);

typedef struct UKdmSignalHandlerPrivate UKdmSignalHandlerPrivate;

typedef struct {
	GObject parent;
	UKdmSignalHandlerPrivate* priv;
} UKdmSignalHandler;

typedef struct {
	GObjectClass parent_class;
} UKdmSignalHandlerClass;

GType ukdm_signal_handler_get_type(void);

UKdmSignalHandler* ukdm_signal_handler_new(void);
void ukdm_signal_handler_set_fatal_func(UKdmSignalHandler* handler, UKdmShutdownHandlerFunc func, gpointer user_data);

void ukdm_signal_handler_add_fatal(UKdmSignalHandler* handler);
guint ukdm_signal_handler_add(UKdmSignalHandler* handler, int signal_number, UKdmSignalHandlerFunc callback, gpointer data);
void ukdm_signal_handler_remove(UKdmSignalHandler* handler, guint id);
void ukdm_signal_handler_remove_func(UKdmSignalHandler* handler, guint signal_number, UKdmSignalHandlerFunc callback, gpointer data);

#ifdef __cplusplus
}
#endif

#endif /* __UKDM_SIGNAL_HANDLER_H */
