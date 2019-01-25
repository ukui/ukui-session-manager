/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2006 Vincent Untz
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
 *     Vincent Untz <vuntz@gnome.org>
 *     handsome_feng <jianfengli@ubuntukylin.com>
 */

#include <config.h>

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "uksm-shortcuts-dialog.h"
#include "gsm-blur.h"
#include "ukdm.h"

#define BORDER_SIZE 30
#define SHORTCUTS_TIMEOUT 10

typedef enum {
        UKSM_LABEL_TYPE_TITLE,
        UKSM_LABEL_TYPE_SUBTITLE,
        UKSM_LABEL_TYPE_KEY,
        UKSM_LABEL_TYPE_FUNCTION
} UksmLabelType;

enum {
    RESPONSE,
    LAST_SIGNAL
};

static guint dialog_signals[LAST_SIGNAL] = { 0 };

typedef struct {
        GtkWidget           *box;

        GtkWidget           *secondary_label;

        GtkWidget           *progressbar;

        GdkPixbuf           *root;

        cairo_pattern_t     *corner_pattern;

        cairo_pattern_t     *left_pattern;

        cairo_pattern_t     *top_pattern;

        int                  timeout;

        unsigned int         timeout_id;
} UksmShortcutsDialogPrivate;

static UksmShortcutsDialog *current_dialog = NULL;

static void uksm_shortcuts_dialog_set_timeout  (UksmShortcutsDialog *shortcuts_dialog);

static void uksm_shortcuts_dialog_draw         (UksmShortcutsDialog *shortcuts_dialog,
                                                cairo_t *cr);

static gboolean uksm_shortcuts_dialog_cancle   (UksmShortcutsDialog *shortcuts_dialog,
                                            GdkEvent        *event,
                                            gpointer        data);

static void uksm_shortcuts_dialog_destroy      (UksmShortcutsDialog *shortcuts_dialog,
                                            gpointer         data);

static void uksm_shortcuts_dialog_show         (UksmShortcutsDialog *shortcuts_dialog,
                                            gpointer         data);

G_DEFINE_TYPE_WITH_PRIVATE (UksmShortcutsDialog, uksm_shortcuts_dialog, GTK_TYPE_WINDOW);

static void
uksm_shortcuts_dialog_class_init (UksmShortcutsDialogClass *klass)
{
        GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

        dialog_signals[RESPONSE] =
                g_signal_new ("response",
                              G_TYPE_FROM_CLASS (gobject_class),
                              G_SIGNAL_RUN_LAST,
                              G_STRUCT_OFFSET (UksmShortcutsDialogClass,
                                               response),
                              NULL, NULL,
                              g_cclosure_marshal_VOID__VOID,
                              G_TYPE_NONE,
                              0);
}

static void
uksm_shortcuts_dialog_init (UksmShortcutsDialog *shortcuts_dialog)
{
        UksmShortcutsDialogPrivate *priv;

        priv = uksm_shortcuts_dialog_get_instance_private (shortcuts_dialog);
        priv->timeout_id = 0;
        priv->timeout = 0;
        GdkWindow *root_win = gdk_get_default_root_window ();
        int width = gdk_window_get_width (root_win);
        int height = gdk_window_get_height (root_win);
        priv->root = gdk_pixbuf_get_from_window (root_win, 0, 0, width, height);

        cairo_surface_t *corner = cairo_image_surface_create_from_png (DATA_DIR "/switcher_corner.png");
        cairo_surface_t *left = cairo_image_surface_create_from_png (DATA_DIR "/switcher_left.png");
        cairo_surface_t *top = cairo_image_surface_create_from_png (DATA_DIR "/switcher_top.png");

        priv->corner_pattern = cairo_pattern_create_for_surface (corner);
        priv->left_pattern = cairo_pattern_create_for_surface (left);
        cairo_pattern_set_extend (priv->left_pattern, CAIRO_EXTEND_REPEAT);
        priv->top_pattern = cairo_pattern_create_for_surface (top);
        cairo_pattern_set_extend (priv->top_pattern, CAIRO_EXTEND_REPEAT);

        cairo_surface_destroy (corner);
        cairo_surface_destroy (left);
        cairo_surface_destroy (top);

        gtk_window_set_skip_taskbar_hint (GTK_WINDOW (shortcuts_dialog), TRUE);
        gtk_window_set_keep_above (GTK_WINDOW (shortcuts_dialog), TRUE);
        gtk_window_stick (GTK_WINDOW (shortcuts_dialog));

        g_signal_connect (shortcuts_dialog,
                          "draw",
                          G_CALLBACK (uksm_shortcuts_dialog_draw),
                          NULL);

        g_signal_connect (shortcuts_dialog,
                          "destroy",
                          G_CALLBACK (uksm_shortcuts_dialog_destroy),
                          NULL);

        g_signal_connect (shortcuts_dialog,
                          "show",
                          G_CALLBACK (uksm_shortcuts_dialog_show),
                          NULL);
}

