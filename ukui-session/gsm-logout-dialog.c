/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-
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
 *	Vincent Untz <vuntz@gnome.org>
 */

#include <config.h>

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "gsm-logout-dialog.h"
#ifdef HAVE_SYSTEMD
#include "gsm-systemd.h"
#endif
#include "gsm-blur.h"
#include "gsm-consolekit.h"
#include "gsm-logout-button.h"
#include "ukdm.h"

#define GSM_LOGOUT_DIALOG_GET_PRIVATE(o)                                \
        (G_TYPE_INSTANCE_GET_PRIVATE ((o), GSM_TYPE_LOGOUT_DIALOG, GsmLogoutDialogPrivate))

#define GSM_ICON_LOGOUT   "system-log-out"
#define GSM_ICON_SHUTDOWN "system-shutdown"

#define SESSION_SCHEMA     "org.ukui.session"
#define KEY_LOGOUT_TIMEOUT "logout-timeout"

#define LOCKDOWN_SCHEMA            "org.mate.lockdown"
#define KEY_USER_SWITCHING_DISABLE "disable-user-switching"

#define BORDER_SIZE 30

typedef enum {
        GSM_DIALOG_LOGOUT_TYPE_LOGOUT,
        GSM_DIALOG_LOGOUT_TYPE_SHUTDOWN
} GsmDialogLogoutType;

enum {
    RESPONSE,
    LAST_SIGNAL
};

static guint dialog_signals[LAST_SIGNAL] = { 0 };

struct _GsmLogoutDialogPrivate
{
        GsmDialogLogoutType  type;
#ifdef HAVE_SYSTEMD
        GsmSystemd          *systemd;
#endif
        GsmConsolekit       *consolekit;

        GtkWidget           *box;

        GtkWidget           *secondary_label;

        GtkWidget           *progressbar;

        GdkPixbuf           *root;

        cairo_pattern_t     *corner_pattern;

        cairo_pattern_t     *left_pattern;

        cairo_pattern_t     *top_pattern;

        int                  timeout;

        unsigned int         timeout_id;

        unsigned int         default_response;
};

static GsmLogoutDialog *current_dialog = NULL;

static void gsm_logout_dialog_set_timeout  (GsmLogoutDialog *logout_dialog);

static void gsm_logout_dialog_draw         (GsmLogoutDialog *logout_dialog,
                                            cairo_t *cr);

static gboolean gsm_logout_dialog_cancle   (GsmLogoutDialog *logout_dialog,
                                            GdkEvent        *event,
                                            gpointer        data);

static void gsm_logout_dialog_destroy      (GsmLogoutDialog *logout_dialog,
                                            gpointer         data);

static void gsm_logout_dialog_show         (GsmLogoutDialog *logout_dialog,
                                            gpointer         data);

G_DEFINE_TYPE (GsmLogoutDialog, gsm_logout_dialog, GTK_TYPE_WINDOW);

static void
gsm_logout_dialog_class_init (GsmLogoutDialogClass *klass)
{
        GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

        dialog_signals[RESPONSE] =
                g_signal_new ("response",
                              G_TYPE_FROM_CLASS (gobject_class),
                              G_SIGNAL_RUN_LAST,
                              G_STRUCT_OFFSET (GsmLogoutDialogClass,
                                               response),
                              NULL, NULL,
                              g_cclosure_marshal_VOID__INT,
                              G_TYPE_NONE, 1,
                              G_TYPE_INT);

        g_type_class_add_private (klass, sizeof (GsmLogoutDialogPrivate));
}

