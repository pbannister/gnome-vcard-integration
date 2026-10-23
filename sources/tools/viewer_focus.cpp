#include "tools/viewer_focus.h"

namespace gnome_vcard::viewer {

namespace {

// A GtkLabel keeps its selection when it loses focus.  Clear it explicitly;
// select_region with an empty range leaves the cursor at the start and selects
// nothing.  A GtkTextView would otherwise keep its selection in the same way.
void clear_widget_selection(GtkWidget *o_widget) {
    if (GTK_IS_LABEL(o_widget)) {
        GtkLabel *o_label = GTK_LABEL(o_widget);
        if (gtk_label_get_selectable(o_label)) {
            int i_start = 0;
            int i_end = 0;
            if (gtk_label_get_selection_bounds(o_label, &i_start, &i_end)) {
                gtk_label_select_region(o_label, 0, 0);
            }
        }
    } else if (GTK_IS_TEXT_VIEW(o_widget)) {
        GtkTextBuffer *o_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(o_widget));
        GtkTextIter o_start;
        GtkTextIter o_end;
        if (gtk_text_buffer_get_selection_bounds(o_buffer, &o_start, &o_end)) {
            gtk_text_buffer_place_cursor(o_buffer, &o_start);
        }
    }
    for (GtkWidget *o_child = gtk_widget_get_first_child(o_widget);
         nullptr != o_child; o_child = gtk_widget_get_next_sibling(o_child)) {
        clear_widget_selection(o_child);
    }
}

}  // namespace

void clear_window_focus_and_selection(GtkWindow *o_window) {
    gtk_window_set_focus(o_window, nullptr);
    GtkWidget *o_child = gtk_window_get_child(o_window);
    if (nullptr != o_child) {
        clear_widget_selection(o_child);
    }
}

}  // namespace gnome_vcard::viewer