static void
uksm_shortcuts_dialog_draw (UksmShortcutsDialog *shortcuts_dialog,
                        cairo_t *cr)
{
        UksmShortcutsDialogPrivate *priv;

        cairo_set_source_rgba (cr, 0, 0, 0, 0.25);
        cairo_paint (cr);

        priv = uksm_shortcuts_dialog_get_instance_private (shortcuts_dialog);
        int box_width = gtk_widget_get_allocated_width (priv->box);
        int box_height = gtk_widget_get_allocated_height (priv->box);
        int win_width = gtk_widget_get_allocated_width (GTK_WIDGET(shortcuts_dialog));
        int win_height = gtk_widget_get_allocated_height (GTK_WIDGET(shortcuts_dialog));

        int x = (win_width - box_width) / 2;
        int y = (win_height - box_height) / 2;

        cairo_surface_t *center_surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, box_width, box_height);
        cairo_t *center_cr = cairo_create (center_surface);
        gdk_cairo_set_source_pixbuf (center_cr, priv->root, -x, -y);
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
        cairo_pattern_set_matrix (priv->corner_pattern, &matrix);
        cairo_set_source (cr, priv->corner_pattern);
        cairo_rectangle (cr, 0, 0, BORDER_SIZE, BORDER_SIZE);
        cairo_fill (cr);

        /* Top right */
        cairo_matrix_init_identity (&matrix);
        cairo_matrix_translate (&matrix, box_width, 0);
        cairo_matrix_scale (&matrix, -1, 1);
        cairo_pattern_set_matrix (priv->corner_pattern, &matrix);
        cairo_set_source (cr, priv->corner_pattern);
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
        cairo_pattern_set_matrix (priv->corner_pattern, &matrix);
        cairo_set_source (cr, priv->corner_pattern);
        cairo_rectangle (cr, 0, box_height - BORDER_SIZE, BORDER_SIZE, BORDER_SIZE);
        cairo_fill (cr);

        /* Bottom right */
        cairo_matrix_init_identity (&matrix);
        cairo_matrix_translate (&matrix, box_width, box_height);
        cairo_matrix_scale (&matrix, -1, -1);
        cairo_pattern_set_matrix (priv->corner_pattern, &matrix);
        cairo_set_source (cr, priv->corner_pattern);
        cairo_rectangle (cr, box_width - BORDER_SIZE, box_height - BORDER_SIZE, BORDER_SIZE, BORDER_SIZE);
        cairo_fill (cr);

        /* Left */
        cairo_matrix_init_identity (&matrix);
        cairo_pattern_set_matrix (priv->left_pattern, &matrix);
        cairo_set_source (cr, priv->left_pattern);
        cairo_rectangle (cr, 0, BORDER_SIZE, BORDER_SIZE, box_height - BORDER_SIZE * 2);
        cairo_fill (cr);

        /* Right */
        cairo_matrix_init_identity (&matrix);
        cairo_matrix_translate (&matrix, box_width, 0);
        cairo_matrix_scale (&matrix, -1, 1);
        cairo_pattern_set_matrix (priv->left_pattern, &matrix);
        cairo_set_source (cr, priv->left_pattern);
        cairo_rectangle (cr, box_width - BORDER_SIZE, BORDER_SIZE, BORDER_SIZE, box_height - BORDER_SIZE * 2);
        cairo_fill (cr);

        /* Top */
        cairo_matrix_init_identity (&matrix);
        cairo_pattern_set_matrix (priv->top_pattern, &matrix);
        cairo_set_source (cr, priv->top_pattern);
        cairo_rectangle (cr, BORDER_SIZE, 0, box_width - BORDER_SIZE * 2, BORDER_SIZE);
        cairo_fill (cr);

        /* Bottom */
        cairo_matrix_init_identity (&matrix);
        cairo_matrix_translate (&matrix, 0, box_height);
        cairo_matrix_scale (&matrix, 1, -1);
        cairo_pattern_set_matrix (priv->top_pattern, &matrix);
        cairo_set_source (cr, priv->top_pattern);
        cairo_rectangle (cr, BORDER_SIZE, box_height - BORDER_SIZE, box_width - BORDER_SIZE * 2, BORDER_SIZE);
        cairo_fill (cr);

        cairo_restore (cr);
}

