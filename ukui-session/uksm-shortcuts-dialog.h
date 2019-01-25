/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2006 Vincent Untz
 * Copyright (C) 2008 Red Hat, Inc.
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 * Authors:
 *     incent Untz <vuntz@gnome.org>
 *     handsome_feng <jianfengli@ubuntukylin.com>
 */

#ifndef __UKSM_SHORTCUTS_DIALOG_H__
#define __UKSM_SHORTCUTS_DIALOG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define UKSM_TYPE_SHORTCUTS_DIALOG         (uksm_shortcuts_dialog_get_type ())
G_DECLARE_DERIVABLE_TYPE (UksmShortcutsDialog, uksm_shortcuts_dialog, UKSM, SHORTCUTS_DIALOG, GtkWindow)

struct _UksmShortcutsDialogClass
{
    GtkWindowClass  parent_class;

    /* signals */
    void (*response) (UksmShortcutsDialog *uksm_shortcuts_dialog,
                      gint response_id);
};

GType        uksm_shortcuts_dialog_get_type   (void) G_GNUC_CONST;

GtkWidget   *uksm_get_shortcuts_dialog        (GdkScreen           *screen,
                                               guint32              activate_time);

G_END_DECLS

#endif /* __UKSM_SHORTCUTS_DIALOG_H__ */
