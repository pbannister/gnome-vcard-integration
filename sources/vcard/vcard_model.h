#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace gnome_vcard::vcard {

// One parameter on a content line, such as TYPE with the values work and voice.
struct parameter_o {
    std::string s_name;
    std::vector<std::string> o_values;
};

// One property on a contact.  s_value is the decoded text; o_components holds
// the decoded structured parts for N, ADR, ORG, and a list-valued property such
// as NICKNAME; s_bytes holds the decoded binary value of a photo or logo.
struct property_o {
    std::string s_group;
    std::string s_name;
    std::vector<parameter_o> o_parameters;
    std::string s_value;
    std::vector<std::string> o_components;
    std::string s_bytes;
    bool b_has_bytes = false;
    std::string s_media_type;
    std::string s_value_type;
    std::string s_encoding;
    std::vector<std::string> o_types;
    std::string s_language;
    bool b_preferred = false;
    int i_preference = 0;

    bool has_parameter(const std::string &s_name) const;
    std::vector<std::string> parameter_values(const std::string &s_name) const;
    std::string first_parameter(const std::string &s_name) const;
    bool has_type(const std::string &s_type) const;
    std::string types_text() const;
    bool is_uri() const;
};

// A postal address, in the seven ADR positions.
struct address_o {
    std::string s_post_office_box;
    std::string s_extended_address;
    std::string s_street;
    std::string s_locality;
    std::string s_region;
    std::string s_postal_code;
    std::string s_country;
    std::string s_label;
    std::vector<std::string> o_types;
    bool b_preferred = false;

    bool is_empty() const;
    std::vector<std::string> formatted_lines() const;
    std::string formatted() const;
};

// What the renderer needs to know about a photo without reopening the file.
struct photo_o {
    bool b_present = false;
    bool b_embedded = false;
    std::string s_media_type;
    std::string s_bytes;
    std::string s_uri;
    std::string s_note;
};

// One contact: the properties, the warnings raised while reading it, and the
// accessors the renderer and the tools use.
struct contact_o {
    std::string s_version;
    std::vector<property_o> o_properties;
    std::vector<std::string> o_warnings;

    std::vector<property_o> properties_named(const std::string &s_name) const;
    const property_o *first_property(const std::string &s_name) const;
    std::string first_text(const std::string &s_name) const;

    std::string formatted_name() const;
    std::vector<std::string> name_components() const;
    std::string organization() const;
    std::string organization_unit() const;
    std::string title() const;
    std::string role() const;
    std::string note() const;
    std::string birthday() const;
    std::string anniversary() const;
    std::vector<std::string> nicknames() const;

    std::vector<property_o> telephones() const;
    std::vector<property_o> emails() const;
    std::vector<property_o> urls() const;
    std::vector<property_o> impps() const;
    std::vector<address_o> addresses() const;
    photo_o photo() const;

    std::size_t property_count() const;
};

// One file: the contacts it held, in file order, and the file-level warnings.
struct file_o {
    std::string s_path;
    std::size_t u_size = 0;
    std::vector<contact_o> o_contacts;
    std::vector<std::string> o_warnings;
};

}  // namespace gnome_vcard::vcard
