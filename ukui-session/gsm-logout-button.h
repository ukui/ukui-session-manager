/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-
 *
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
 *	handsome_feng <jianfengli@ubuntukylin.com>
 */

#ifndef __GSM_LOGOUT_BUTTON_H__
#define __GSM_LOGOUT_BUTTON_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef enum {
        GSM_BUTTON_LOGOUT_TYPE_LOGOUT,
        GSM_BUTTON_LOGOUT_TYPE_USER,
        GSM_BUTTON_LOGOUT_TYPE_SHUTDOWN,
        GSM_BUTTON_LOGOUT_TYPE_REBOOT,
        GSM_BUTTON_LOGOUT_TYPE_HIBERNATE,
        GSM_BUTTON_LOGOUT_TYPE_SLEEP,
} GsmLogoutButtonType;

#define GSM_TYPE_LOGOUT_BUTTON        (gsm_logout_button_get_type ())
G_DECLARE_DERIVABLE_TYPE (GsmLogoutButton, gsm_logout_button, GSM, LOGOUT_BUTTON, GtkEventBox)

struct _GsmLogoutButtonClass
{
        GtkEventBoxClass  parent_class;

        void (* clicked) (GsmLogoutButton *button);
};

GType        gsm_logout_button_get_type   (void) G_GNUC_CONST;

GsmLogoutButton *gsm_logout_button_new (unsigned int   button_type,
                                        const char    *label,
                                        const char    *normal_img,
                                        const char    *prelight_img);

GsmLogoutButtonType gsm_logout_button_get_btype (GsmLogoutButton *button);

G_END_DECLS

#endif /* __GSM_LOGOUT_BUTTON_H__ */
