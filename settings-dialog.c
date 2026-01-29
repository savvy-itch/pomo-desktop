#include <gtk/gtk.h>

#include "settings-dialog.h"

struct _PomoSettingsDialog
{
  GtkWindow parent;
  GSettings *settings;
  GtkSpinButton *workdur;
  GtkSpinButton *breakdur;
  GtkDropDown *theme;
};

G_DEFINE_TYPE (PomoSettingsDialog, pomo_settings_dialog, GTK_TYPE_WINDOW);

static void pomo_settings_dialog_init(PomoSettingsDialog *win)
{
  gtk_widget_init_template(GTK_WIDGET(win));
  
  win->settings = g_settings_new("org.gtk.pomodoro");
  g_settings_bind(win->settings, "workdur", win->workdur, "value", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(win->settings, "breakdur", win->breakdur, "value", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(win->settings, "theme", win->theme, "selected", G_SETTINGS_BIND_DEFAULT);
  g_assert(win->workdur);
  g_assert(win->breakdur);
  g_assert(win->theme);
}

/*
Clears reference to a GObject and drop all references to other objects
*/
static void pomo_settings_dialog_dispose(GObject *gobject)
{
  PomoSettingsDialog *win;
  win = POMO_SETTINGS_DIALOG(gobject);
  g_clear_object(&win->settings);
  G_OBJECT_CLASS(pomo_settings_dialog_parent_class)->dispose(gobject);
}

static void pomo_settings_dialog_class_init(PomoSettingsDialogClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  object_class->dispose = pomo_settings_dialog_dispose;

  gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS (klass), "/org/gtk/pomodoro/ui/settings.ui");
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS (klass), PomoSettingsDialog, workdur);
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS (klass), PomoSettingsDialog, breakdur);
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS (klass), PomoSettingsDialog, theme);
}

GtkWindow *pomo_settings_dialog_new(GtkWindow *parent)
{
  return g_object_new(pomo_settings_dialog_get_type(), "transient-for", parent, NULL);
}

void on_settings_clicked(GtkButton *btn, gpointer user_data)
{
  GtkWindow *parent = GTK_WINDOW(user_data);
  GtkWindow *win = pomo_settings_dialog_new(parent);
  gtk_window_present(win);
}