static void
gsm_logout_dialog_init (GsmLogoutDialog *logout_dialog)
{
        logout_dialog->priv = GSM_LOGOUT_DIALOG_GET_PRIVATE (logout_dialog);

        logout_dialog->priv->timeout_id = 0;
        logout_dialog->priv->timeout = 0;
        logout_dialog->priv->default_response = GTK_RESPONSE_CANCEL;
        GdkWindow *root_win = gdk_get_default_root_window ();
        int width = gdk_window_get_width (root_win);
        int height = gdk_window_get_height (root_win);
        logout_dialog->priv->root = gdk_pixbuf_get_from_window (root_win, 0, 0, width, height);

        cairo_surface_t *corner = cairo_image_surface_create_from_png (DATA_DIR "/switcher_corner.png");
        cairo_surface_t *left = cairo_image_surface_create_from_png (DATA_DIR "/switcher_left.png");
        cairo_surface_t *top = cairo_image_surface_create_from_png (DATA_DIR "/switcher_top.png");

        logout_dialog->priv->corner_pattern = cairo_pattern_create_for_surface (corner);
        logout_dialog->priv->left_pattern = cairo_pattern_create_for_surface (left);
        cairo_pattern_set_extend (logout_dialog->priv->left_pattern, CAIRO_EXTEND_REPEAT);
        logout_dialog->priv->top_pattern = cairo_pattern_create_for_surface (top);
        cairo_pattern_set_extend (logout_dialog->priv->top_pattern, CAIRO_EXTEND_REPEAT);

        cairo_surface_destroy (corner);
        cairo_surface_destroy (left);
        cairo_surface_destroy (top);

        gtk_window_set_skip_taskbar_hint (GTK_WINDOW (logout_dialog), TRUE);
        gtk_window_set_keep_above (GTK_WINDOW (logout_dialog), TRUE);
        gtk_window_stick (GTK_WINDOW (logout_dialog));
#ifdef HAVE_SYSTEMD
        if (LOGIND_RUNNING())
            logout_dialog->priv->systemd = gsm_get_systemd ();
        else
#endif
        logout_dialog->priv->consolekit = gsm_get_consolekit ();

        g_signal_connect (logout_dialog,
                          "draw",
                          G_CALLBACK (gsm_logout_dialog_draw),
                          NULL);

        g_signal_connect (logout_dialog,
                          "destroy",
                          G_CALLBACK (gsm_logout_dialog_destroy),
                          NULL);

        g_signal_connect (logout_dialog,
                          "show",
                          G_CALLBACK (gsm_logout_dialog_show),
                          NULL);
}

