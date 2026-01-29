#include <gtk/gtk.h>

G_DECLARE_FINAL_TYPE (PomoSettingsDialog, pomo_settings_dialog, POMO, SETTINGS_DIALOG, GtkWindow);

/*
creates and displays a new settings modal window
*/
void on_settings_clicked(GtkButton *btn, gpointer user_data);