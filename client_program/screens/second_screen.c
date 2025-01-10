#include <gtk/gtk.h>
#include <pthread.h>
#include <../include/logic.h>

extern GtkWidget *window;
extern const char* server1_ip;
extern const char* server2_ip;

static GtkWidget *text_view;
GtkWidget *scrolled_window;
GtkWidget *button_server1;
GtkWidget *button_server2;

static pthread_t server1_thread_id = 0;
static pthread_t server2_thread_id = 0;

static gboolean adjust_scroll_position(gpointer data) {
    GtkAdjustment *vadjustment = GTK_ADJUSTMENT(data);
    gdouble upper = gtk_adjustment_get_upper(vadjustment);
    gdouble page_size = gtk_adjustment_get_page_size(vadjustment);
    gdouble max_value = upper - page_size;

    // Если верхняя граница больше нуля, устанавливаем значение прокрутки
    if (max_value > 0) {
        gtk_adjustment_set_value(vadjustment, max_value);
    }

    return FALSE;  // Прекращаем таймер после одного срабатывания
}
static void append_text_to_text_view_callback(const char *text) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    GtkTextIter end_iter;

    // Добавляем текст
    gtk_text_buffer_get_end_iter(buffer, &end_iter);
    gtk_text_buffer_insert(buffer, &end_iter, text, -1);
    gtk_text_buffer_insert(buffer, &end_iter, "\n", -1);

    gtk_widget_queue_resize(text_view);

    // Получаем объект вертикальной прокрутки
    GtkAdjustment *vadjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrolled_window));

    // Настройка таймера на 50 миллисекунд
    g_timeout_add(50, adjust_scroll_position, vadjustment);
}

static void append_text_to_text_view(const char *text) {
    // Обновление текста должно быть выполнено в главном потоке
    g_idle_add((GSourceFunc) append_text_to_text_view_callback, (gpointer) text);
}

static void *server1_thread_func(void *arg) {
    const char *server_ip = (const char *)arg;
    server1_getinfo(server_ip, append_text_to_text_view);
}

static void *server2_thread_func(void *arg) {
    const char *server_ip = (const char *)arg;
    server2_getinfo(server_ip, append_text_to_text_view);
}

static void server1_logic(GtkWidget *widget, gpointer data)
{   if (server1_thread_id == 0) {
        if (pthread_create(&server1_thread_id, NULL, server1_thread_func, (void *)server1_ip) != 0) {
            g_printerr("Failed to create thread\n");
        } else {
            pthread_detach(server1_thread_id);
            gtk_button_set_label(button_server1, "Отключиться");
        }
    } else {
        pthread_cancel(server1_thread_id);

        pthread_join(server1_thread_id, NULL);

        printf("Thread was cancelled successfully.\n");

        server1_thread_id = 0;
        gtk_button_set_label(button_server1, "Подключиться к Server 1");
    }
}

static void server2_logic(GtkWidget *widget, gpointer data)
{
    if (server2_thread_id == 0) {
        if (pthread_create(&server2_thread_id, NULL, server2_thread_func, (void *)server2_ip) != 0) {
            g_printerr("Failed to create thread\n");
        } else {
            pthread_detach(server2_thread_id);
            gtk_button_set_label(button_server2, "Отключиться");
        }
    } else {
        pthread_cancel(server2_thread_id);
        pthread_join(server2_thread_id, NULL);

        printf("Thread was cancelled successfully.\n");

        server2_thread_id = 0;
        gtk_button_set_label(button_server2, "Подключиться к Server 2");
    }
}

void second_screen(GtkWidget *screen2)
{

    button_server1 = gtk_button_new_with_label("Подключиться к Server 1");
    g_signal_connect(button_server1, "clicked", G_CALLBACK(server1_logic), NULL);
    gtk_widget_set_size_request(button_server1, -1, 100);
    gtk_box_append(GTK_BOX(screen2), button_server1);

    button_server2 = gtk_button_new_with_label("Подключиться к Server 2");
    g_signal_connect(button_server2, "clicked", G_CALLBACK(server2_logic), NULL);
    gtk_widget_set_size_request(button_server2, -1, 100);
    gtk_box_append(GTK_BOX(screen2), button_server2);

    // Create a text view and add it below the buttons
    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_widget_set_size_request(text_view, -1, 400);

    scrolled_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
    gtk_widget_set_size_request(scrolled_window, -1, 400);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), text_view);

    gtk_box_append(GTK_BOX(screen2), scrolled_window);
}