static void
gsm_logout_dialog_draw (GsmLogoutDialog *logout_dialog,
                        cairo_t *cr)
{
        cairo_set_source_rgba (cr, 0, 0, 0, 0.25);
        cairo_paint (cr);

        int box_width = gtk_widget_get_allocated_width (logout_dialog->priv->box);
        int box_height = gtk_widget_get_allocated_height (logout_dialog->priv->box);
        int win_width = gtk_widget_get_allocated_width (GTK_WIDGET(logout_dialog));
        int win_height = gtk_widget_get_allocated_height (GTK_WIDGET(logout_dialog));

        int x = (win_width - box_width) / 2;
        int y = (win_height - box_height) / 2;

        cairo_surface_t *center_surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, box_width, box_height);
        cairo_t *center_cr = cairo_create (center_surface);
        gdk_cairo_set_source_pixbuf (center_cr, logout_dialog->priv->root, -x, -y);
        cairo_rectangle (center_cr, 0, 0, box_width, box_height);
        cairo_fill (center_cr);
        cairo_destroy (center_cr);

        blur (center_surface, 8);

        cairo_save (cr);
        cairo_translate (cr, x, y);

        cairo_rectangle (cr, 0, 0, box_width, box_height);
        cairo_set_source_surface (cr, center_surface, 0, 0);
        cairo_fill_preserve (cr);
        cairo_set_source_rgba (cr, 0.5, 0.5, 0.5, 0.5);
        cairo_fill (cr);
        cairo_surface_destroy (center_surface);

        cairo_restore(cr);

        x -= BORDER_SIZE;
        y -= BORDER_SIZE;
        box_width += BORDER_SIZE * 2;
        box_height += BORDER_SIZE * 2;

        cairo_save (cr);
        cairo_translate (cr, x, y);

        /* Top left */
        cairo_matrix_t matrix;
        cairo_matrix_init_identity (&matrix);
        cairo_pattern_set_matrix (logout_dialog->priv->corner_pattern, &matrix);
        cairo_set_source (cr, logout_dialog->priv->corner_pattern);
        cairo_rectangle (cr, 0, 0, BORDER_SIZE, BORDER_SIZE);
        cairo_fill (cr);

        /* Top right */
        cairo_matrix_init_identity (&matrix);
        cairo_matrix_translate (&matrix, box_width, 0);
        cairo_matrix_scale (&matrix, -1, 1);
        cairo_pattern_set_matrix (logout_dialog->priv->corner_pattern, &matrix);
        cairo_set_source (cr, logout_dialog->priv->corner_pattern);
        cairo_rectangle (cr, box_width - BORDER_SIZE, 0, BORDER_SIZE, BORDER_SIZE);
        cairo_fill (cr);

        /* close icon */
        cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.9);
        cairo_move_to (cr, box_width - BORDER_SIZE + 7 , 13);
        cairo_line_to (cr, box_width - BORDER_SIZE + 17, 23);
        cairo_move_to (cr, box_width - BORDER_SIZE + 7, 23);
        cairo_line_to (cr, box_width - BORDER_SIZE + 17, 13);
        cairo_stroke (cr);


        /* Bottom left */
        cairo_matrix_init_identity (&matrix);
        cairo_matrix_translate (&matrix, 0, box_height);
        cairo_matrix_scale (&matrix, 1, -1);
        cairo_pattern_set_matrix (logout_dialog->priv->corner_pattern, &matrix);
        cairo_set_source (cr, logout_dialog->priv->corner_pattern);
        cairo_rectangle (cr, 0, box_height - BORDER_SIZE, BORDER_SIZE, BORDER_SIZE);
        cairo_fill (cr);

        /* Bottom right */
        cairo_matrix_init_identity (&matrix);
        cairo_matrix_translate (&matrix, box_width, box_height);
        cairo_matrix_scale (&matrix, -1, -1);
        cairo_pattern_set_matrix (logout_dialog->priv->corner_pattern, &matrix);
        cairo_set_source (cr, logout_dialog->priv->corner_pattern);
        cairo_rectangle (cr, box_width - BORDER_SIZE, box_height - BORDER_SIZE, BORDER_SIZE, BORDER_SIZE);
        cairo_fill (cr);

        /* Left */
        cairo_matrix_init_identity (&matrix);
        cairo_pattern_set_matrix (logout_dialog->priv->left_pattern, &matrix);
        cairo_set_source (cr, logout_dialog->priv->left_pattern);
        cairo_rectangle (cr, 0, BORDER_SIZE, BORDER_SIZE, box_height - BORDER_SIZE * 2);
        cairo_fill (cr);

        /* Right */
        cairo_matrix_init_identity (&matrix);
        cairo_matrix_translate (&matrix, box_width, 0);
        cairo_matrix_scale (&matrix, -1, 1);
        cairo_pattern_set_matrix (logout_dialog->priv->left_pattern, &matrix);
        cairo_set_source (cr, logout_dialog->priv->left_pattern);
        cairo_rectangle (cr, box_width - BORDER_SIZE, BORDER_SIZE, BORDER_SIZE, box_height - BORDER_SIZE * 2);
        cairo_fill (cr);

        /* Top */
        cairo_matrix_init_identity (&matrix);
        cairo_pattern_set_matrix (logout_dialog->priv->top_pattern, &matrix);
        cairo_set_source (cr, logout_dialog->priv->top_pattern);
        cairo_rectangle (cr, BORDER_SIZE, 0, box_width - BORDER_SIZE * 2, BORDER_SIZE);
        cairo_fill (cr);

        /* Bottom */
        cairo_matrix_init_identity (&matrix);
        cairo_matrix_translate (&matrix, 0, box_height);
        cairo_matrix_scale (&matrix, 1, -1);
        cairo_pattern_set_matrix (logout_dialog->priv->top_pattern, &matrix);
        cairo_set_source (cr, logout_dialog->priv->top_pattern);
        cairo_rectangle (cr, BORDER_SIZE, box_height - BORDER_SIZE, box_width - BORDER_SIZE * 2, BORDER_SIZE);
        cairo_fill (cr);

        cairo_restore (cr);
}