static gboolean
uksm_shortcuts_dialog_cancle(UksmShortcutsDialog *shortcuts_dialog,
                         GdkEvent        *event,
                         gpointer        data)
{
        gtk_widget_destroy (GTK_WIDGET(data));

        return TRUE;
}

static void
uksm_shortcuts_dialog_destroy (UksmShortcutsDialog *shortcuts_dialog,
                               gpointer         data)
{
        UksmShortcutsDialogPrivate *priv;
        priv = uksm_shortcuts_dialog_get_instance_private (shortcuts_dialog);
        if (priv->timeout_id != 0) {
                g_source_remove (priv->timeout_id);
                priv->timeout_id = 0;
        }

        if (priv->root) {
                g_object_unref (priv->root);
                priv->root = NULL;
        }

        if (priv->corner_pattern) {
                cairo_pattern_destroy (priv->corner_pattern);
                priv->corner_pattern = NULL;
        }

        if (priv->left_pattern) {
                cairo_pattern_destroy (priv->left_pattern);
                priv->left_pattern = NULL;
        }

        if (priv->top_pattern) {
                cairo_pattern_destroy (priv->top_pattern);
                priv->top_pattern = NULL;
        }

        current_dialog = NULL;
}

static void
uksm_shortcuts_dialog_show (UksmShortcutsDialog *shortcuts_dialog, gpointer user_data)
{
        uksm_shortcuts_dialog_set_timeout (shortcuts_dialog);
}

static gboolean
uksm_shortcuts_dialog_timeout (gpointer data)
{
        UksmShortcutsDialog *shortcuts_dialog;
        char                *seconds_warning;
        char                *secondary_text;
        UksmShortcutsDialogPrivate *priv;

        shortcuts_dialog = (UksmShortcutsDialog *) data;
        priv = uksm_shortcuts_dialog_get_instance_private (shortcuts_dialog);
        if (!priv->timeout) {
                g_signal_emit(shortcuts_dialog, dialog_signals[RESPONSE], 0);

                return FALSE;
        }

        seconds_warning = ngettext ("This window will be automatically closed "
                                    "in %d second",
                                    "This window will be automatically closed "
                                    "in %d seconds",
                                    priv->timeout);

        seconds_warning = g_strdup_printf (seconds_warning, priv->timeout);
        secondary_text = g_strdup (seconds_warning);

        gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (priv->progressbar),
                                       priv->timeout / 10.0);
        gtk_progress_bar_set_text (GTK_PROGRESS_BAR (priv->progressbar),
                                   seconds_warning);

        priv->timeout--;

        g_free (secondary_text);
        g_free (seconds_warning);

        return TRUE;
}

