//
// vcard-render: draw a contact as a standalone SVG card.
//
// The renderer is the platform-independent half of the project: it reads one
// card and writes one document, without touching the desktop.
//
#include "render/contact_render.h"
#include "util/file_io.h"
#include "vcard/vcard_reader.h"
#include "version/version.h"

#include <cctype>
#include <iostream>
#include <string>

namespace {

constexpr int EXIT_OK = 0;
constexpr int EXIT_ERROR = 1;
constexpr int EXIT_USAGE = 2;

struct options_o {
    std::string s_path;
    std::string s_output;
    int i_index = 1;
    bool b_include_photo = true;
};

void print_usage(std::ostream &s_out) {
    s_out << "usage: vcard-render [--output FILE] [--index N] [--no-photo] FILE.vcf\n"
             "\n"
             "  --output FILE  write the SVG to FILE instead of standard output\n"
             "  --index N      draw the Nth contact in the file, counting from 1\n"
             "  --no-photo     draw the card without the photograph\n";
}

bool parse_positive_int(const std::string &s_text, int &i_value) {
    if (s_text.empty()) {
        return false;
    }
    int i_result = 0;
    for (const char c_next : s_text) {
        if (c_next < '0' || c_next > '9') {
            return false;
        }
        i_result = i_result * 10 + (c_next - '0');
        if (1000000 < i_result) {
            return false;
        }
    }
    if (0 >= i_result) {
        return false;
    }
    i_value = i_result;
    return true;
}

}  // namespace

int main(int i_argument_count, char *o_arguments[]) {
    options_o o_options;
    for (int i_argument = 1; i_argument < i_argument_count; ++i_argument) {
        const std::string s_argument = o_arguments[i_argument];
        if ("--output" == s_argument) {
            if (i_argument + 1 >= i_argument_count) {
                std::cerr << "vcard-render: --output needs a file\n";
                return EXIT_USAGE;
            }
            o_options.s_output = o_arguments[++i_argument];
        } else if ("--index" == s_argument) {
            if (i_argument + 1 >= i_argument_count ||
                !parse_positive_int(o_arguments[i_argument + 1], o_options.i_index)) {
                std::cerr << "vcard-render: --index needs a positive number\n";
                return EXIT_USAGE;
            }
            ++i_argument;
        } else if ("--no-photo" == s_argument) {
            o_options.b_include_photo = false;
        } else if ("--help" == s_argument || "-h" == s_argument) {
            print_usage(std::cout);
            return EXIT_OK;
        } else if ("--version" == s_argument || "-V" == s_argument) {
            std::cout << "vcard-render " << gnome_vcard::version::version_string() << " (build "
                      << gnome_vcard::version::version_build_counter() << ")\n";
            return EXIT_OK;
        } else if ("--" == s_argument.substr(0, 2)) {
            std::cerr << "vcard-render: unknown option: " << s_argument << '\n';
            print_usage(std::cerr);
            return EXIT_USAGE;
        } else if (o_options.s_path.empty()) {
            o_options.s_path = s_argument;
        } else {
            std::cerr << "vcard-render: only one file may be given\n";
            return EXIT_USAGE;
        }
    }

    if (o_options.s_path.empty()) {
        print_usage(std::cerr);
        return EXIT_USAGE;
    }

    gnome_vcard::vcard::file_o o_file;
    std::string s_error;
    if (!gnome_vcard::vcard::read_vcards(o_options.s_path, o_file, s_error)) {
        std::cerr << "vcard-render: " << s_error << '\n';
        return EXIT_ERROR;
    }

    if (static_cast<std::size_t>(o_options.i_index) > o_file.o_contacts.size()) {
        std::cerr << "vcard-render: " << o_options.s_path << " has " << o_file.o_contacts.size()
                  << " contact(s); there is no contact " << o_options.i_index << '\n';
        return EXIT_ERROR;
    }

    const gnome_vcard::vcard::contact_o &o_contact =
        o_file.o_contacts[static_cast<std::size_t>(o_options.i_index) - 1];

    gnome_vcard::render::options_o o_render_options;
    o_render_options.b_include_photo = o_options.b_include_photo;
    const std::string s_svg = gnome_vcard::render::render_contact_svg(o_contact, o_render_options);

    const gnome_vcard::vcard::photo_o o_photo = o_contact.photo();
    if (o_options.b_include_photo && o_photo.b_present && !o_photo.b_embedded &&
        !o_photo.s_note.empty()) {
        std::cerr << "vcard-render: the photo was not embedded: " << o_photo.s_note << '\n';
    }

    if (o_options.s_output.empty()) {
        std::cout << s_svg;
    } else if (!gnome_vcard::util::write_file(o_options.s_output, s_svg, s_error)) {
        std::cerr << "vcard-render: " << s_error << '\n';
        return EXIT_ERROR;
    }

    return EXIT_OK;
}
