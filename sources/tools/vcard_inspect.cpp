//
// vcard-inspect: report what a vCard file contains.
//
// The report is the platform-independent part of this project: it reads the
// cards and their properties without touching the desktop.  `--json` prints the
// same facts for another program, and `--properties` adds every content line.
//
#include "util/file_io.h"
#include "vcard/vcard_reader.h"
#include "version/version.h"

#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

constexpr int EXIT_OK = 0;
constexpr int EXIT_ERROR = 1;
constexpr int EXIT_USAGE = 2;

struct options_o {
    std::string s_path;
    bool b_json = false;
    bool b_properties = false;
};

void print_usage(std::ostream &s_out) {
    s_out << "usage: vcard-inspect [--json] [--properties] FILE.vcf\n"
             "\n"
             "  --json        print the cards as JSON instead of a report\n"
             "  --properties  also list every property and its parameters\n";
}

std::string flatten(const std::string &s_text) {
    std::string s_result;
    s_result.reserve(s_text.size());
    for (const char c_next : s_text) {
        if ('\n' == c_next) {
            s_result += " \\n ";
            continue;
        }
        if ('\r' == c_next) {
            continue;
        }
        s_result.push_back(c_next);
    }
    return s_result;
}

std::string property_label(const gnome_vcard::vcard::property_o &o_property) {
    std::string s_label = o_property.s_name;
    for (const gnome_vcard::vcard::parameter_o &o_parameter : o_property.o_parameters) {
        s_label += ';';
        s_label += o_parameter.s_name;
        if (!o_parameter.o_values.empty()) {
            s_label += '=';
            for (std::size_t u_index = 0; u_index < o_parameter.o_values.size(); ++u_index) {
                if (0 < u_index) {
                    s_label += ',';
                }
                s_label += o_parameter.o_values[u_index];
            }
        }
    }
    return s_label;
}

std::string types_and_preference(const gnome_vcard::vcard::property_o &o_property) {
    std::string s_result = o_property.types_text();
    if (o_property.b_preferred) {
        if (!s_result.empty()) {
            s_result += ", ";
        }
        s_result += "preferred";
    }
    return s_result;
}

void print_property(const gnome_vcard::vcard::property_o &o_property) {
    std::cout << "  property " << property_label(o_property) << '\n';
    if (!o_property.s_group.empty()) {
        std::cout << "    group: " << o_property.s_group << '\n';
    }
    if (o_property.b_has_bytes) {
        std::cout << "    value: <binary: " << o_property.s_bytes.size() << " bytes";
        if (!o_property.s_media_type.empty()) {
            std::cout << ", " << o_property.s_media_type;
        }
        std::cout << ">\n";
    } else {
        std::cout << "    value: " << flatten(o_property.s_value) << '\n';
    }
    if (!o_property.o_types.empty()) {
        std::cout << "    types: " << o_property.types_text() << '\n';
    }
}

void print_photo(const gnome_vcard::vcard::contact_o &o_contact) {
    const gnome_vcard::vcard::photo_o o_photo = o_contact.photo();
    if (!o_photo.b_present) {
        return;
    }
    if (o_photo.b_embedded) {
        std::cout << "  photo: " << o_photo.s_media_type << " (" << o_photo.s_bytes.size()
                  << " bytes, embedded)\n";
        return;
    }
    std::cout << "  photo: not embedded";
    if (!o_photo.s_uri.empty()) {
        std::cout << " (" << o_photo.s_uri << ')';
    }
    if (!o_photo.s_note.empty()) {
        std::cout << ": " << o_photo.s_note;
    }
    std::cout << '\n';
}

