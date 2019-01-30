#include <config.h>

#include <glib/gi18n.h>

#include "uksm-shortcuts-dialog.h"

static gboolean show_version = FALSE;
static gboolean show_shortcuts = FALSE;

static void
shortcuts_dialog_response (GtkWidget *dialog,
                           gpointer data)
{
    g_debug ("GsmManager: Shortcuts dialog destroy");

    gtk_widget_destroy (GTK_WIDGET (dialog));

    gtk_main_quit ();
}

static gboolean
show_shortcuts_dialog ()
{
    GtkWidget *dialog = uksm_get_shortcuts_dialog (gdk_screen_get_default (),
                                                   gtk_get_current_event_time ());

    g_signal_connect (dialog,
                      "response",
                      G_CALLBACK (shortcuts_dialog_response),
                      NULL);
    gtk_widget_show_all (dialog);

    return FALSE;
}

static gboolean
first_run()
{
    gchar *dir = g_build_filename (g_get_user_config_dir(), "ukui-session", NULL);
    gchar *first_run_stamp = g_build_filename (dir, "first_run.stamp", NULL);
    if (!g_file_test(first_run_stamp, G_FILE_TEST_EXISTS))
    {
        if (g_mkdir_with_parents (dir, 0755) != 0)
        {
            g_warning ("Could not create directory '%s'", dir);
            exit (1);
        } else {
            GError *error = NULL;
            g_file_set_contents(first_run_stamp, "", 0, &error);
            if (error != NULL)
			{
                g_warning ("Impossible to saver the ukui-session stamp file: %s", error->message);
			    g_error_free (error);
                exit (1);
			}
        }

        return TRUE;
    }
    g_free (dir);
    g_free (first_run_stamp);

    return FALSE;
}

int main(int argc, char** argv)
{
    GError* error;

    bindtextdomain (GETTEXT_PACKAGE, LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

    static GOptionEntry options[] = {
        {"version", 0, 0, G_OPTION_ARG_NONE, &show_version, N_("Version of this application"), NULL},
        {"show", 0, 0, G_OPTION_ARG_NONE, &show_shortcuts, N_("Show the keyboard shortcuts"), NULL},
        {NULL, 0, 0, 0, NULL, NULL, NULL }
    };

    error = NULL;
	if (!gtk_init_with_args(&argc, &argv, NULL, options, NULL, &error))
	{
		g_warning ("Unable to start: %s", error->message);
		g_error_free (error);
		exit (1);
	}

    if (show_shortcuts || first_run ())
    {
        show_shortcuts_dialog ();
        gtk_main ();
    }

    return 0;
}