static void
uksm_shortcuts_dialog_set_timeout (UksmShortcutsDialog *shortcuts_dialog)
{
        UksmShortcutsDialogPrivate *priv;
        priv = uksm_shortcuts_dialog_get_instance_private (shortcuts_dialog);
        priv->timeout = SHORTCUTS_TIMEOUT;

        if (priv->timeout > 0) {
                /* Sets the secondary text */
                uksm_shortcuts_dialog_timeout (shortcuts_dialog);

                if (priv->timeout_id != 0) {
                        g_source_remove (priv->timeout_id);
                }

                priv->timeout_id = g_timeout_add (1000,
                                                  uksm_shortcuts_dialog_timeout,
                                                  shortcuts_dialog);
        }
        else {
                gtk_widget_hide (priv->progressbar);
        }
}

static void
uksm_add_shortcut (GtkWidget *grid, char *text, UksmLabelType type, int left, int top, int width, int height)
{
        GtkWidget *label = gtk_label_new ("");

        char *tmp_format;
        switch (type) {
        case UKSM_LABEL_TYPE_TITLE:
                tmp_format = "<span color=\"white\" font=\"30\">\%s</span>";
                gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
                break;
        case UKSM_LABEL_TYPE_SUBTITLE:
                tmp_format = "<span color=\"white\" font=\"20\">\%s</span>";
                gtk_widget_set_halign (label, GTK_ALIGN_START);
                break;
        case UKSM_LABEL_TYPE_KEY:
                tmp_format = "<span color=\"white\" alpha=\"65535\" font=\"15\">\%s</span>";
                gtk_widget_set_halign (label, GTK_ALIGN_START);
                break;
        case UKSM_LABEL_TYPE_FUNCTION:
                tmp_format = "<span color=\"white\" alpha=\"65535\" font=\"13\">\%s</span>";
                gtk_widget_set_halign (label, GTK_ALIGN_START);
                break;
        default:
                break;
        }

        const char *format = tmp_format;
        char *markup = g_markup_printf_escaped (format, text);
        gtk_label_set_markup (GTK_LABEL (label), markup);
        g_free (markup);
        gtk_grid_attach (GTK_GRID (grid), label, left, top, width, height);
}

