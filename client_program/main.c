#include <gtk/gtk.h>
#include <interface.h>
#include <time.h>

int main (int argc, char **argv) {
    GtkApplication *app;
    int status;

    time_t curr_time = time(NULL);
    // Генерация уникального идентификатора для каждого экземпляра
    char app_id[50];
    snprintf(app_id, sizeof(app_id), "org.gtk.client_kursovaya_%ld", curr_time);

    app = gtk_application_new(app_id, G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}