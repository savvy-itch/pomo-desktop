#include <stdio.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <stdbool.h>

#include "filesave.h"
#include "settings-dialog.h"

typedef struct {
  GtkWidget *clock;
  GtkWidget *state_lbl;
  GtkWidget *btn;
} Elements;

typedef struct {
  GtkWidget *total_today_lbl;
} RecordData;

typedef struct {
  int secs_left;
  bool is_ticking;
  bool is_work;
  short total_today;
  GtkWidget *clock_lbl;
} AppState;

static void layout(GtkApplication *app, AppState *state);
static void handle_click(GtkButton *btn, gpointer user_data);
static gboolean countdown(gpointer user_data);
static gboolean handle_record_update(gpointer user_data);
static void update_timer(GtkLabel *state_lbl, GtkLabel *clock_lbl);
static void on_settings_changed(GSettings *settings, gchar *key, gpointer user_data);

static AppState state;

int main(int argc, char *argv[])
{
  GtkApplication *app;
  int status;
  GSettings *settings = g_settings_new("org.gtk.pomodoro");
  int work_dur = g_settings_get_int(settings, "workdur");
  state.secs_left = work_dur;
  state.is_ticking = false;
  state.is_work = true;

  app = gtk_application_new("org.gtk.pomodoro", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(layout), &state);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}

static void layout(GtkApplication *app, AppState *state)
{
  GtkWidget *window;
  GtkWidget *box;
  GtkWidget *clock_lbl;
  GtkWidget *state_lbl;
  GtkWidget *total_today_lbl;
  GtkWidget *btn;
  GtkWidget *settings_btn;

  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Pomodoro");
  gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);

  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
  gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(box, GTK_ALIGN_CENTER);

  gtk_window_set_child(GTK_WINDOW(window), box);

  short read_mins = read_today();
  state->total_today = read_mins;
  gchar *total_text = g_strdup_printf("Time working today: %hdh %hdmin", state->total_today / 60, state->total_today % 60);
  total_today_lbl = gtk_label_new(total_text);
  g_free(total_text);
  gtk_box_append(GTK_BOX(box), total_today_lbl);
  RecordData *rec_data = g_malloc(sizeof(*rec_data));
  rec_data->total_today_lbl = total_today_lbl;
  g_timeout_add_seconds(UPD_FREQ, handle_record_update, rec_data);

  settings_btn = gtk_button_new_with_label("Settings");
  g_signal_connect(settings_btn, "clicked", G_CALLBACK(on_settings_clicked), window);
  gtk_box_append(GTK_BOX(box), settings_btn);

  state_lbl = gtk_label_new("Work");
  gtk_widget_set_halign(state_lbl, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(state_lbl, GTK_ALIGN_CENTER);
  gtk_box_append(GTK_BOX(box), state_lbl);

  int mins = state->secs_left / 60;
  int secs = state->secs_left % 60;
  gchar *text = g_strdup_printf("%2.2d:%2.2d", mins, secs);
  clock_lbl = gtk_label_new(text);
  g_free(text);
  gtk_widget_set_halign(clock_lbl, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(clock_lbl, GTK_ALIGN_CENTER);
  gtk_box_append(GTK_BOX(box), clock_lbl);
  state->clock_lbl = clock_lbl;
  GSettings *settings = g_settings_new("org.gtk.pomodoro");
  g_signal_connect(settings, "changed", G_CALLBACK(on_settings_changed), NULL);
  
  btn = gtk_button_new_with_label("Start");
  gtk_widget_set_halign(btn, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(btn, GTK_ALIGN_CENTER);

  Elements *elems = g_malloc(sizeof(*elems));
  elems->clock = clock_lbl;
  elems->btn = btn;
  elems->state_lbl = state_lbl;

  g_signal_connect(btn, "clicked", G_CALLBACK(handle_click), elems);
  gtk_box_append(GTK_BOX(box), btn);

  // present a window to the user
  gtk_window_present(GTK_WINDOW(window));
}

static void handle_click(GtkButton *btn, gpointer user_data)
{
  state.is_ticking = !state.is_ticking;

  if (state.is_ticking) {
    gtk_button_set_label(btn, "Stop");
    g_timeout_add_seconds(1, countdown, user_data);
  } else {
    gtk_button_set_label(btn, "Start");
  }
}

static gboolean countdown(gpointer user_data)
{
  if (!state.is_ticking) {
    return G_SOURCE_REMOVE;
  }

  Elements *elems = user_data;
  GtkLabel *clock = GTK_LABEL(elems->clock);
  GtkLabel *state_lbl = GTK_LABEL(elems->state_lbl);
  GtkButton *btn = GTK_BUTTON(elems->btn);
  
  if (state.secs_left == 0) {
    state.is_work = !state.is_work;
    state.is_ticking = false;
    GSettings *settings = g_settings_new("org.gtk.pomodoro");
    int work_dur = g_settings_get_int(settings, "workdur");
    int break_dur = g_settings_get_int(settings, "breakdur");
    // if work interval has passed
    if (!state.is_work) {
      state.total_today += work_dur;
    }
    state.secs_left = state.is_work ? work_dur : break_dur;
    gtk_button_set_label(btn, "Start");
    update_timer(state_lbl, clock);
    return G_SOURCE_REMOVE;
  }
  
  state.secs_left--;
  update_timer(state_lbl, clock);

  return state.secs_left >= 0;
}

static void update_timer(GtkLabel *state_lbl, GtkLabel *clock_lbl)
{
  int mins = state.secs_left / 60;
  int secs = state.secs_left % 60;
  gchar *text = g_strdup_printf("%2.2d:%2.2d", mins, secs);
  gtk_label_set_text(clock_lbl, text);
  gtk_label_set_text(state_lbl, state.is_work ? "Work" : "Break");
  g_free(text);
}

static gboolean handle_record_update(gpointer user_data)
{
  RecordData *data = user_data;
  GtkLabel *total_today_lbl = GTK_LABEL(data->total_today_lbl);
  write_today(state.total_today);
  gchar *total_s = g_strdup_printf("Time working today: %hdh %hdmin", state.total_today / 60, state.total_today % 60);
  gtk_label_set_text(total_today_lbl, total_s);
  g_free(total_s);
  return TRUE;
}

static void on_settings_changed(GSettings *settings, gchar *key, gpointer user_data)
{
  if (!state.is_ticking) {
    int dur = state.is_work 
      ? g_settings_get_int(settings, "workdur") 
      : g_settings_get_int(settings, "breakdur");
    state.secs_left = dur;
    int mins = state.secs_left / 60;
    int secs = state.secs_left % 60;
    gchar *text = g_strdup_printf("%2.2d:%2.2d", mins, secs);
    gtk_label_set_text(GTK_LABEL(state.clock_lbl), text);
    g_free(text);
  }
}