void print_report(const gnome_vcard::vcard::file_o &o_file, bool b_properties) {
    std::cout << "file: " << o_file.s_path << '\n';
    std::cout << "size: " << o_file.u_size << " bytes\n";
    std::cout << "cards: " << o_file.o_contacts.size() << '\n';
    for (const std::string &s_warning : o_file.o_warnings) {
        std::cout << "warning: " << s_warning << '\n';
    }

    for (std::size_t u_index = 0; u_index < o_file.o_contacts.size(); ++u_index) {
        const gnome_vcard::vcard::contact_o &o_contact = o_file.o_contacts[u_index];
        std::cout << "card " << (u_index + 1) << ":\n";
        std::cout << "  version: " << o_contact.s_version << '\n';
        std::cout << "  formatted name: " << flatten(o_contact.formatted_name()) << '\n';

        const std::vector<std::string> o_name = o_contact.name_components();
        if (!o_name.empty()) {
            std::cout << "  name:";
            for (const std::string &s_part : o_name) {
                std::cout << ' ' << s_part << ';';
            }
            std::cout << '\n';
        }

        const auto print_line = [](const char *s_key, const std::string &s_value) {
            if (!s_value.empty()) {
                std::cout << "  " << s_key << ": " << flatten(s_value) << '\n';
            }
        };
        print_line("organization", o_contact.organization());
        print_line("organization unit", o_contact.organization_unit());
        print_line("title", o_contact.title());
        print_line("role", o_contact.role());

        for (const std::string &s_nickname : o_contact.nicknames()) {
            std::cout << "  nickname: " << s_nickname << '\n';
        }
        print_line("birthday", o_contact.birthday());
        print_line("anniversary", o_contact.anniversary());

        for (const gnome_vcard::vcard::property_o &o_property : o_contact.emails()) {
            std::cout << "  email: " << flatten(o_property.s_value);
            const std::string s_types = types_and_preference(o_property);
            if (!s_types.empty()) {
                std::cout << " (" << s_types << ')';
            }
            std::cout << '\n';
        }
        for (const gnome_vcard::vcard::property_o &o_property : o_contact.telephones()) {
            std::cout << "  telephone: " << flatten(o_property.s_value);
            const std::string s_types = types_and_preference(o_property);
            if (!s_types.empty()) {
                std::cout << " (" << s_types << ')';
            }
            std::cout << '\n';
        }
        for (const gnome_vcard::vcard::address_o &o_address : o_contact.addresses()) {
            if (o_address.is_empty()) {
                continue;
            }
            std::cout << "  address: " << flatten(o_address.formatted());
            if (!o_address.o_types.empty()) {
                std::cout << " (";
                bool b_wrote_type = false;
                for (const std::string &s_type : o_address.o_types) {
                    if ("PREF" == s_type) {
                        continue;
                    }
                    if (b_wrote_type) {
                        std::cout << ", ";
                    }
                    std::cout << s_type;
                    b_wrote_type = true;
                }
                if (o_address.b_preferred) {
                    std::cout << ", preferred";
                }
                std::cout << ')';
            }
            std::cout << '\n';
        }
        for (const gnome_vcard::vcard::property_o &o_property : o_contact.urls()) {
            std::cout << "  url: " << o_property.s_value << '\n';
        }
        for (const gnome_vcard::vcard::property_o &o_property : o_contact.impps()) {
            std::cout << "  impp: " << o_property.s_value;
            const std::string s_types = types_and_preference(o_property);
            if (!s_types.empty()) {
                std::cout << " (" << s_types << ')';
            }
            std::cout << '\n';
        }
        print_line("note", o_contact.note());
        print_photo(o_contact);

        std::cout << "  properties: " << o_contact.property_count() << '\n';
        for (const std::string &s_warning : o_contact.o_warnings) {
            std::cout << "  warning: " << s_warning << '\n';
        }

        if (b_properties) {
            for (const gnome_vcard::vcard::property_o &o_property : o_contact.o_properties) {
                print_property(o_property);
            }
        }
    }
}

std::string json_escape(const std::string &s_text) {
    std::string s_result = "\"";
    for (const char c_next : s_text) {
        const unsigned char u_next = static_cast<unsigned char>(c_next);
        switch (c_next) {
            case '"':
                s_result += "\\\"";
                break;
            case '\\':
                s_result += "\\\\";
                break;
            case '\n':
                s_result += "\\n";
                break;
            case '\r':
                s_result += "\\r";
                break;
            case '\t':
                s_result += "\\t";
                break;
            default:
                if (0x20 > u_next) {
                    char o_buffer[8];
                    std::snprintf(o_buffer, sizeof(o_buffer), "\\u%04x",
                                  static_cast<unsigned int>(u_next));
                    s_result += o_buffer;
                } else {
                    s_result.push_back(c_next);
                }
                break;
        }
    }
    s_result.push_back('"');
    return s_result;
}

