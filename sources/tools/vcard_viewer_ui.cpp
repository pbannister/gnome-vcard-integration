//
// vcard-viewer: the GTK4 window a double-clicked card opens in.
//
// The window is built only where the GTK4 development files are present.  The
// command-line tools work without it, and `vcard-integrate handle` then prints
// the equivalent commands instead.
//
#include "render/contact_render.h"
#include "tools/viewer_content.h"
#include "vcard/vcard_reader.h"

#include <gtk/gtk.h>

#include <cctype>
#include <string>
#include <vector>

namespace {

const gnome_vcard::vcard::contact_o *g_contact = nullptr;

bool starts_with_ci(const std::string &s_text, const std::string &s_prefix) {
    if (s_text.size() < s_prefix.size()) {
        return false;
    }
    for (std::size_t u_at = 0; u_at < s_prefix.size(); ++u_at) {
        if (std::tolower(static_cast<unsigned char>(s_text[u_at])) !=
            std::tolower(static_cast<unsigned char>(s_prefix[u_at]))) {
            return false;
        }
    }
    return true;
}

bool is_safe_href(const std::string &s_value) {
    return starts_with_ci(s_value, "http://") || starts_with_ci(s_value, "https://") ||
           starts_with_ci(s_value, "mailto:") || starts_with_ci(s_value, "tel:");
}

std::string tel_target(const std::string &s_number) {
    std::string s_result;
    for (const char c_next : s_number) {
        if ('+' == c_next || ('0' <= c_next && '9' >= c_next)) {
            s_result.push_back(c_next);
        }
    }
    return s_result;
}

GtkWidget *make_label(const std::string &s_text, float f_scale, bool b_bold, bool b_dim,
                      bool b_wrap) {
    GtkWidget *o_label = gtk_label_new(s_text.c_str());
    gtk_label_set_xalign(GTK_LABEL(o_label), 0.0f);
    gtk_widget_set_halign(o_label, GTK_ALIGN_START);
    if (b_wrap) {
        gtk_label_set_wrap(GTK_LABEL(o_label), TRUE);
        gtk_label_set_wrap_mode(GTK_LABEL(o_label), PANGO_WRAP_WORD_CHAR);
        gtk_label_set_selectable(GTK_LABEL(o_label), TRUE);
        gtk_widget_set_hexpand(o_label, TRUE);
    }
    PangoAttrList *o_attributes = pango_attr_list_new();
    if (1.0f != f_scale) {
        pango_attr_list_insert(o_attributes, pango_attr_scale_new(f_scale));
    }
    if (b_bold) {
        pango_attr_list_insert(o_attributes, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
    }
    if (!o_attributes) {
        return o_label;
    }
    gtk_label_set_attributes(GTK_LABEL(o_label), o_attributes);
    pango_attr_list_unref(o_attributes);
    if (b_dim) {
        gtk_widget_add_css_class(o_label, "dim-label");
    }
    return o_label;
}

GtkWidget *make_heading(const std::string &s_text) {
    GtkWidget *o_label = gtk_label_new(s_text.c_str());
    gtk_label_set_xalign(GTK_LABEL(o_label), 0.0f);
    gtk_widget_set_halign(o_label, GTK_ALIGN_START);
    gtk_widget_set_margin_top(o_label, 12);
    PangoAttrList *o_attributes = pango_attr_list_new();
    pango_attr_list_insert(o_attributes, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
    pango_attr_list_insert(o_attributes, pango_attr_scale_new(0.9));
    gtk_label_set_attributes(GTK_LABEL(o_label), o_attributes);
    pango_attr_list_unref(o_attributes);
    gtk_widget_add_css_class(o_label, "dim-label");
    return o_label;
}

GtkWidget *make_row(const std::string &s_label, const std::string &s_value,
                    const std::string &s_href) {
    GtkWidget *o_row = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    if (!s_label.empty()) {
        GtkWidget *o_label = make_label(s_label, 0.85f, false, true, false);
        gtk_box_append(GTK_BOX(o_row), o_label);
    }
    if (!s_href.empty()) {
        GtkWidget *o_link = gtk_link_button_new_with_label(s_href.c_str(), s_value.c_str());
        gtk_widget_set_halign(o_link, GTK_ALIGN_START);
        gtk_widget_set_hexpand(o_link, TRUE);
        gtk_box_append(GTK_BOX(o_row), o_link);
    } else {
        GtkWidget *o_value = make_label(s_value, 1.0f, false, false, true);
        gtk_box_append(GTK_BOX(o_row), o_value);
    }
    return o_row;
}

std::string label_for(const gnome_vcard::vcard::property_o &o_property) {
    std::string s_label;
    for (const std::string &s_type : o_property.o_types) {
        if (starts_with_ci(s_type, "PREF") && 4 == s_type.size()) {
            continue;
        }
        if (!s_label.empty()) {
            s_label += ", ";
        }
        s_label += s_type;
    }
    if (o_property.b_preferred) {
        if (!s_label.empty()) {
            s_label += " · ";
        }
        s_label += "preferred";
    }
    return s_label;
}

GtkWidget *build_contact_page() {
    const gnome_vcard::vcard::contact_o &o_contact = *g_contact;

    GtkWidget *o_scroll = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(o_scroll, TRUE);

    GtkWidget *o_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_margin_top(o_box, 24);
    gtk_widget_set_margin_bottom(o_box, 24);
    gtk_widget_set_margin_start(o_box, 24);
    gtk_widget_set_margin_end(o_box, 24);

    GtkWidget *o_header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 18);
    GtkWidget *o_names = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_hexpand(o_names, TRUE);

    const std::string s_name = o_contact.formatted_name().empty()
                                   ? std::string("(no name)")
                                   : o_contact.formatted_name();
    gtk_box_append(GTK_BOX(o_names), make_label(s_name, 1.9f, true, false, true));

    std::string s_organization = o_contact.organization();
    if (!o_contact.organization_unit().empty()) {
        s_organization += " · " + o_contact.organization_unit();
    }
    if (!s_organization.empty()) {
        gtk_box_append(GTK_BOX(o_names), make_label(s_organization, 1.05f, false, false, true));
    }
    if (!o_contact.title().empty()) {
        gtk_box_append(GTK_BOX(o_names), make_label(o_contact.title(), 1.0f, false, true, true));
    }
    gtk_box_append(GTK_BOX(o_header), o_names);

    const gnome_vcard::vcard::photo_o o_photo = o_contact.photo();
    if (o_photo.b_embedded && gnome_vcard::render::is_embeddable_image(o_photo.s_media_type)) {
        GBytes *o_bytes = g_bytes_new(o_photo.s_bytes.data(), o_photo.s_bytes.size());
        GError *o_error = nullptr;
        GdkTexture *o_texture = gdk_texture_new_from_bytes(o_bytes, &o_error);
        g_bytes_unref(o_bytes);
        if (nullptr != o_texture) {
            GtkWidget *o_picture = gtk_picture_new_for_paintable(GDK_PAINTABLE(o_texture));
            gtk_picture_set_can_shrink(GTK_PICTURE(o_picture), TRUE);
            gtk_picture_set_content_fit(GTK_PICTURE(o_picture), GTK_CONTENT_FIT_CONTAIN);
            gtk_widget_set_size_request(o_picture, 144, 144);
            gtk_box_append(GTK_BOX(o_header), o_picture);
            g_object_unref(o_texture);
        }
        if (nullptr != o_error) {
            g_error_free(o_error);
        }
    }
    gtk_box_append(GTK_BOX(o_box), o_header);

    bool b_has_reach = false;
    for (const gnome_vcard::vcard::property_o &o_property : o_contact.emails()) {
        if (o_property.s_value.empty()) {
            continue;
        }
        if (!b_has_reach) {
            gtk_box_append(GTK_BOX(o_box), make_heading("Reach"));
            b_has_reach = true;
        }
        gtk_box_append(GTK_BOX(o_box),
                       make_row(label_for(o_property), o_property.s_value,
                                "mailto:" + o_property.s_value));
    }
    for (const gnome_vcard::vcard::property_o &o_property : o_contact.telephones()) {
        if (o_property.s_value.empty()) {
            continue;
        }
        if (!b_has_reach) {
            gtk_box_append(GTK_BOX(o_box), make_heading("Reach"));
            b_has_reach = true;
        }
        const std::string s_target = tel_target(o_property.s_value);
        gtk_box_append(GTK_BOX(o_box),
                       make_row(label_for(o_property), o_property.s_value,
                                s_target.empty() ? std::string() : "tel:" + s_target));
    }

    bool b_has_address = false;
    for (const gnome_vcard::vcard::address_o &o_address : o_contact.addresses()) {
        if (o_address.is_empty()) {
            continue;
        }
        if (!b_has_address) {
            gtk_box_append(GTK_BOX(o_box), make_heading("Address"));
            b_has_address = true;
        }
        std::string s_label;
        for (const std::string &s_type : o_address.o_types) {
            if (starts_with_ci(s_type, "PREF") && 4 == s_type.size()) {
                continue;
            }
            if (!s_label.empty()) {
                s_label += ", ";
            }
            s_label += s_type;
        }
        std::string s_value;
        for (const std::string &s_line : o_address.formatted_lines()) {
            if (!s_value.empty()) {
                s_value += "\n";
            }
            s_value += s_line;
        }
        gtk_box_append(GTK_BOX(o_box), make_row(s_label, s_value, std::string()));
    }

    bool b_has_web = false;
    for (const gnome_vcard::vcard::property_o &o_property : o_contact.urls()) {
        if (o_property.s_value.empty()) {
            continue;
        }
        if (!b_has_web) {
            gtk_box_append(GTK_BOX(o_box), make_heading("Web"));
            b_has_web = true;
        }
        const std::string s_href =
            is_safe_href(o_property.s_value) ? o_property.s_value : std::string();
        gtk_box_append(GTK_BOX(o_box), make_row("URL", o_property.s_value, s_href));
    }
    for (const gnome_vcard::vcard::property_o &o_property : o_contact.impps()) {
        if (o_property.s_value.empty()) {
            continue;
        }
        if (!b_has_web) {
            gtk_box_append(GTK_BOX(o_box), make_heading("Web"));
            b_has_web = true;
        }
        gtk_box_append(GTK_BOX(o_box),
                       make_row(label_for(o_property), o_property.s_value, std::string()));
    }

    bool b_has_details = false;
    const auto add_detail = [&b_has_details, o_box](const std::string &s_label,
                                                   const std::string &s_value) {
        if (s_value.empty()) {
            return;
        }
        if (!b_has_details) {
            gtk_box_append(GTK_BOX(o_box), make_heading("Details"));
            b_has_details = true;
        }
        gtk_box_append(GTK_BOX(o_box), make_row(s_label, s_value, std::string()));
    };
    add_detail("Role", o_contact.role());
    add_detail("Birthday", o_contact.birthday());
    add_detail("Anniversary", o_contact.anniversary());
    std::string s_nicknames;
    for (const std::string &s_nickname : o_contact.nicknames()) {
        if (!s_nicknames.empty()) {
            s_nicknames += ", ";
        }
        s_nicknames += s_nickname;
    }
    add_detail("Nickname", s_nicknames);

    if (!o_contact.note().empty()) {
        gtk_box_append(GTK_BOX(o_box), make_heading("Note"));
        gtk_box_append(GTK_BOX(o_box),
                       make_row(std::string(), o_contact.note(), std::string()));
    }

    for (const std::string &s_warning : o_contact.o_warnings) {
        gtk_box_append(GTK_BOX(o_box), make_label("warning: " + s_warning, 0.85f, false, true, true));
    }

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(o_scroll), o_box);
    return o_scroll;
}

GtkWidget *build_error_page(const std::string &s_message) {
    GtkWidget *o_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_top(o_box, 32);
    gtk_widget_set_margin_bottom(o_box, 32);
    gtk_widget_set_margin_start(o_box, 32);
    gtk_widget_set_margin_end(o_box, 32);
    gtk_box_append(GTK_BOX(o_box), make_label("This file could not be opened", 1.5f, true,
                                              false, true));
    gtk_box_append(GTK_BOX(o_box), make_label(s_message, 1.0f, false, true, true));
    return o_box;
}

GtkWidget *make_text_view(const std::string &s_text, bool b_vexpand) {
    GtkWidget *o_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(o_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(o_view), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(o_view), TRUE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(o_view), GTK_WRAP_WORD_CHAR);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(o_view), 4);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(o_view), 4);
    gtk_text_view_set_top_margin(GTK_TEXT_VIEW(o_view), 4);
    gtk_text_view_set_bottom_margin(GTK_TEXT_VIEW(o_view), 4);
    if (b_vexpand) {
        gtk_widget_set_vexpand(o_view, TRUE);
    }
    GtkTextBuffer *o_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(o_view));
    gtk_text_buffer_set_text(o_buffer, s_text.c_str(), -1);
    return o_view;
}