static gboolean
gsm_logout_dialog_cancle(GsmLogoutDialog *logout_dialog,
                         GdkEvent        *event,
                         gpointer        data)
{
        gtk_widget_destroy (GTK_WIDGET(data));

        return TRUE;
}

static void
gsm_logout_dialog_destroy (GsmLogoutDialog *logout_dialog,
                           gpointer         data)
{
        if (logout_dialog->priv->timeout_id != 0) {
                g_source_remove (logout_dialog->priv->timeout_id);
                logout_dialog->priv->timeout_id = 0;
        }
#ifdef HAVE_SYSTEMD
        if (logout_dialog->priv->systemd) {
                g_object_unref (logout_dialog->priv->systemd);
                logout_dialog->priv->systemd = NULL;
        }
#endif

        if (logout_dialog->priv->consolekit) {
                g_object_unref (logout_dialog->priv->consolekit);
                logout_dialog->priv->consolekit = NULL;
        }

        if (logout_dialog->priv->root) {
                g_object_unref (logout_dialog->priv->root);
                logout_dialog->priv->root = NULL;
        }

        if (logout_dialog->priv->corner_pattern) {
                cairo_pattern_destroy (logout_dialog->priv->corner_pattern);
                logout_dialog->priv->corner_pattern = NULL;
        }

        if (logout_dialog->priv->left_pattern) {
                cairo_pattern_destroy (logout_dialog->priv->left_pattern);
                logout_dialog->priv->left_pattern = NULL;
        }

        if (logout_dialog->priv->top_pattern) {
                cairo_pattern_destroy (logout_dialog->priv->top_pattern);
                logout_dialog->priv->top_pattern = NULL;
        }

        current_dialog = NULL;
}

static gboolean
gsm_logout_supports_system_suspend (GsmLogoutDialog *logout_dialog)
{
        gboolean ret;
        ret = FALSE;
#ifdef HAVE_SYSTEMD
        if (LOGIND_RUNNING())
            ret = gsm_systemd_can_suspend (logout_dialog->priv->systemd);
        else
#endif
        ret = gsm_consolekit_can_suspend (logout_dialog->priv->consolekit);
        return ret;
}

static gboolean
gsm_logout_supports_system_hibernate (GsmLogoutDialog *logout_dialog)
{
        gboolean ret;
        ret = FALSE;
#ifdef HAVE_SYSTEMD
        if (LOGIND_RUNNING())
            ret = gsm_systemd_can_hibernate (logout_dialog->priv->systemd);
        else
#endif
        ret = gsm_consolekit_can_hibernate (logout_dialog->priv->consolekit);
        return ret;
}

static gboolean
gsm_logout_supports_switch_user (GsmLogoutDialog *logout_dialog)
{
        GSettings *settings;
        gboolean   ret = FALSE;
        gboolean   locked;

        settings = g_settings_new (LOCKDOWN_SCHEMA);

        locked = g_settings_get_boolean (settings, KEY_USER_SWITCHING_DISABLE);
        g_object_unref (settings);

        if (!locked) {
#ifdef HAVE_SYSTEMD
            if (LOGIND_RUNNING())
                ret = gsm_systemd_can_switch_user (logout_dialog->priv->systemd);
            else
#endif
            ret = gsm_consolekit_can_switch_user (logout_dialog->priv->consolekit);
        }

        return ret;
}

static gboolean
gsm_logout_supports_reboot (GsmLogoutDialog *logout_dialog)
{
        gboolean ret;

#ifdef HAVE_SYSTEMD
        if (LOGIND_RUNNING())
            ret = gsm_systemd_can_restart (logout_dialog->priv->systemd);
        else
#endif
        ret = gsm_consolekit_can_restart (logout_dialog->priv->consolekit);
        if (!ret) {
                ret = ukdm_supports_logout_action (UKDM_LOGOUT_ACTION_REBOOT);
        }

        return ret;
}

