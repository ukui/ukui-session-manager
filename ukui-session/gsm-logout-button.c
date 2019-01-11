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

#include <gtk/gtk.h>

#include "gsm-logout-button.h"

#define GSM_LOGOUT_BUTTON_GET_PRIVATE(o)                                \
        (G_TYPE_INSTANCE_GET_PRIVATE ((o), GSM_TYPE_LOGOUT_BUTTON, GsmLogoutButtonPrivate))

struct _GsmLogoutButtonPrivate
{
        GsmLogoutButtonType  type;

        GtkWidget   *image;

        GtkWidget   *label;

        char *label_text;

        char *normal_img;

        char *prelight_img;
};

enum {
        CLICKED,
        LAST_SIGNAL
};

enum {
        PROP_0,
        PROP_LABEL_TEXT,
        PROP_NORMAL_IMG,
        PROP_PRELIGHT_IMG
};

static guint button_signals[LAST_SIGNAL] = { 0 };

static void gsm_logout_button_class_init (GsmLogoutButtonClass *klass);
static void gsm_logout_button_init       (GsmLogoutButton      *button);
static void gsm_logout_button_dispose    (GObject              *object);
static void gsm_logout_button_finalize   (GObject              *object);

G_DEFINE_TYPE (GsmLogoutButton, gsm_logout_button, GTK_TYPE_EVENT_BOX);

 static gboolean
 gsm_logout_button_enter (GsmLogoutButton *logout_button, gpointer data)
 {
        gtk_widget_grab_focus(GTK_WIDGET(logout_button));

        return TRUE;
 }

static gboolean
gsm_logout_button_focus_in (GsmLogoutButton *logout_button, gpointer data)
{
        gtk_image_set_from_file(GTK_IMAGE(logout_button->priv->image), logout_button->priv->prelight_img);
        const char *format = "<span color=\"white\" alpha=\"65535\">\%s</span>";
        char *markup = g_markup_printf_escaped (format, logout_button->priv->label_text);
        gtk_label_set_markup (GTK_LABEL (logout_button->priv->label), markup);
        g_free (markup);

        return TRUE;
}

static gboolean
gsm_logout_button_focus_out (GsmLogoutButton *logout_button, gpointer data)
{
        gtk_image_set_from_file(GTK_IMAGE(logout_button->priv->image), logout_button->priv->normal_img);
        const char *format = "<span alpha=\"1\">\%s</span>";
        char *markup = g_markup_printf_escaped (format, logout_button->priv->label_text);
        gtk_label_set_markup (GTK_LABEL (logout_button->priv->label), markup);
        g_free (markup);
        return TRUE;
}

static void
gsm_logout_button_set_label_text (GsmLogoutButton *button,
                                  const char *label_text)
{
        g_return_if_fail (GSM_IS_LOGOUT_BUTTON (button));

        g_free (button->priv->label_text);

        button->priv->label_text = g_strdup (label_text);
        g_object_notify (G_OBJECT (button), "label_text");
}

static void
gsm_logout_button_set_normal_img (GsmLogoutButton *button,
                                  const char      *normal_img)
{
        g_return_if_fail (GSM_IS_LOGOUT_BUTTON (button));

        g_free (button->priv->normal_img);

        button->priv->normal_img = g_strdup (normal_img);
        g_object_notify (G_OBJECT (button), "normal_img");
}

static void
gsm_logout_button_set_prelight_img (GsmLogoutButton *button,
                                    const char      *prelight_img)
{
        g_return_if_fail (GSM_IS_LOGOUT_BUTTON (button));

        g_free (button->priv->prelight_img);

        button->priv->prelight_img = g_strdup (prelight_img);
        g_object_notify (G_OBJECT (button), "prelight_img");
}

static void
gsm_logout_button_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
        GsmLogoutButton *button = GSM_LOGOUT_BUTTON (object);

        switch (prop_id) {
        case PROP_LABEL_TEXT:
                gsm_logout_button_set_label_text (button, g_value_get_string (value));
                break;
        case PROP_NORMAL_IMG:
                gsm_logout_button_set_normal_img (button, g_value_get_string (value));
                break;
        case PROP_PRELIGHT_IMG:
                gsm_logout_button_set_prelight_img (button, g_value_get_string (value));
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                break;
        }
}

GsmLogoutButtonType gsm_logout_button_get_btype (GsmLogoutButton *button)
{
    return button->priv->type;
}

static void
gsm_logout_button_get_property (GObject     *object,
                                guint        prop_id,
                                GValue      *value,
                                GParamSpec  *pspec)
{
        GsmLogoutButton *button = GSM_LOGOUT_BUTTON (object);

        switch (prop_id) {
        case PROP_LABEL_TEXT:
                g_value_set_string (value, button->priv->label_text);
                break;
        case PROP_NORMAL_IMG:
                g_value_set_string (value, button->priv->normal_img);
                break;
        case PROP_PRELIGHT_IMG:
                g_value_set_string (value, button->priv->prelight_img);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                break;
        }
}