GtkWidget *build_discovered_page(const std::string &s_path) {
    GtkWidget *o_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_vexpand(o_box, TRUE);
    gtk_widget_set_margin_top(o_box, 20);
    gtk_widget_set_margin_bottom(o_box, 20);
    gtk_widget_set_margin_start(o_box, 20);
    gtk_widget_set_margin_end(o_box, 20);

    if (s_path.empty()) {
        gtk_box_append(GTK_BOX(o_box),
                       make_label("No file was given, so there is no vcard-inspect command to run.",
                                  1.0f, false, true, true));
        return o_box;
    }

    std::string s_command;
    std::string s_output;
    std::string s_error;
    int i_status = 0;
    const bool b_ran =
        gnome_vcard::viewer::run_inspect(s_path, s_command, s_output, i_status, s_error);

    gtk_box_append(GTK_BOX(o_box), make_heading("Command"));
    GtkWidget *o_command_scroll = gtk_scrolled_window_new();
    gtk_widget_set_size_request(o_command_scroll, -1, 58);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(o_command_scroll), GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_child(
        GTK_SCROLLED_WINDOW(o_command_scroll),
        make_text_view(b_ran ? s_command : "(vcard-inspect was not found)", false));
    gtk_box_append(GTK_BOX(o_box), o_command_scroll);

    gtk_box_append(GTK_BOX(o_box), make_heading("Output"));
    if (!b_ran) {
        gtk_box_append(GTK_BOX(o_box), make_label(s_error, 1.0f, false, true, true));
        return o_box;
    }

    GtkWidget *o_output_scroll = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(o_output_scroll, TRUE);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(o_output_scroll), GTK_POLICY_NEVER,
                                   GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_child(
        GTK_SCROLLED_WINDOW(o_output_scroll),
        make_text_view(s_output.empty() ? "(no output)" : s_output, false));
    gtk_box_append(GTK_BOX(o_box), o_output_scroll);
    gtk_box_append(GTK_BOX(o_box),
                   make_label("Exit status: " + std::to_string(i_status), 0.85f, false, true,
                              false));
    return o_box;
}