static gboolean
gsm_logout_supports_shutdown (GsmLogoutDialog *logout_dialog)
{
        gboolean ret;

#ifdef HAVE_SYSTEMD
        if (LOGIND_RUNNING())
            ret = gsm_systemd_can_stop (logout_dialog->priv->systemd);
        else
#endif
        ret = gsm_consolekit_can_stop (logout_dialog->priv->consolekit);

        if (!ret) {
                ret = ukdm_supports_logout_action (UKDM_LOGOUT_ACTION_SHUTDOWN);
        }

        return ret;
}

static void
gsm_logout_dialog_show (GsmLogoutDialog *logout_dialog, gpointer user_data)
{
        gsm_logout_dialog_set_timeout (logout_dialog);
}

static gboolean
gsm_logout_dialog_timeout (gpointer data)
{
        GsmLogoutDialog *logout_dialog;
        char            *seconds_warning;
        char            *secondary_text;
        static char     *session_type = NULL;
        static gboolean  is_not_login;

        logout_dialog = (GsmLogoutDialog *) data;

        if (!logout_dialog->priv->timeout) {
                g_signal_emit(logout_dialog, dialog_signals[RESPONSE], 0,
                              logout_dialog->priv->default_response);

                return FALSE;
        }

        switch (logout_dialog->priv->type) {
        case GSM_DIALOG_LOGOUT_TYPE_LOGOUT:
                seconds_warning = ngettext ("You will be automatically logged "
                                            "out in %d second",
                                            "You will be automatically logged "
                                            "out in %d seconds",
                                            logout_dialog->priv->timeout);
                break;

        case GSM_DIALOG_LOGOUT_TYPE_SHUTDOWN:
                seconds_warning = ngettext ("This system will be automatically "
                                            "shut down in %d second",
                                            "This system will be automatically "
                                            "shut down in %d seconds",
                                            logout_dialog->priv->timeout);
                break;

        default:
                g_assert_not_reached ();
        }
        seconds_warning = g_strdup_printf (seconds_warning, logout_dialog->priv->timeout);

        if (session_type == NULL) {
#ifdef HAVE_SYSTEMD
                if (LOGIND_RUNNING()) {
                    GsmSystemd *systemd;
                    systemd = gsm_get_systemd ();
                    session_type = gsm_systemd_get_current_session_type (systemd);
                    g_object_unref (systemd);
                    is_not_login = (g_strcmp0 (session_type, GSM_SYSTEMD_SESSION_TYPE_LOGIN_WINDOW) != 0);
                }
                else {
#endif
                GsmConsolekit *consolekit;
                consolekit = gsm_get_consolekit ();
                session_type = gsm_consolekit_get_current_session_type (consolekit);
                g_object_unref (consolekit);
                is_not_login = (g_strcmp0 (session_type, GSM_CONSOLEKIT_SESSION_TYPE_LOGIN_WINDOW) != 0);
#ifdef HAVE_SYSTEMD
                }
#endif
        }

        if (is_not_login) {
                char *name;

                name = g_locale_to_utf8 (g_get_real_name (), -1, NULL, NULL, NULL);

                if (!name || name[0] == '\0' || strcmp (name, "Unknown") == 0) {
                        name = g_locale_to_utf8 (g_get_user_name (), -1 , NULL, NULL, NULL);
                }

                if (!name) {
                        name = g_strdup (g_get_user_name ());
                }

                secondary_text = g_strdup_printf (_("You are currently logged in as \"%s\"."), name);

                g_free (name);
        } else {
                secondary_text = g_strdup (seconds_warning);
        }

        gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (logout_dialog->priv->progressbar),
                                       logout_dialog->priv->timeout / 60.0);
        gtk_progress_bar_set_text (GTK_PROGRESS_BAR (logout_dialog->priv->progressbar),
                                   seconds_warning);

        const char *format = "<span color=\"white\" alpha=\"65535\">\%s</span>";
        char *markup = g_markup_printf_escaped (format, secondary_text);
        gtk_label_set_markup (GTK_LABEL (logout_dialog->priv->secondary_label), markup);
        g_free (markup);

        logout_dialog->priv->timeout--;

        g_free (secondary_text);
        g_free (seconds_warning);

        return TRUE;
}

