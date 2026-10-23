#pragma once

#include <gtk/gtk.h>

namespace gnome_vcard::viewer {

// Clear the window focus and any text selection a tab switch created.
//
// A GtkNotebook focuses the first focusable child of the page it selects.  In
// the vCard and About tabs that child is a selectable title label, and a
// selectable GtkLabel selects all of its text when it gains focus.  Clearing
// the window focus alone leaves that selection behind, so the title still
// arrives highlighted.  Clear the focus and then the selection; the values stay
// selectable by hand.
void clear_window_focus_and_selection(GtkWindow *o_window);

}  // namespace gnome_vcard::viewer
