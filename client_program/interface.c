#include <gtk/gtk.h>
#include <screens.h>

GtkWidget *window;
const char *server1_ip;
const char *server2_ip;

void activate(GtkApplication *app, gpointer user_data)
{
  GtkWidget *box;
  GtkWidget *screen1;
  GtkWidget *screen2;
  
  GtkWidget *button_server1;
  GtkWidget *button_server2;

  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Server Configuration");
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 200);

  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_window_set_child(GTK_WINDOW(window), box);

  screen1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_box_append(GTK_BOX(box), screen1);

  screen2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_widget_set_visible(screen2, FALSE);
  gtk_box_append(GTK_BOX(box), screen2);

  first_screen(screen1);

  second_screen(screen2);

  gtk_window_present(GTK_WINDOW(window));
}