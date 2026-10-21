#include "render/contact_render.h"

#include "util/file_io.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

namespace gnome_vcard::render {
namespace {

constexpr int WIDTH = 760;
constexpr int HEADER_HEIGHT = 150;
constexpr int MARGIN = 36;
constexpr int MAX_CHARS = 78;

std::string escape_xml(const std::string &s_text) {
    std::string s_result;
    s_result.reserve(s_text.size());
    for (const char c_next : s_text) {
        switch (c_next) {
            case '&':
                s_result += "&amp;";
                break;
            case '<':
                s_result += "&lt;";
                break;
            case '>':
                s_result += "&gt;";
                break;
            case '"':
                s_result += "&quot;";
                break;
            case '\'':
                s_result += "&apos;";
                break;
            default:
                s_result.push_back(c_next);
                break;
        }
    }
    return s_result;
}

bool starts_with_ci(const std::string &s_text, const std::string &s_prefix) {
    if (s_text.size() < s_prefix.size()) {
        return false;
    }
    for (std::size_t u_at = 0; u_at < s_prefix.size(); ++u_at) {
        const char c_left = static_cast<char>(
            std::tolower(static_cast<unsigned char>(s_text[u_at])));
        const char c_right = static_cast<char>(
            std::tolower(static_cast<unsigned char>(s_prefix[u_at])));
        if (c_left != c_right) {
            return false;
        }
    }
    return true;
}

bool same_ci(const std::string &s_left, const std::string &s_right) {
    return s_left.size() == s_right.size() && starts_with_ci(s_left, s_right);
}


// A value becomes a link only when it names a scheme this project trusts.  A
// card is untrusted input, so javascript: and data: are never link targets.
bool is_safe_href(const std::string &s_value) {
    return starts_with_ci(s_value, "http://") || starts_with_ci(s_value, "https://") ||
           starts_with_ci(s_value, "mailto:") || starts_with_ci(s_value, "tel:");
}

std::string tel_target(const std::string &s_number) {
    std::string s_result;
    for (const char c_next : s_number) {
        if ('+' == c_next || (c_next >= '0' && c_next <= '9')) {
            s_result.push_back(c_next);
        }
    }
    return s_result;
}

std::string label_for(const vcard::property_o &o_property) {
    std::string s_label;
    for (const std::string &s_type : o_property.o_types) {
        if (same_ci(s_type, "PREF")) {
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

std::vector<std::string> wrap_text(const std::string &s_text, std::size_t u_width) {
    std::vector<std::string> o_lines;

    // Split the whole value on its newlines so an explicit break is preserved
    // as a break; each paragraph is then wrapped on its spaces.
    std::string s_current_paragraph;
    std::vector<std::string> o_paragraphs;
    for (const char c_next : s_text) {
        if ('\n' == c_next) {
            o_paragraphs.push_back(s_current_paragraph);
            s_current_paragraph.clear();
            continue;
        }
        s_current_paragraph.push_back(c_next);
    }
    o_paragraphs.push_back(s_current_paragraph);

    for (const std::string &s_one : o_paragraphs) {
        if (s_one.empty()) {
            o_lines.emplace_back();
            continue;
        }
        std::istringstream s_words(s_one);
        std::string s_word;
        std::string s_line;
        while (s_words >> s_word) {
            if (!s_line.empty() && s_line.size() + 1 + s_word.size() > u_width) {
                o_lines.push_back(s_line);
                s_line.clear();
            }
            if (!s_line.empty()) {
                s_line.push_back(' ');
            }
            if (s_word.size() > u_width) {
                while (s_word.size() > u_width) {
                    if (!s_line.empty()) {
                        o_lines.push_back(s_line);
                        s_line.clear();
                    }
                    o_lines.push_back(s_word.substr(0, u_width));
                    s_word = s_word.substr(u_width);
                }
            }
            s_line += s_word;
        }
        if (!s_line.empty() || o_lines.empty()) {
            o_lines.push_back(s_line);
        }
    }

    if (o_lines.empty()) {
        o_lines.emplace_back();
    }
    return o_lines;
}

struct entry_o {
    std::string s_label;
    std::string s_value;
    std::string s_href;
};

struct section_o {
    std::string s_title;
    std::vector<entry_o> o_entries;
};

int entry_height(const entry_o &o_entry) {
    int i_height = 0;
    if (!o_entry.s_label.empty()) {
        i_height += 15;
    }
    i_height += static_cast<int>(wrap_text(o_entry.s_value, MAX_CHARS).size()) * 21;
    i_height += 10;
    return i_height;
}

std::vector<section_o> build_sections(const vcard::contact_o &o_contact) {
    std::vector<section_o> o_sections;

    section_o o_emails;
    o_emails.s_title = "Email";
    for (const vcard::property_o &o_property : o_contact.emails()) {
        if (o_property.s_value.empty()) {
            continue;
        }
        entry_o o_entry;
        o_entry.s_label = label_for(o_property);
        o_entry.s_value = o_property.s_value;
        o_entry.s_href = "mailto:" + o_property.s_value;
        o_emails.o_entries.push_back(std::move(o_entry));
    }
    if (!o_emails.o_entries.empty()) {
        o_sections.push_back(std::move(o_emails));
    }

    section_o o_telephones;
    o_telephones.s_title = "Telephone";
    for (const vcard::property_o &o_property : o_contact.telephones()) {
        if (o_property.s_value.empty()) {
            continue;
        }
        entry_o o_entry;
        o_entry.s_label = label_for(o_property);
        o_entry.s_value = o_property.s_value;
        const std::string s_target = tel_target(o_property.s_value);
        if (!s_target.empty()) {
            o_entry.s_href = "tel:" + s_target;
        }
        o_telephones.o_entries.push_back(std::move(o_entry));
    }
    if (!o_telephones.o_entries.empty()) {
        o_sections.push_back(std::move(o_telephones));
    }

    section_o o_addresses;
    o_addresses.s_title = "Address";
    for (const vcard::address_o &o_address : o_contact.addresses()) {
        if (o_address.is_empty()) {
            continue;
        }
        entry_o o_entry;
        for (const std::string &s_type : o_address.o_types) {
            if (same_ci(s_type, "PREF")) {
                continue;
            }
            if (!o_entry.s_label.empty()) {
                o_entry.s_label += ", ";
            }
            o_entry.s_label += s_type;
        }
        if (o_address.b_preferred) {
            if (!o_entry.s_label.empty()) {
                o_entry.s_label += " · ";
            }
            o_entry.s_label += "preferred";
        }
        o_entry.s_value = o_address.formatted();
        o_addresses.o_entries.push_back(std::move(o_entry));
    }
    if (!o_addresses.o_entries.empty()) {
        o_sections.push_back(std::move(o_addresses));
    }

    section_o o_web;
    o_web.s_title = "Web";
    for (const vcard::property_o &o_property : o_contact.urls()) {
        if (o_property.s_value.empty()) {
            continue;
        }
        entry_o o_entry;
        o_entry.s_label = "URL";
        o_entry.s_value = o_property.s_value;
        if (is_safe_href(o_property.s_value)) {
            o_entry.s_href = o_property.s_value;
        }
        o_web.o_entries.push_back(std::move(o_entry));
    }
    for (const vcard::property_o &o_property : o_contact.impps()) {
        if (o_property.s_value.empty()) {
            continue;
        }
        entry_o o_entry;
        o_entry.s_label = label_for(o_property);
        if (o_entry.s_label.empty()) {
            o_entry.s_label = "IM";
        }
        o_entry.s_value = o_property.s_value;
        o_web.o_entries.push_back(std::move(o_entry));
    }
    if (!o_web.o_entries.empty()) {
        o_sections.push_back(std::move(o_web));
    }

    section_o o_details;
    o_details.s_title = "Details";
    const auto add_detail = [&o_details](const std::string &s_label,
                                         const std::string &s_value) {
        if (s_value.empty()) {
            return;
        }
        entry_o o_entry;
        o_entry.s_label = s_label;
        o_entry.s_value = s_value;
        o_details.o_entries.push_back(std::move(o_entry));
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
    if (!o_details.o_entries.empty()) {
        o_sections.push_back(std::move(o_details));
    }

    section_o o_note;
    o_note.s_title = "Note";
    if (!o_contact.note().empty()) {
        entry_o o_entry;
        o_entry.s_value = o_contact.note();
        o_note.o_entries.push_back(std::move(o_entry));
        o_sections.push_back(std::move(o_note));
    }

    return o_sections;
}

std::string initials_of(const std::string &s_name) {
    std::string s_initials;
    bool b_new_word = true;
    for (const char c_next : s_name) {
        if (' ' == c_next || '\t' == c_next) {
            b_new_word = true;
            continue;
        }
        if (b_new_word && 2 > s_initials.size()) {
            s_initials.push_back(static_cast<char>(
                std::toupper(static_cast<unsigned char>(c_next))));
        }
        b_new_word = false;
    }
    return s_initials;
}

void render_photo(std::ostringstream &s_svg, const vcard::photo_o &o_photo, bool b_include,
                  const std::string &s_initials) {
    constexpr int PHOTO_X = 604;
    constexpr int PHOTO_Y = 12;
    constexpr int PHOTO_SIZE = 126;

    if (!b_include || !o_photo.b_present) {
        s_svg << "  <circle cx=\"" << (PHOTO_X + PHOTO_SIZE / 2) << "\" cy=\""
              << (PHOTO_Y + PHOTO_SIZE / 2) << "\" r=\"" << (PHOTO_SIZE / 2 - 4)
              << "\" fill=\"#3f6f5f\"/>\n";
        if (!s_initials.empty()) {
            s_svg << "  <text x=\"" << (PHOTO_X + PHOTO_SIZE / 2) << "\" y=\""
                  << (PHOTO_Y + PHOTO_SIZE / 2 + 10)
                  << "\" text-anchor=\"middle\" font-family=\"DejaVu Sans, sans-serif\" "
                     "font-size=\"40\" font-weight=\"bold\" fill=\"#ffffff\">"
                  << escape_xml(s_initials) << "</text>\n";
        }
        return;
    }

    if (o_photo.b_embedded && is_embeddable_image(o_photo.s_media_type)) {
        const std::string s_data_uri =
            "data:" + o_photo.s_media_type + ";base64," + util::base64_encode(o_photo.s_bytes);
        s_svg << "  <defs>\n"
              << "    <clipPath id=\"photo-clip\">\n"
              << "      <rect x=\"" << PHOTO_X << "\" y=\"" << PHOTO_Y << "\" width=\""
              << PHOTO_SIZE << "\" height=\"" << PHOTO_SIZE << "\" rx=\"10\"/>\n"
              << "    </clipPath>\n"
              << "  </defs>\n"
              << "  <image x=\"" << PHOTO_X << "\" y=\"" << PHOTO_Y << "\" width=\""
              << PHOTO_SIZE << "\" height=\"" << PHOTO_SIZE
              << "\" preserveAspectRatio=\"xMidYMid slice\" clip-path=\"url(#photo-clip)\" "
                 "href=\""
              << escape_xml(s_data_uri) << "\"/>\n";
        return;
    }

    std::string s_reason = "not embedded";
    if (!o_photo.s_note.empty()) {
        if (std::string::npos != o_photo.s_note.find("URI")) {
            s_reason = "remote URI not fetched";
        } else if (std::string::npos != o_photo.s_note.find("decoded")) {
            s_reason = "image not readable";
        } else if (std::string::npos != o_photo.s_note.find("no value")) {
            s_reason = "no image data";
        }
    }
    s_svg << "  <g>\n"
          << "    <title>" << escape_xml("photo " + s_reason) << "</title>\n"
          << "    <rect x=\"" << PHOTO_X << "\" y=\"" << PHOTO_Y << "\" width=\""
          << PHOTO_SIZE << "\" height=\"" << PHOTO_SIZE
          << "\" rx=\"10\" fill=\"#3f6f5f\" stroke=\"#ffffff\" stroke-opacity=\"0.4\"/>\n"
          << "    <text x=\"" << (PHOTO_X + PHOTO_SIZE / 2) << "\" y=\""
          << (PHOTO_Y + PHOTO_SIZE / 2 - 14)
          << "\" text-anchor=\"middle\" font-family=\"DejaVu Sans, sans-serif\" "
             "font-size=\"12\" fill=\"#ffffff\">photo</text>\n"
          << "    <text x=\"" << (PHOTO_X + PHOTO_SIZE / 2) << "\" y=\""
          << (PHOTO_Y + PHOTO_SIZE / 2 + 4)
          << "\" text-anchor=\"middle\" font-family=\"DejaVu Sans, sans-serif\" "
             "font-size=\"12\" fill=\"#ffffff\">not embedded</text>\n"
          << "    <text x=\"" << (PHOTO_X + PHOTO_SIZE / 2) << "\" y=\""
          << (PHOTO_Y + PHOTO_SIZE / 2 + 22)
          << "\" text-anchor=\"middle\" font-family=\"DejaVu Sans, sans-serif\" "
             "font-size=\"9\" fill=\"#d7e8e0\">"
          << escape_xml(s_reason) << "</text>\n"
          << "  </g>\n";
}

std::string render_text_value(const entry_o &o_entry, int i_y) {
    std::ostringstream s_svg;
    const std::vector<std::string> o_lines = wrap_text(o_entry.s_value, MAX_CHARS);
    for (std::size_t u_index = 0; u_index < o_lines.size(); ++u_index) {
        const int i_line_y = i_y + static_cast<int>(u_index) * 21;
        if (!o_entry.s_href.empty()) {
            s_svg << "  <a href=\"" << escape_xml(o_entry.s_href) << "\">\n"
                  << "    <text x=\"" << MARGIN << "\" y=\"" << i_line_y
                  << "\" font-family=\"DejaVu Sans, sans-serif\" font-size=\"15\" "
                     "fill=\"#1f4fbf\" text-decoration=\"underline\">"
                  << escape_xml(o_lines[u_index]) << "</text>\n"
                  << "  </a>\n";
        } else {
            s_svg << "  <text x=\"" << MARGIN << "\" y=\"" << i_line_y
                  << "\" font-family=\"DejaVu Sans, sans-serif\" font-size=\"15\" "
                     "fill=\"#111827\">"
                  << escape_xml(o_lines[u_index]) << "</text>\n";
        }
    }
    return s_svg.str();
}

}  // namespace

bool is_embeddable_image(const std::string &s_media_type) {
    std::string s_type = s_media_type;
    const std::size_t u_semicolon = s_type.find(';');
    if (std::string::npos != u_semicolon) {
        s_type = s_type.substr(0, u_semicolon);
    }
    std::size_t u_begin = 0;
    std::size_t u_end = s_type.size();
    while (u_begin < u_end && std::isspace(static_cast<unsigned char>(s_type[u_begin]))) {
        ++u_begin;
    }
    while (u_end > u_begin && std::isspace(static_cast<unsigned char>(s_type[u_end - 1]))) {
        --u_end;
    }
    s_type = s_type.substr(u_begin, u_end - u_begin);
    return same_ci(s_type, "image/png") || same_ci(s_type, "image/jpeg") ||
           same_ci(s_type, "image/gif") || same_ci(s_type, "image/webp");
}

std::string render_contact_svg(const vcard::contact_o &o_contact, const options_o &o_options) {
    const std::vector<section_o> o_sections = build_sections(o_contact);

    int i_body_height = 0;
    for (const section_o &o_section : o_sections) {
        i_body_height += 28;
        for (const entry_o &o_entry : o_section.o_entries) {
            i_body_height += entry_height(o_entry);
        }
        i_body_height += 8;
    }
    const int i_height = HEADER_HEIGHT + 34 + i_body_height + 46;

    const std::string s_name = o_contact.formatted_name();
    const std::string s_display_name = s_name.empty() ? "(no name)" : s_name;
    const std::string s_organization = o_contact.organization();
    const std::string s_organization_unit = o_contact.organization_unit();
    const std::string s_title = o_contact.title();

    std::ostringstream s_svg;
    s_svg << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
          << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << WIDTH << "\" height=\""
          << i_height << "\" viewBox=\"0 0 " << WIDTH << ' ' << i_height
          << "\" role=\"img\" aria-label=\"Contact card\">\n"
          << "  <title>" << escape_xml(s_display_name) << "</title>\n"
          << "  <rect x=\"0\" y=\"0\" width=\"" << WIDTH << "\" height=\"" << i_height
          << "\" fill=\"#ffffff\"/>\n"
          << "  <rect x=\"0\" y=\"0\" width=\"" << WIDTH << "\" height=\"" << HEADER_HEIGHT
          << "\" fill=\"#2f5e4e\"/>\n";

    render_photo(s_svg, o_contact.photo(), o_options.b_include_photo, initials_of(s_display_name));

    const int i_name_size = 32 < s_display_name.size() ? 24 : 30;
    s_svg << "  <text x=\"" << MARGIN << "\" y=\"58\" font-family=\"DejaVu Sans, sans-serif\" "
             "font-size=\""
          << i_name_size << "\" font-weight=\"bold\" fill=\"#ffffff\">"
          << escape_xml(s_display_name) << "</text>\n";

    int i_header_y = 88;
    if (!s_organization.empty()) {
        std::string s_line = s_organization;
        if (!s_organization_unit.empty()) {
            s_line += " · " + s_organization_unit;
        }
        s_svg << "  <text x=\"" << MARGIN << "\" y=\"" << i_header_y
              << "\" font-family=\"DejaVu Sans, sans-serif\" font-size=\"16\" "
                 "fill=\"#d7e8e0\">"
              << escape_xml(s_line) << "</text>\n";
        i_header_y += 24;
    }
    if (!s_title.empty()) {
        s_svg << "  <text x=\"" << MARGIN << "\" y=\"" << i_header_y
              << "\" font-family=\"DejaVu Sans, sans-serif\" font-size=\"14\" "
                 "fill=\"#b9d6c9\">"
              << escape_xml(s_title) << "</text>\n";
    }

    int i_y = HEADER_HEIGHT + 34;
    for (const section_o &o_section : o_sections) {
        s_svg << "  <text x=\"" << MARGIN << "\" y=\"" << i_y
              << "\" font-family=\"DejaVu Sans, sans-serif\" font-size=\"12\" "
                 "font-weight=\"bold\" letter-spacing=\"1.5\" fill=\"#6b7280\">"
              << escape_xml(o_section.s_title) << "</text>\n";
        i_y += 22;
        for (const entry_o &o_entry : o_section.o_entries) {
            if (!o_entry.s_label.empty()) {
                s_svg << "  <text x=\"" << MARGIN << "\" y=\"" << i_y
                      << "\" font-family=\"DejaVu Sans, sans-serif\" font-size=\"11\" "
                         "fill=\"#6b7280\">"
                      << escape_xml(o_entry.s_label) << "</text>\n";
                i_y += 15;
            }
            s_svg << render_text_value(o_entry, i_y);
            i_y += static_cast<int>(wrap_text(o_entry.s_value, MAX_CHARS).size()) * 21;
            i_y += 10;
        }
        i_y += 8;
    }

    s_svg << "  <text x=\"" << MARGIN << "\" y=\"" << (i_height - 20)
          << "\" font-family=\"DejaVu Sans, sans-serif\" font-size=\"11\" fill=\"#9ca3af\">"
             "vCard · GNOME VCARD Integration</text>\n"
          << "</svg>\n";
    return s_svg.str();
}

}  // namespace gnome_vcard::render