static void
gsm_logout_dialog_set_timeout (GsmLogoutDialog *logout_dialog)
{
        GSettings *settings;

        settings = g_settings_new (SESSION_SCHEMA);

        logout_dialog->priv->timeout = g_settings_get_int (settings, KEY_LOGOUT_TIMEOUT);

        if (logout_dialog->priv->timeout > 0) {
                /* Sets the secondary text */
                gsm_logout_dialog_timeout (logout_dialog);

                if (logout_dialog->priv->timeout_id != 0) {
                        g_source_remove (logout_dialog->priv->timeout_id);
                }

                logout_dialog->priv->timeout_id = g_timeout_add (1000,
                                                                 gsm_logout_dialog_timeout,
                                                                 logout_dialog);
        }
        else {
                gtk_widget_hide (logout_dialog->priv->progressbar);
        }

        g_object_unref (settings);
}

static gboolean
gsm_logout_button_press (GtkWidget *widget, GdkEvent *event, gpointer logout_dialog)
{
        GsmLogoutButtonType type = gsm_logout_button_get_btype(GSM_LOGOUT_BUTTON(widget));

        guint signal_id;
        switch (type) {
        case GSM_BUTTON_LOGOUT_TYPE_USER:
                signal_id = GSM_LOGOUT_RESPONSE_SWITCH_USER;
                break;
        case GSM_LOGOUT_RESPONSE_HIBERNATE:
                signal_id = GSM_LOGOUT_RESPONSE_HIBERNATE;
                break;
        case GSM_LOGOUT_RESPONSE_SLEEP:
                signal_id = GSM_LOGOUT_RESPONSE_SLEEP;
                break;
        case GSM_LOGOUT_RESPONSE_SHUTDOWN:
                signal_id = GSM_LOGOUT_RESPONSE_SHUTDOWN;
                break;
        case GSM_LOGOUT_RESPONSE_REBOOT:
                signal_id = GSM_LOGOUT_RESPONSE_REBOOT;
                break;
        case GSM_LOGOUT_RESPONSE_LOGOUT:
                signal_id = GSM_LOGOUT_RESPONSE_LOGOUT;
                break;
        default:
                g_assert_not_reached ();
                break;
        }

        g_signal_emit(logout_dialog, dialog_signals[RESPONSE], 0, signal_id);

        return TRUE;
}