GtkWidget *build_about_page() {
    GtkWidget *o_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_top(o_box, 32);
    gtk_widget_set_margin_bottom(o_box, 32);
    gtk_widget_set_margin_start(o_box, 32);
    gtk_widget_set_margin_end(o_box, 32);

    gtk_box_append(GTK_BOX(o_box),
                   make_label(gnome_vcard::viewer::PROJECT_NAME, 1.8f, true, false, true));
    gtk_box_append(GTK_BOX(o_box),
                   make_label(gnome_vcard::viewer::version_line(), 1.0f, false, true, false));
    gtk_box_append(GTK_BOX(o_box),
                   make_label(gnome_vcard::viewer::PROJECT_DESCRIPTION, 1.0f, false, false, true));

    gtk_box_append(GTK_BOX(o_box), make_heading("Repository"));
    GtkWidget *o_link = gtk_link_button_new_with_label(gnome_vcard::viewer::PROJECT_REPOSITORY,
                                                       gnome_vcard::viewer::PROJECT_REPOSITORY);
    gtk_widget_set_halign(o_link, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(o_box), o_link);
    return o_box;
}

// GtkNotebook focuses the first focusable child of a page when the page is
// selected.  The first child is a selectable title label, so it arrives with a
// selection.  Clear the focus after the switch settles; the values remain
// selectable, but nothing is selected just because a tab was opened.
gboolean clear_window_focus(gpointer o_data) {
    gtk_window_set_focus(GTK_WINDOW(o_data), nullptr);
    return G_SOURCE_REMOVE;
}

