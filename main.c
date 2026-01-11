#include <stdio.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <stdbool.h>

#include "filesave.h"

static void layout(GtkApplication *app);
static void handle_click(GtkButton *btn, gpointer user_data);
static gboolean countdown(gpointer user_data);
void update_timer(GtkLabel *state_lbl, GtkLabel *clock_lbl);

typedef struct {
  int work_time;
  int break_time;
} Prefs;

typedef struct {
  GtkWidget *clock;
  GtkWidget *state_lbl;
  GtkWidget *btn;
} Elements;

typedef struct {
  int secs_left;
  bool is_ticking;
  bool is_work;
} AppState;

static Prefs app_prefs;
static AppState state;

int main(int argc, char *argv[])
{
  GtkApplication *app;
  int status;
  state.secs_left = 75;
  state.is_ticking = false;
  state.is_work = true;

  app_prefs.work_time = 75;
  app_prefs.break_time = 300;

  app = gtk_application_new("org.gtk.pomodoro", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(layout), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}

static void layout(GtkApplication *app)
{
  GtkWidget *window;
  GtkWidget *box;
  GtkWidget *clock;
  GtkWidget *state_lbl;
  GtkWidget *total_today;
  GtkWidget *btn;

  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Pomodoro");
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);

  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
  gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(box, GTK_ALIGN_CENTER);

  gtk_window_set_child(GTK_WINDOW(window), box);

  total_today = gtk_label_new("Total today: ");
  gtk_widget_set_halign(total_today, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(total_today, GTK_ALIGN_CENTER);
  gtk_box_append(GTK_BOX(box), total_today);
  g_print("Today's time: %d mins\n", read_today());

  state_lbl = gtk_label_new("Work");
  gtk_widget_set_halign(state_lbl, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(state_lbl, GTK_ALIGN_CENTER);
  gtk_box_append(GTK_BOX(box), state_lbl);

  int mins = state.secs_left / 60;
  int secs = state.secs_left % 60;
  gchar *text = g_strdup_printf("%2.2d:%2.2d", mins, secs);
  clock = gtk_label_new(text);
  g_free(text);
  gtk_widget_set_halign(clock, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(clock, GTK_ALIGN_CENTER);
  gtk_box_append(GTK_BOX(box), clock);
  
  btn = gtk_button_new_with_label("Start");
  gtk_widget_set_halign(btn, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(btn, GTK_ALIGN_CENTER);

  Elements *elems = g_malloc(sizeof(*elems));
  elems->clock = clock;
  elems->btn = btn;
  elems->state_lbl = state_lbl;

  g_signal_connect(btn, "clicked", G_CALLBACK(handle_click), elems);
  gtk_box_append(GTK_BOX(box), btn);

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
    state.secs_left = state.is_work ? app_prefs.work_time : app_prefs.break_time;
    gtk_button_set_label(btn, "Start");
    update_timer(state_lbl, clock);
    return G_SOURCE_REMOVE;
  }
  
  state.secs_left--;
  update_timer(state_lbl, clock);

  return state.secs_left >= 0;
}

void update_timer(GtkLabel *state_lbl, GtkLabel *clock_lbl)
{
  int mins = state.secs_left / 60;
  int secs = state.secs_left % 60;
  gchar *text = g_strdup_printf("%2.2d:%2.2d", mins, secs);
  gtk_label_set_text(clock_lbl, text);
  gtk_label_set_text(state_lbl, state.is_work ? "Work" : "Break");
  g_free(text);
}
