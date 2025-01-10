#include <gtk/gtk.h>

typedef struct
{
    GtkWidget *entry_server1;
    GtkWidget *entry_server2;
} AppData;

extern GtkWidget *window;
extern const char* server1_ip;
extern const char* server2_ip;

static void on_next_button_clicked(GtkWidget *widget, gpointer data)
{

    AppData *app_data = (AppData *)data;

    server1_ip = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(app_data->entry_server1)));
    server2_ip = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(app_data->entry_server2)));

    GtkWidget *screen1 = gtk_widget_get_parent(widget);
    GtkWidget *screen2 = gtk_widget_get_next_sibling(screen1);

    // Скрываем первый экран
    gtk_widget_set_visible(screen1, FALSE);

    // Показываем второй экран
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 650);

    gtk_widget_set_visible(screen2, TRUE);
}

void first_screen(GtkWidget* screen1) {

    GtkWidget *label1;
    GtkWidget *entry_server1;
    GtkWidget *label2;
    GtkWidget *entry_server2;
    GtkWidget *next_button;
    GtkEntryBuffer *buffer1 = gtk_entry_buffer_new("127.0.0.1", -1);
    GtkEntryBuffer *buffer2 = gtk_entry_buffer_new("127.0.0.1", -1);

    // ПЕРВЫЙ ЭКРАН - Ввод IP-адресов серверов //

    label1 = gtk_label_new("Введите IP первого сервера");
    gtk_box_append(GTK_BOX(screen1), label1);
    entry_server1 = gtk_entry_new_with_buffer(buffer1);
    gtk_box_append(GTK_BOX(screen1), entry_server1);

    label2 = gtk_label_new("Введите IP второго сервера");
    gtk_box_append(GTK_BOX(screen1), label2);
    entry_server2 = gtk_entry_new_with_buffer(buffer2);
    gtk_box_append(GTK_BOX(screen1), entry_server2);

    // Кнопка "Далее"
    AppData *app_data = g_new(AppData, 1);
    app_data->entry_server1 = entry_server1;
    app_data->entry_server2 = entry_server2;
    next_button = gtk_button_new_with_label("Next");
    g_signal_connect(next_button, "clicked", G_CALLBACK(on_next_button_clicked), app_data);
    gtk_box_append(GTK_BOX(screen1), next_button);
}