void on_notebook_switch_page(GtkNotebook * /*o_notebook*/, GtkWidget * /*o_page*/,
                             guint /*u_page_number*/, gpointer o_data) {
    g_idle_add(clear_window_focus, o_data);
}

void on_window_destroy(GtkWidget *, gpointer o_data) {
    GMainLoop *o_loop = static_cast<GMainLoop *>(o_data);
    if (nullptr != o_loop) {
        g_main_loop_quit(o_loop);
    }
}

}  // namespace

int main(int i_argument_count, char *o_arguments[]) {
    g_set_prgname("vcard-viewer");
    gtk_init();

    std::string s_path;
    if (1 < i_argument_count) {
        s_path = o_arguments[1];
    }

    std::string s_title = "vCard";
    GtkWidget *o_vcard_page = nullptr;
    gnome_vcard::vcard::file_o o_file;

    if (s_path.empty()) {
        o_vcard_page = build_error_page("usage: vcard-viewer FILE.vcf");
    } else {
        std::string s_error;
        if (!gnome_vcard::vcard::read_vcards(s_path, o_file, s_error)) {
            o_vcard_page = build_error_page(s_error);
        } else if (o_file.o_contacts.empty()) {
            o_vcard_page = build_error_page(s_path + " contains no contact");
        } else {
            g_contact = &o_file.o_contacts.front();
            s_title = g_contact->formatted_name().empty() ? std::string("vCard")
                                                          : g_contact->formatted_name() + " — vCard";
            o_vcard_page = build_contact_page();
        }
    }
    if (nullptr == o_vcard_page) {
        o_vcard_page = build_error_page("the card could not be displayed");
    }

    GtkWidget *o_notebook = gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(o_notebook), GTK_POS_TOP);
    gtk_notebook_append_page(GTK_NOTEBOOK(o_notebook), o_vcard_page,
                             gtk_label_new(gnome_vcard::viewer::TAB_VCARD));
    gtk_notebook_append_page(GTK_NOTEBOOK(o_notebook), build_discovered_page(s_path),
                             gtk_label_new(gnome_vcard::viewer::TAB_DISCOVERED));
    gtk_notebook_append_page(GTK_NOTEBOOK(o_notebook), build_about_page(),
                             gtk_label_new(gnome_vcard::viewer::TAB_ABOUT));
    gtk_notebook_set_current_page(GTK_NOTEBOOK(o_notebook), 0);

    GtkWidget *o_window = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(o_window), s_title.c_str());
    gtk_window_set_default_size(GTK_WINDOW(o_window), 640, 760);
    gtk_window_set_child(GTK_WINDOW(o_window), o_notebook);
    g_signal_connect(o_notebook, "switch-page", G_CALLBACK(on_notebook_switch_page), o_window);

    GMainLoop *o_loop = g_main_loop_new(nullptr, FALSE);
    g_signal_connect(o_window, "destroy", G_CALLBACK(on_window_destroy), o_loop);
    gtk_window_present(GTK_WINDOW(o_window));
    // The first page is focused as the window is presented; clear that too.
    g_idle_add(clear_window_focus, o_window);
    g_main_loop_run(o_loop);
    g_main_loop_unref(o_loop);

    g_contact = nullptr;
    return 0;
}