GtkWidget *
uksm_get_shortcuts_dialog (GdkScreen                *screen,
                           guint32                  activate_time)
{
        UksmShortcutsDialog *shortcuts_dialog;
        GtkWidget           *event_box;
        GtkWidget           *label_grid;
        UksmShortcutsDialogPrivate *priv;

        if (current_dialog != NULL) {
                gtk_widget_destroy (GTK_WIDGET (current_dialog));
        }

        shortcuts_dialog = g_object_new (UKSM_TYPE_SHORTCUTS_DIALOG, NULL);

        current_dialog = shortcuts_dialog;

        gtk_window_fullscreen (GTK_WINDOW (shortcuts_dialog));
        gtk_widget_set_visual (GTK_WIDGET (shortcuts_dialog), gdk_screen_get_rgba_visual(screen));
        gtk_widget_set_app_paintable(GTK_WIDGET(shortcuts_dialog), TRUE);

        event_box = gtk_event_box_new();
        gtk_container_add (GTK_CONTAINER(shortcuts_dialog), event_box);
        g_signal_connect (event_box,
                          "button-press-event",
                          G_CALLBACK (uksm_shortcuts_dialog_cancle),
                          shortcuts_dialog);

        priv = uksm_shortcuts_dialog_get_instance_private (shortcuts_dialog);
        priv->box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
        gtk_widget_set_valign (priv->box, GTK_ALIGN_CENTER);
        gtk_widget_set_halign (priv->box, GTK_ALIGN_CENTER);

        label_grid = gtk_grid_new ();
        gtk_grid_set_column_homogeneous (GTK_GRID(label_grid), TRUE);
        gtk_widget_set_margin_top (label_grid, 10);
        gtk_widget_set_margin_start (label_grid, 10);
        gtk_widget_set_margin_end (label_grid, 10);
        gtk_grid_set_column_spacing (GTK_GRID(label_grid), 5);

        uksm_add_shortcut (label_grid, _("Keyboard Shortcuts"), UKSM_LABEL_TYPE_TITLE, 0, 0, 4, 1);

        uksm_add_shortcut (label_grid, _("Application"), UKSM_LABEL_TYPE_SUBTITLE, 0, 1, 2, 1);
        uksm_add_shortcut (label_grid, _("Super"), UKSM_LABEL_TYPE_KEY, 0, 2, 1, 1);
        uksm_add_shortcut (label_grid, _("Open start menu."), UKSM_LABEL_TYPE_FUNCTION, 1, 2, 1, 1);
        uksm_add_shortcut (label_grid, _("Ctrl+Alt+T"), UKSM_LABEL_TYPE_KEY, 0, 3, 1, 1);
        uksm_add_shortcut (label_grid, _("Open termianl."), UKSM_LABEL_TYPE_FUNCTION, 1, 3, 1, 1);
        uksm_add_shortcut (label_grid, _("Ctrl+Alt+D"), UKSM_LABEL_TYPE_KEY, 0, 4, 1, 1);
        uksm_add_shortcut (label_grid, _("Show desktop."), UKSM_LABEL_TYPE_FUNCTION, 1, 4, 1, 1);
        uksm_add_shortcut (label_grid, _("Ctrl+Alt+L"), UKSM_LABEL_TYPE_KEY, 0, 5, 1, 1);
        uksm_add_shortcut (label_grid, _("Lock screen."), UKSM_LABEL_TYPE_FUNCTION, 1, 5, 1, 1);

        // Separator
        uksm_add_shortcut (label_grid, _("  "), UKSM_LABEL_TYPE_KEY, 0, 6, 2, 1);

        uksm_add_shortcut (label_grid, _("Screenshot"), UKSM_LABEL_TYPE_SUBTITLE, 0, 7, 2, 1);
        uksm_add_shortcut (label_grid, _("Print"), UKSM_LABEL_TYPE_KEY, 0, 8, 1, 1);
        uksm_add_shortcut (label_grid, _("Take a full-screen screenshot."), UKSM_LABEL_TYPE_FUNCTION, 1, 8, 1, 1);
        uksm_add_shortcut (label_grid, _("Ctrl+Print"), UKSM_LABEL_TYPE_KEY, 0, 9, 1, 1);
        uksm_add_shortcut (label_grid, _("Take a screenshot of the current window."), UKSM_LABEL_TYPE_FUNCTION, 1, 9, 1, 1);
        uksm_add_shortcut (label_grid, _("Shift+Print"), UKSM_LABEL_TYPE_KEY, 0, 10, 1, 1);
        uksm_add_shortcut (label_grid, _("Select area to grab."), UKSM_LABEL_TYPE_FUNCTION, 1, 10, 1, 1);

        uksm_add_shortcut (label_grid, _("Switching"), UKSM_LABEL_TYPE_SUBTITLE, 2, 1, 2, 1);
        uksm_add_shortcut (label_grid, _("Alt+Tab"), UKSM_LABEL_TYPE_KEY, 2, 2, 1, 1);
        uksm_add_shortcut (label_grid, _("Switches between applications."), UKSM_LABEL_TYPE_FUNCTION, 3, 2, 1, 1);
        uksm_add_shortcut (label_grid, _("Alt+Shift+Tab"), UKSM_LABEL_TYPE_KEY, 2, 3, 1, 1);
        uksm_add_shortcut (label_grid, _("Switches between applications in reverse order."), UKSM_LABEL_TYPE_FUNCTION, 3, 3, 1, 1);
        uksm_add_shortcut (label_grid, _("Super_L+P/F3/F7"), UKSM_LABEL_TYPE_KEY, 2, 4, 1, 1);
        uksm_add_shortcut (label_grid, _("Switches between display."), UKSM_LABEL_TYPE_FUNCTION, 3, 4, 1, 1);
        uksm_add_shortcut (label_grid, _("Ctrl+Alt+ArrowKeys"), UKSM_LABEL_TYPE_KEY, 2, 5, 1, 1);
        uksm_add_shortcut (label_grid, _("Switches between workspaces."), UKSM_LABEL_TYPE_FUNCTION, 3, 5, 1, 1);

        // Separator
        uksm_add_shortcut (label_grid, _("  "), UKSM_LABEL_TYPE_KEY, 2, 6, 2, 1);

        uksm_add_shortcut (label_grid, _("Windows"), UKSM_LABEL_TYPE_SUBTITLE, 2, 7, 2, 1);
        uksm_add_shortcut (label_grid, _("Alt+F10"), UKSM_LABEL_TYPE_KEY, 2, 8, 1, 1);
        uksm_add_shortcut (label_grid, _("Maximize the current window."), UKSM_LABEL_TYPE_FUNCTION, 3, 8, 1, 1);
        uksm_add_shortcut (label_grid, _("Alt+F9"), UKSM_LABEL_TYPE_KEY, 2, 9, 1, 1);
        uksm_add_shortcut (label_grid, _("Minimize the current window."), UKSM_LABEL_TYPE_FUNCTION, 3, 9, 1, 1);
        uksm_add_shortcut (label_grid, _("Alt+F5"), UKSM_LABEL_TYPE_KEY, 2, 10, 1, 1);
        uksm_add_shortcut (label_grid, _("Cancel maximize the current window."), UKSM_LABEL_TYPE_FUNCTION, 3, 10, 1, 1);
        uksm_add_shortcut (label_grid, _("Alt+F4"), UKSM_LABEL_TYPE_KEY, 2, 11, 1, 1);
        uksm_add_shortcut (label_grid, _("Close the current window."), UKSM_LABEL_TYPE_FUNCTION, 3, 11, 1, 1);

        gtk_container_add (GTK_CONTAINER (priv->box), label_grid);

        priv->secondary_label = gtk_label_new ("");
        gtk_box_pack_start (GTK_BOX (priv->box),
                            priv->secondary_label,
                            FALSE, FALSE, 0);

        priv->progressbar = gtk_progress_bar_new ();
        gtk_widget_set_halign (priv->progressbar, GTK_ALIGN_CENTER);
        gtk_progress_bar_set_show_text (GTK_PROGRESS_BAR (priv->progressbar), TRUE);
        gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (priv->progressbar), 1.0);
        gtk_box_pack_start (GTK_BOX (priv->box),
                            priv->progressbar,
                            FALSE, FALSE, 12);

        GtkStyleContext *context = gtk_widget_get_style_context (priv->progressbar);
        GtkCssProvider *provider = gtk_css_provider_new ();
        gtk_css_provider_load_from_data (GTK_CSS_PROVIDER (provider),
                                         "text {\n"
                                         "  color: white;\n"
                                          "}\n", -1, NULL);
        gtk_style_context_add_provider (context,
                                        GTK_STYLE_PROVIDER (provider),
                                        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        g_object_unref (provider);


        gtk_widget_show_all (priv->box);
        gtk_container_add (GTK_CONTAINER ( event_box), priv->box);

        gtk_window_set_screen (GTK_WINDOW (shortcuts_dialog), screen);

        return GTK_WIDGET (shortcuts_dialog);
}