std::string json_string_array(const std::vector<std::string> &o_values) {
    std::string s_result = "[";
    for (std::size_t u_index = 0; u_index < o_values.size(); ++u_index) {
        if (0 < u_index) {
            s_result += ",";
        }
        s_result += json_escape(o_values[u_index]);
    }
    s_result += "]";
    return s_result;
}

std::string json_property(const gnome_vcard::vcard::property_o &o_property) {
    std::ostringstream s_json;
    s_json << "{";
    s_json << "\"group\":" << json_escape(o_property.s_group);
    s_json << ",\"name\":" << json_escape(o_property.s_name);
    s_json << ",\"parameters\":[";
    for (std::size_t u_index = 0; u_index < o_property.o_parameters.size(); ++u_index) {
        if (0 < u_index) {
            s_json << ",";
        }
        s_json << "{\"name\":" << json_escape(o_property.o_parameters[u_index].s_name)
               << ",\"values\":"
               << json_string_array(o_property.o_parameters[u_index].o_values) << "}";
    }
    s_json << "]";
    s_json << ",\"value\":" << json_escape(o_property.s_value);
    s_json << ",\"types\":" << json_string_array(o_property.o_types);
    s_json << ",\"preferred\":" << (o_property.b_preferred ? "true" : "false");
    s_json << ",\"bytes\":" << o_property.s_bytes.size();
    if (!o_property.s_media_type.empty()) {
        s_json << ",\"mediaType\":" << json_escape(o_property.s_media_type);
    }
    s_json << "}";
    return s_json.str();
}

std::string json_properties(const std::vector<gnome_vcard::vcard::property_o> &o_properties) {
    std::string s_json = "[";
    for (std::size_t u_index = 0; u_index < o_properties.size(); ++u_index) {
        if (0 < u_index) {
            s_json += ",";
        }
        s_json += json_property(o_properties[u_index]);
    }
    s_json += "]";
    return s_json;
}