static GObject *
gsm_logout_button_constructor (GType type,
                               guint    n_construct_app,
                               GObjectConstructParam *construct_app)
{
        GsmLogoutButton *logout_button;

        logout_button = GSM_LOGOUT_BUTTON (G_OBJECT_CLASS (gsm_logout_button_parent_class)->constructor (type,
                                                                                                         n_construct_app,
                                                                                                         construct_app));
        GtkWidget *box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);

        logout_button->priv->image = gtk_image_new_from_file (logout_button->priv->normal_img);
        gtk_container_add (GTK_CONTAINER (box), logout_button->priv->image);

        logout_button->priv->label = gtk_label_new (logout_button->priv->label_text);
        const char *format = "<span alpha=\"1\">\%s</span>";
        char *markup = g_markup_printf_escaped (format, logout_button->priv->label_text);
        gtk_label_set_markup (GTK_LABEL (logout_button->priv->label), markup);
        g_free (markup);
        gtk_container_add (GTK_CONTAINER (box), logout_button->priv->label);

        gtk_container_add (GTK_CONTAINER (logout_button), box);

        g_signal_connect (logout_button, "enter-notify-event",
                          G_CALLBACK (gsm_logout_button_enter), NULL);

        g_signal_connect (logout_button, "focus-in-event",
                          G_CALLBACK(gsm_logout_button_focus_in), NULL);

        g_signal_connect (logout_button, "focus-out-event",
                          G_CALLBACK(gsm_logout_button_focus_out), NULL);

        return G_OBJECT (logout_button);
}
static void
gsm_logout_button_class_init (GsmLogoutButtonClass *klass)
{
        GObjectClass *gobject_class;

        gobject_class = G_OBJECT_CLASS (klass);

        gobject_class->set_property = gsm_logout_button_set_property;
        gobject_class->get_property = gsm_logout_button_get_property;
        gobject_class->constructor = gsm_logout_button_constructor;
        gobject_class->dispose = gsm_logout_button_dispose;
        gobject_class->finalize = gsm_logout_button_finalize;

        klass->clicked = NULL;

        g_object_class_install_property (gobject_class,
                                         PROP_LABEL_TEXT,
                                         g_param_spec_string ("label_text",
                                                              "label_text",
                                                              "label_text",
                                                              NULL,
                                                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

        g_object_class_install_property (gobject_class,
                                         PROP_NORMAL_IMG,
                                         g_param_spec_string ("normal_img",
                                                              "normal_img",
                                                              "normal_img",
                                                              NULL,
                                                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
        g_object_class_install_property (gobject_class,
                                         PROP_PRELIGHT_IMG,
                                         g_param_spec_string ("prelight_img",
                                                              "prelight_img",
                                                              "prelight_img",
                                                              NULL,
                                                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

        button_signals[CLICKED] =
                g_signal_new ("clicked",
                G_OBJECT_CLASS_TYPE (gobject_class),
                G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                G_STRUCT_OFFSET (GsmLogoutButtonClass, clicked),
                NULL, NULL,
                NULL,
                G_TYPE_NONE, 0);

        g_type_class_add_private (klass, sizeof (GsmLogoutButtonPrivate));
}

static void
gsm_logout_button_dispose (GObject *object)
{
        GsmLogoutButton *button;

        g_return_if_fail (object != NULL);
        g_return_if_fail (GSM_IS_LOGOUT_BUTTON (object));

        button = GSM_LOGOUT_BUTTON (object);

        g_debug ("GsmLogoutButton: dispose called");

        if (button->priv->label_text != NULL) {
                g_free (button->priv->label_text);
                button->priv->label_text = NULL;
        }

        if (button->priv->normal_img != NULL) {
                g_free (button->priv->normal_img);
                button->priv->normal_img = NULL;
        }

        if (button->priv->prelight_img != NULL) {
                g_free (button->priv->prelight_img);
                button->priv->prelight_img = NULL;
        }

        G_OBJECT_CLASS (gsm_logout_button_parent_class)->dispose (object);
}

static void
gsm_logout_button_finalize (GObject *object)
{
        GsmLogoutButton *button;

        g_return_if_fail (object != NULL);
        g_return_if_fail (GSM_IS_LOGOUT_BUTTON (object));

        button = GSM_LOGOUT_BUTTON (object);

        g_return_if_fail (button->priv != NULL);

        g_debug ("GsmLogoutButton: finalizing");

        G_OBJECT_CLASS (gsm_logout_button_parent_class)->finalize (object);
}

static void
gsm_logout_button_init (GsmLogoutButton *logout_button)
{
        logout_button->priv = GSM_LOGOUT_BUTTON_GET_PRIVATE (logout_button);
}

GsmLogoutButton *
gsm_logout_button_new (GsmLogoutButtonType   button_type,
                       const char    *label_text,
                       const char    *normal_img,
                       const char    *prelight_img)
{
        GsmLogoutButton *logout_button;

        logout_button = g_object_new (GSM_TYPE_LOGOUT_BUTTON,
                                      "label_text", label_text,
                                      "normal_img", normal_img,
                                      "prelight_img", prelight_img,
                                      NULL);

        logout_button->priv->type = button_type;

        return logout_button;
}