static GtkWidget *
gsm_get_dialog (GsmDialogLogoutType type,
                GdkScreen          *screen,
                guint32             activate_time)
{
        GsmLogoutDialog *logout_dialog;
        GtkWidget       *event_box;
        GtkWidget       *buttons_box;
        GtkWidget       *primary_label;
        const char      *primary_text;

        if (current_dialog != NULL) {
                gtk_widget_destroy (GTK_WIDGET (current_dialog));
        }

        logout_dialog = g_object_new (GSM_TYPE_LOGOUT_DIALOG, NULL);

        current_dialog = logout_dialog;

        gtk_window_fullscreen (GTK_WINDOW (logout_dialog));
        gtk_widget_set_visual (GTK_WIDGET (logout_dialog), gdk_screen_get_rgba_visual(screen));
        gtk_widget_set_app_paintable(GTK_WIDGET(logout_dialog), TRUE);

        logout_dialog->priv->type = type;

        primary_text = NULL;

        event_box = gtk_event_box_new();
        gtk_container_add (GTK_CONTAINER(logout_dialog), event_box);
        g_signal_connect (event_box,
                          "button-press-event",
                          G_CALLBACK (gsm_logout_dialog_cancle),
                          logout_dialog);

        logout_dialog->priv->box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
        gtk_widget_set_valign (logout_dialog->priv->box, GTK_ALIGN_CENTER);
        gtk_widget_set_halign (logout_dialog->priv->box, GTK_ALIGN_CENTER);

        buttons_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 20);
        primary_label = gtk_label_new ("");
        const char *format = "<span color=\"white\" alpha=\"65535\">\%s</span>";
        char *markup;

        switch (type) {
        case GSM_DIALOG_LOGOUT_TYPE_LOGOUT:
                primary_text = _("Log out of this system now?");
                markup = g_markup_printf_escaped (format, primary_text);
                gtk_label_set_markup (GTK_LABEL (primary_label), markup);

                gtk_box_pack_start (GTK_BOX (logout_dialog->priv->box), primary_label, FALSE, FALSE, 0);

                logout_dialog->priv->default_response = GSM_LOGOUT_RESPONSE_LOGOUT;

                if (gsm_logout_supports_switch_user (logout_dialog)) {
                        GsmLogoutButton *user_button = gsm_logout_button_new (GSM_BUTTON_LOGOUT_TYPE_USER,
                                                                              _("Switch User"),
                                                                              DATA_DIR "/user.png",
                                                                              DATA_DIR "/user_prelight.png");
                        g_signal_connect (G_OBJECT (user_button),
                                          "button-press-event",
                                          G_CALLBACK (gsm_logout_button_press),
                                          logout_dialog);

                        gtk_box_pack_start(GTK_BOX(buttons_box), GTK_WIDGET(user_button), FALSE, FALSE, 0);
                }

                // Logout button
                GsmLogoutButton *logout_button = gsm_logout_button_new (GSM_BUTTON_LOGOUT_TYPE_LOGOUT,
                                                                        _("Log Out"),
                                                                        DATA_DIR "/logout.png",
                                                                        DATA_DIR "/logout_prelight.png");
                g_signal_connect (G_OBJECT (logout_button),
                                  "button-press-event",
                                  G_CALLBACK (gsm_logout_button_press),
                                  logout_dialog);

                gtk_box_pack_start(GTK_BOX(buttons_box), GTK_WIDGET(logout_button), FALSE, FALSE, 0);

                break;
        case GSM_DIALOG_LOGOUT_TYPE_SHUTDOWN:
                primary_text = _("Shut down this system now?");
                markup = g_markup_printf_escaped (format, primary_text);
                gtk_label_set_markup (GTK_LABEL (primary_label), markup);

                gtk_box_pack_start (GTK_BOX (logout_dialog->priv->box), primary_label, FALSE, FALSE, 0);

                logout_dialog->priv->default_response = GSM_LOGOUT_RESPONSE_SHUTDOWN;

                if (gsm_logout_supports_system_suspend (logout_dialog)) {
                        GsmLogoutButton *sleep_button = gsm_logout_button_new (GSM_BUTTON_LOGOUT_TYPE_SLEEP,
                                                                               _("Suspend"),
                                                                               DATA_DIR "/suspend.png",
                                                                               DATA_DIR "/suspend_prelight.png");
                        g_signal_connect (G_OBJECT (sleep_button),
                                          "button-press-event",
                                          G_CALLBACK (gsm_logout_button_press),
                                          logout_dialog);

                        gtk_box_pack_start(GTK_BOX(buttons_box), GTK_WIDGET(sleep_button), FALSE, FALSE, 0);
                }

                if (gsm_logout_supports_system_hibernate (logout_dialog)) {
                        GsmLogoutButton *hibernate_button = gsm_logout_button_new (GSM_BUTTON_LOGOUT_TYPE_HIBERNATE,
                                                                                   _("Hibernate"),
                                                                                  DATA_DIR "/hibernate.png",
                                                                                  DATA_DIR "/hibernate_prelight.png");
                        g_signal_connect (G_OBJECT (hibernate_button),
                                          "button-press-event",
                                          G_CALLBACK (gsm_logout_button_press),
                                          logout_dialog);

                        gtk_box_pack_start(GTK_BOX(buttons_box), GTK_WIDGET(hibernate_button), FALSE, FALSE, 0);
                }

                if (gsm_logout_supports_reboot (logout_dialog)) {
                        GsmLogoutButton *reboot_button = gsm_logout_button_new (GSM_BUTTON_LOGOUT_TYPE_REBOOT,
                                                                                _("Restart"),
                                                                                DATA_DIR "/reboot.png",
                                                                                DATA_DIR "/reboot_prelight.png");
                        g_signal_connect (G_OBJECT (reboot_button),
                                          "button-press-event",
                                          G_CALLBACK (gsm_logout_button_press),
                                          logout_dialog);

                        gtk_box_pack_start(GTK_BOX(buttons_box), GTK_WIDGET(reboot_button), FALSE, FALSE, 0);
                }

                if (gsm_logout_supports_shutdown (logout_dialog)) {

                        GsmLogoutButton *shutdown_button = gsm_logout_button_new (GSM_BUTTON_LOGOUT_TYPE_SHUTDOWN,
                                                                                  _("Shut Down"),
                                                                                  DATA_DIR "/shutdown.png",
                                                                                  DATA_DIR "/shutdown_prelight.png");
                        g_signal_connect (G_OBJECT (shutdown_button),
                                          "button-press-event",
                                          G_CALLBACK (gsm_logout_button_press),
                                          logout_dialog);

                        gtk_box_pack_start(GTK_BOX(buttons_box), GTK_WIDGET(shutdown_button), FALSE, FALSE, 0);
                }
                break;
        default:
                g_assert_not_reached ();
        }

        g_free (markup);

        gtk_container_add (GTK_CONTAINER (logout_dialog->priv->box), buttons_box);

        logout_dialog->priv->secondary_label = gtk_label_new ("");
        gtk_box_pack_start (GTK_BOX (logout_dialog->priv->box),
                            logout_dialog->priv->secondary_label,
                            FALSE, FALSE, 0);

        logout_dialog->priv->progressbar = gtk_progress_bar_new ();
        gtk_widget_set_halign (logout_dialog->priv->progressbar, GTK_ALIGN_CENTER);
        gtk_progress_bar_set_show_text (GTK_PROGRESS_BAR (logout_dialog->priv->progressbar), TRUE);
        gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (logout_dialog->priv->progressbar), 1.0);
        gtk_box_pack_start (GTK_BOX (logout_dialog->priv->box),
                            logout_dialog->priv->progressbar,
                            FALSE, FALSE, 12);

        GtkStyleContext *context = gtk_widget_get_style_context (logout_dialog->priv->progressbar);
        GtkCssProvider *provider = gtk_css_provider_new ();
        gtk_css_provider_load_from_data (GTK_CSS_PROVIDER (provider),
                                         "text {\n"
                                         "  color: white;\n"
                                          "}\n", -1, NULL);
        gtk_style_context_add_provider (context,
                                        GTK_STYLE_PROVIDER (provider),
                                        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        g_object_unref (provider);


        gtk_widget_show_all (logout_dialog->priv->box);
        gtk_container_add (GTK_CONTAINER ( event_box), logout_dialog->priv->box);

        gtk_window_set_screen (GTK_WINDOW (logout_dialog), screen);

        return GTK_WIDGET (logout_dialog);
}

GtkWidget *
gsm_get_shutdown_dialog (GdkScreen *screen,
                         guint32    activate_time)
{
        return gsm_get_dialog (GSM_DIALOG_LOGOUT_TYPE_SHUTDOWN,
                               screen,
                               activate_time);
}

GtkWidget *
gsm_get_logout_dialog (GdkScreen *screen,
                       guint32    activate_time)
{
        return gsm_get_dialog (GSM_DIALOG_LOGOUT_TYPE_LOGOUT,
                               screen,
                               activate_time);
}
