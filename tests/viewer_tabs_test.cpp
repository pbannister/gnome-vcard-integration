//
// viewer_tabs_test: the tab switch must not leave the page's first selectable
// label selected.
//
// A GtkNotebook focuses the first focusable child of the page it selects, and a
// selectable GtkLabel selects all of its text when it gains focus.  Clearing the
// focus alone leaves that selection behind; this test grabs the focus the way
// the notebook does, then checks that the helper clears the selection and that
// the label can still be selected by hand.
//
// Usage: viewer-tabs-test
//
#include "tools/viewer_focus.h"

#include "test_harness.h"

#include <gtk/gtk.h>

namespace {

void pump_events() {
    while (g_main_context_iteration(nullptr, FALSE)) {
    }
}

}  // namespace

int main() {
    gtk_init();

    GtkWidget *o_window = gtk_window_new();
    GtkWidget *o_notebook = gtk_notebook_new();
    GtkWidget *o_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *o_label = gtk_label_new("GNOME VCARD Integration");
    gtk_label_set_selectable(GTK_LABEL(o_label), TRUE);
    gtk_label_set_xalign(GTK_LABEL(o_label), 0.0f);
    gtk_box_append(GTK_BOX(o_page), o_label);
    gtk_notebook_append_page(GTK_NOTEBOOK(o_notebook), o_page, gtk_label_new("About"));
    gtk_window_set_child(GTK_WINDOW(o_window), o_notebook);
    gtk_window_present(GTK_WINDOW(o_window));
    pump_events();

    // This is what GtkNotebook does to the first focusable child of the page.
    gtk_widget_grab_focus(o_label);
    pump_events();

    int i_start = 0;
    int i_end = 0;
    EXPECT_TRUE(gtk_label_get_selection_bounds(GTK_LABEL(o_label), &i_start, &i_end));

    gnome_vcard::viewer::clear_window_focus_and_selection(GTK_WINDOW(o_window));
    pump_events();

    EXPECT_TRUE(nullptr == gtk_window_get_focus(GTK_WINDOW(o_window)));
    EXPECT_FALSE(gtk_label_get_selection_bounds(GTK_LABEL(o_label), &i_start, &i_end));

    // The clear must not make the label unselectable by hand.
    gtk_label_select_region(GTK_LABEL(o_label), 0, 5);
    EXPECT_TRUE(gtk_label_get_selection_bounds(GTK_LABEL(o_label), &i_start, &i_end));

    gtk_window_destroy(GTK_WINDOW(o_window));
    return vcard_test::finish("viewer-tabs-test");
}