std::string json_contact(const gnome_vcard::vcard::contact_o &o_contact) {
    std::ostringstream s_json;
    s_json << "{";
    s_json << "\"version\":" << json_escape(o_contact.s_version);
    s_json << ",\"formattedName\":" << json_escape(o_contact.formatted_name());
    s_json << ",\"nameComponents\":" << json_string_array(o_contact.name_components());
    s_json << ",\"organization\":" << json_escape(o_contact.organization());
    s_json << ",\"organizationUnit\":" << json_escape(o_contact.organization_unit());
    s_json << ",\"title\":" << json_escape(o_contact.title());
    s_json << ",\"role\":" << json_escape(o_contact.role());
    s_json << ",\"nicknames\":" << json_string_array(o_contact.nicknames());
    s_json << ",\"birthday\":" << json_escape(o_contact.birthday());
    s_json << ",\"anniversary\":" << json_escape(o_contact.anniversary());
    s_json << ",\"note\":" << json_escape(o_contact.note());

    const auto properties = [](const std::vector<gnome_vcard::vcard::property_o> &o_values) {
        return json_properties(o_values);
    };
    s_json << ",\"telephones\":" << properties(o_contact.telephones());
    s_json << ",\"emails\":" << properties(o_contact.emails());
    s_json << ",\"urls\":" << properties(o_contact.urls());
    s_json << ",\"impps\":" << properties(o_contact.impps());

    s_json << ",\"addresses\":[";
    const std::vector<gnome_vcard::vcard::address_o> o_addresses = o_contact.addresses();
    for (std::size_t u_index = 0; u_index < o_addresses.size(); ++u_index) {
        if (0 < u_index) {
            s_json << ",";
        }
        const gnome_vcard::vcard::address_o &o_address = o_addresses[u_index];
        s_json << "{\"components\":{";
        s_json << "\"postOfficeBox\":" << json_escape(o_address.s_post_office_box);
        s_json << ",\"extendedAddress\":" << json_escape(o_address.s_extended_address);
        s_json << ",\"street\":" << json_escape(o_address.s_street);
        s_json << ",\"locality\":" << json_escape(o_address.s_locality);
        s_json << ",\"region\":" << json_escape(o_address.s_region);
        s_json << ",\"postalCode\":" << json_escape(o_address.s_postal_code);
        s_json << ",\"country\":" << json_escape(o_address.s_country);
        s_json << "},\"formatted\":" << json_escape(o_address.formatted());
        s_json << ",\"label\":" << json_escape(o_address.s_label);
        s_json << ",\"types\":" << json_string_array(o_address.o_types);
        s_json << ",\"preferred\":" << (o_address.b_preferred ? "true" : "false");
        s_json << "}";
    }
    s_json << "]";

    const gnome_vcard::vcard::photo_o o_photo = o_contact.photo();
    s_json << ",\"photo\":{";
    s_json << "\"present\":" << (o_photo.b_present ? "true" : "false");
    s_json << ",\"embedded\":" << (o_photo.b_embedded ? "true" : "false");
    s_json << ",\"mediaType\":" << json_escape(o_photo.s_media_type);
    s_json << ",\"sizeBytes\":" << o_photo.s_bytes.size();
    s_json << ",\"uri\":" << json_escape(o_photo.s_uri);
    s_json << ",\"note\":" << json_escape(o_photo.s_note);
    s_json << "}";

    s_json << ",\"properties\":" << json_properties(o_contact.o_properties);
    s_json << ",\"warnings\":" << json_string_array(o_contact.o_warnings);
    s_json << "}";
    return s_json.str();
}

std::string json_report(const gnome_vcard::vcard::file_o &o_file) {
    std::ostringstream s_json;
    s_json << "{";
    s_json << "\"path\":" << json_escape(o_file.s_path);
    s_json << ",\"sizeBytes\":" << o_file.u_size;
    s_json << ",\"warnings\":" << json_string_array(o_file.o_warnings);
    s_json << ",\"cards\":[";
    for (std::size_t u_index = 0; u_index < o_file.o_contacts.size(); ++u_index) {
        if (0 < u_index) {
            s_json << ",";
        }
        s_json << json_contact(o_file.o_contacts[u_index]);
    }
    s_json << "]}";
    return s_json.str();
}

}  // namespace

int main(int i_argument_count, char *o_arguments[]) {
    options_o o_options;
    for (int i_argument = 1; i_argument < i_argument_count; ++i_argument) {
        const std::string s_argument = o_arguments[i_argument];
        if ("--json" == s_argument) {
            o_options.b_json = true;
        } else if ("--properties" == s_argument) {
            o_options.b_properties = true;
        } else if ("--help" == s_argument || "-h" == s_argument) {
            print_usage(std::cout);
            return EXIT_OK;
        } else if ("--version" == s_argument || "-V" == s_argument) {
            std::cout << "vcard-inspect " << gnome_vcard::version::version_string() << " (build "
                      << gnome_vcard::version::version_build_counter() << ")\n";
            return EXIT_OK;
        } else if ("--" == s_argument.substr(0, 2)) {
            std::cerr << "vcard-inspect: unknown option: " << s_argument << '\n';
            print_usage(std::cerr);
            return EXIT_USAGE;
        } else if (o_options.s_path.empty()) {
            o_options.s_path = s_argument;
        } else {
            std::cerr << "vcard-inspect: only one file may be given\n";
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
        std::cerr << "vcard-inspect: " << s_error << '\n';
        return EXIT_ERROR;
    }

    if (o_options.b_json) {
        std::cout << json_report(o_file) << '\n';
    } else {
        print_report(o_file, o_options.b_properties);
    }

    // A card that is readable but incomplete is a finding, not a failure to
    // read; the command still exits 0 so a caller can read the report.
    return EXIT_OK;
}
