#include "vcard/vcard_reader.h"

#include "util/file_io.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <sstream>
#include <utility>

namespace gnome_vcard::vcard {
namespace {

bool is_space(char c_next) {
    return ' ' == c_next || '\t' == c_next || '\r' == c_next || '\n' == c_next;
}

std::string trim(const std::string &s_text) {
    std::size_t u_begin = 0;
    std::size_t u_end = s_text.size();
    while (u_begin < u_end && is_space(s_text[u_begin])) {
        ++u_begin;
    }
    while (u_end > u_begin && is_space(s_text[u_end - 1])) {
        --u_end;
    }
    return s_text.substr(u_begin, u_end - u_begin);
}

std::string upper(const std::string &s_text) {
    std::string s_result = s_text;
    for (char &c_next : s_result) {
        c_next = static_cast<char>(
            std::toupper(static_cast<unsigned char>(c_next)));
    }
    return s_result;
}

bool equals_ci(const std::string &s_left, const std::string &s_right) {
    return upper(s_left) == upper(s_right);
}

bool starts_with_ci(const std::string &s_text, const std::string &s_prefix) {
    if (s_text.size() < s_prefix.size()) {
        return false;
    }
    return equals_ci(s_text.substr(0, s_prefix.size()), s_prefix);
}

bool ends_with_ci(const std::string &s_text, const std::string &s_suffix) {
    if (s_text.size() < s_suffix.size()) {
        return false;
    }
    return equals_ci(s_text.substr(s_text.size() - s_suffix.size()), s_suffix);
}

// Find a character that is neither inside a quoted parameter value nor escaped
// by a backslash.  The first unquoted colon separates the head from the value.
std::size_t find_unquoted(const std::string &s_text, char c_wanted, std::size_t u_from = 0) {
    bool b_quoted = false;
    for (std::size_t u_at = u_from; u_at < s_text.size(); ++u_at) {
        const char c_next = s_text[u_at];
        if ('\\' == c_next) {
            ++u_at;
            continue;
        }
        if ('"' == c_next) {
            b_quoted = !b_quoted;
            continue;
        }
        if (!b_quoted && c_wanted == c_next) {
            return u_at;
        }
    }
    return std::string::npos;
}

std::vector<std::string> split_unquoted(const std::string &s_text, char c_delimiter,
                                        bool b_keep_empty) {
    std::vector<std::string> o_parts;
    std::string s_current;
    bool b_quoted = false;
    for (std::size_t u_at = 0; u_at < s_text.size(); ++u_at) {
        const char c_next = s_text[u_at];
        if ('\\' == c_next && u_at + 1 < s_text.size()) {
            s_current.push_back(c_next);
            s_current.push_back(s_text[u_at + 1]);
            ++u_at;
            continue;
        }
        if ('"' == c_next) {
            b_quoted = !b_quoted;
            s_current.push_back(c_next);
            continue;
        }
        if (!b_quoted && c_delimiter == c_next) {
            if (b_keep_empty || !s_current.empty()) {
                o_parts.push_back(s_current);
            }
            s_current.clear();
            continue;
        }
        s_current.push_back(c_next);
    }
    if (b_keep_empty || !s_current.empty()) {
        o_parts.push_back(s_current);
    }
    return o_parts;
}

// Split a structured value on a delimiter, honouring a backslash escape and a
// quoted section.  The delimiters are removed; the escapes stay for the caller
// to unescape component by component.
std::vector<std::string> split_structured(const std::string &s_text, char c_delimiter) {
    std::vector<std::string> o_parts;
    std::string s_current;
    bool b_quoted = false;
    for (std::size_t u_at = 0; u_at < s_text.size(); ++u_at) {
        const char c_next = s_text[u_at];
        if ('\\' == c_next && u_at + 1 < s_text.size()) {
            s_current.push_back(c_next);
            s_current.push_back(s_text[u_at + 1]);
            ++u_at;
            continue;
        }
        if ('"' == c_next) {
            b_quoted = !b_quoted;
            s_current.push_back(c_next);
            continue;
        }
        if (!b_quoted && c_delimiter == c_next) {
            o_parts.push_back(s_current);
            s_current.clear();
            continue;
        }
        s_current.push_back(c_next);
    }
    o_parts.push_back(s_current);
    return o_parts;
}

std::string strip_quotes(const std::string &s_text) {
    if (2 <= s_text.size() && '"' == s_text.front() && '"' == s_text.back()) {
        return s_text.substr(1, s_text.size() - 2);
    }
    return s_text;
}

// RFC 6868 parameter value encoding: ^n is a newline, ^^ a caret, ^' a quote.
std::string decode_parameter_value(const std::string &s_text) {
    std::string s_result;
    for (std::size_t u_at = 0; u_at < s_text.size(); ++u_at) {
        if ('^' == s_text[u_at] && u_at + 1 < s_text.size()) {
            const char c_next = s_text[u_at + 1];
            if ('n' == c_next || 'N' == c_next) {
                s_result.push_back('\n');
                ++u_at;
                continue;
            }
            if ('^' == c_next) {
                s_result.push_back('^');
                ++u_at;
                continue;
            }
            if ('\'' == c_next) {
                s_result.push_back('"');
                ++u_at;
                continue;
            }
        }
        s_result.push_back(s_text[u_at]);
    }
    return s_result;
}

std::vector<std::string> split_physical_lines(const std::string &s_text) {
    std::vector<std::string> o_lines;
    std::string s_current;
    for (std::size_t u_at = 0; u_at < s_text.size(); ++u_at) {
        const char c_next = s_text[u_at];
        if ('\r' == c_next) {
            o_lines.push_back(s_current);
            s_current.clear();
            if (u_at + 1 < s_text.size() && '\n' == s_text[u_at + 1]) {
                ++u_at;
            }
            continue;
        }
        if ('\n' == c_next) {
            o_lines.push_back(s_current);
            s_current.clear();
            continue;
        }
        s_current.push_back(c_next);
    }
    if (!s_current.empty()) {
        o_lines.push_back(s_current);
    }
    return o_lines;
}

// Unfold a content line split across physical lines.  A quoted-printable soft
// break appears as a trailing "=" when the physical lines are joined, so that
// "=" is dropped as part of the join.
std::vector<std::string> unfold_lines(const std::vector<std::string> &o_lines) {
    std::vector<std::string> o_unfolded;
    for (const std::string &s_line : o_lines) {
        if (!s_line.empty() && (' ' == s_line[0] || '\t' == s_line[0]) &&
            !o_unfolded.empty()) {
            std::string s_continuation = s_line.substr(1);
            std::string &s_previous = o_unfolded.back();
            if (!s_previous.empty() && '=' == s_previous.back()) {
                s_previous.pop_back();
            }
            s_previous += s_continuation;
            continue;
        }
        o_unfolded.push_back(s_line);
    }
    return o_unfolded;
}

int hex_value(char c_next) {
    if ('0' <= c_next && '9' >= c_next) {
        return c_next - '0';
    }
    if ('a' <= c_next && 'f' >= c_next) {
        return c_next - 'a' + 10;
    }
    if ('A' <= c_next && 'F' >= c_next) {
        return c_next - 'A' + 10;
    }
    return -1;
}

std::string quoted_printable_decode(const std::string &s_text) {
    std::string s_result;
    s_result.reserve(s_text.size());
    for (std::size_t u_at = 0; u_at < s_text.size(); ++u_at) {
        const char c_next = s_text[u_at];
        if ('=' != c_next) {
            s_result.push_back(c_next);
            continue;
        }
        if (u_at + 1 >= s_text.size()) {
            // A trailing "=" is a soft break with no continuation.
            break;
        }
        const char c_one = s_text[u_at + 1];
        if ('\r' == c_one || '\n' == c_one) {
            // Soft line break; skip the line ending.
            if ('\r' == c_one && u_at + 2 < s_text.size() && '\n' == s_text[u_at + 2]) {
                ++u_at;
            }
            ++u_at;
            continue;
        }
        if (' ' == c_one || '\t' == c_one) {
            // A soft break that was folded; skip the "=" and the whitespace.
            while (u_at + 1 < s_text.size() &&
                   (' ' == s_text[u_at + 1] || '\t' == s_text[u_at + 1])) {
                ++u_at;
            }
            continue;
        }
        if (u_at + 2 < s_text.size()) {
            const int i_high = hex_value(s_text[u_at + 1]);
            const int i_low = hex_value(s_text[u_at + 2]);
            if (0 <= i_high && 0 <= i_low) {
                s_result.push_back(static_cast<char>((i_high << 4) | i_low));
                u_at += 2;
                continue;
            }
        }
        // Not a valid escape; keep the "=" as written.
        s_result.push_back('=');
    }
    return s_result;
}

void append_utf8(std::string &s_result, unsigned int u_code_point) {
    if (0x80 > u_code_point) {
        s_result.push_back(static_cast<char>(u_code_point));
    } else if (0x800 > u_code_point) {
        s_result.push_back(static_cast<char>(0xc0 | (u_code_point >> 6)));
        s_result.push_back(static_cast<char>(0x80 | (u_code_point & 0x3f)));
    } else {
        s_result.push_back(static_cast<char>(0xe0 | (u_code_point >> 12)));
        s_result.push_back(static_cast<char>(0x80 | ((u_code_point >> 6) & 0x3f)));
        s_result.push_back(static_cast<char>(0x80 | (u_code_point & 0x3f)));
    }
}

// Transcode the handful of charsets a vCard names in practice.  An unknown
// charset is kept as written and named in a warning, because a wrong
// transcoding is worse than a visible replacement character.
std::string transcode_to_utf8(const std::string &s_text, const std::string &s_charset,
                              std::vector<std::string> &o_warnings) {
    const std::string s_upper = upper(trim(s_charset));
    if (s_upper.empty() || "UTF-8" == s_upper || "UTF8" == s_upper) {
        return s_text;
    }
    if ("US-ASCII" == s_upper || "ASCII" == s_upper) {
        return s_text;
    }
    if ("ISO-8859-1" == s_upper || "ISO8859-1" == s_upper || "LATIN1" == s_upper ||
        "LATIN-1" == s_upper) {
        std::string s_result;
        s_result.reserve(s_text.size());
        for (const char c_next : s_text) {
            append_utf8(s_result, static_cast<unsigned int>(static_cast<unsigned char>(c_next)));
        }
        return s_result;
    }
    o_warnings.push_back("unsupported CHARSET=" + trim(s_charset) +
                         "; the value was kept as written");
    return s_text;
}

std::string unescape_text(const std::string &s_text) {
    std::string s_result;
    s_result.reserve(s_text.size());
    for (std::size_t u_at = 0; u_at < s_text.size(); ++u_at) {
        if ('\\' != s_text[u_at] || u_at + 1 >= s_text.size()) {
            s_result.push_back(s_text[u_at]);
            continue;
        }
        const char c_next = s_text[u_at + 1];
        switch (c_next) {
            case 'n':
            case 'N':
                s_result.push_back('\n');
                ++u_at;
                break;
            case ',':
                s_result.push_back(',');
                ++u_at;
                break;
            case ';':
                s_result.push_back(';');
                ++u_at;
                break;
            case '\\':
                s_result.push_back('\\');
                ++u_at;
                break;
            default:
                s_result.push_back('\\');
                break;
        }
    }
    return s_result;
}

void normalise_newlines(std::string &s_text) {
    std::string s_result;
    s_result.reserve(s_text.size());
    for (std::size_t u_at = 0; u_at < s_text.size(); ++u_at) {
        if ('\r' == s_text[u_at]) {
            if (u_at + 1 < s_text.size() && '\n' == s_text[u_at + 1]) {
                ++u_at;
            }
            s_result.push_back('\n');
            continue;
        }
        s_result.push_back(s_text[u_at]);
    }
    s_text = std::move(s_result);
}

std::string percent_decode(const std::string &s_text) {
    std::string s_result;
    for (std::size_t u_at = 0; u_at < s_text.size(); ++u_at) {
        if ('%' == s_text[u_at] && u_at + 2 < s_text.size()) {
            const int i_high = hex_value(s_text[u_at + 1]);
            const int i_low = hex_value(s_text[u_at + 2]);
            if (0 <= i_high && 0 <= i_low) {
                s_result.push_back(static_cast<char>((i_high << 4) | i_low));
                u_at += 2;
                continue;
            }
        }
        s_result.push_back(s_text[u_at]);
    }
    return s_result;
}

std::string sniff_media_type(const std::string &s_bytes) {
    if (4 <= s_bytes.size() &&
        static_cast<unsigned char>(s_bytes[0]) == 0x89 &&
        'P' == s_bytes[1] && 'N' == s_bytes[2] && 'G' == s_bytes[3]) {
        return "image/png";
    }
    if (3 <= s_bytes.size() && static_cast<unsigned char>(s_bytes[0]) == 0xff &&
        static_cast<unsigned char>(s_bytes[1]) == 0xd8 &&
        static_cast<unsigned char>(s_bytes[2]) == 0xff) {
        return "image/jpeg";
    }
    if (6 <= s_bytes.size() && 0 == s_bytes.compare(0, 6, "GIF87a")) {
        return "image/gif";
    }
    if (6 <= s_bytes.size() && 0 == s_bytes.compare(0, 6, "GIF89a")) {
        return "image/gif";
    }
    if (12 <= s_bytes.size() && 0 == s_bytes.compare(0, 4, "RIFF") &&
        0 == s_bytes.compare(8, 4, "WEBP")) {
        return "image/webp";
    }
    return std::string();
}

std::string media_type_from_type_values(const std::vector<std::string> &o_types) {
    for (const std::string &s_type : o_types) {
        const std::string s_upper = upper(s_type);
        if (std::string::npos != s_upper.find('/')) {
            return s_upper;
        }
        if ("PNG" == s_upper) {
            return "image/png";
        }
        if ("JPEG" == s_upper || "JPG" == s_upper) {
            return "image/jpeg";
        }
        if ("GIF" == s_upper) {
            return "image/gif";
        }
        if ("WEBP" == s_upper) {
            return "image/webp";
        }
    }
    return std::string();
}

// Parse a `data:` URI into a media type and the bytes it carries.  Only a
// base64 payload and a percent-encoded text payload are accepted; a URI that
// names a remote resource is kept as a URI and not fetched.
bool parse_data_uri(const std::string &s_uri, std::string &s_media_type, std::string &s_bytes) {
    if (!starts_with_ci(s_uri, "data:")) {
        return false;
    }
    const std::size_t u_comma = s_uri.find(',');
    if (std::string::npos == u_comma) {
        return false;
    }
    const std::string s_metadata = s_uri.substr(5, u_comma - 5);
    const std::string s_payload = s_uri.substr(u_comma + 1);
    const bool b_base64 = ends_with_ci(s_metadata, ";base64");
    std::string s_type = s_metadata;
    if (b_base64) {
        s_type = s_metadata.substr(0, s_metadata.size() - 7);
    }
    if (s_type.empty()) {
        s_type = "text/plain";
    }
    if (b_base64) {
        if (!util::base64_decode(s_payload, s_bytes)) {
            return false;
        }
    } else {
        s_bytes = percent_decode(s_payload);
    }
    s_media_type = s_type;
    return true;
}

struct head_o {
    std::string s_group;
    std::string s_name;
    std::vector<parameter_o> o_parameters;
};

bool parse_head(const std::string &s_head, head_o &o_head, std::string &s_error) {
    const std::vector<std::string> o_segments = split_unquoted(s_head, ';', true);
    if (o_segments.empty()) {
        s_error = "empty property name";
        return false;
    }

    std::string s_name_part = trim(o_segments[0]);
    const std::size_t u_dot = s_name_part.find('.');
    if (std::string::npos != u_dot) {
        o_head.s_group = s_name_part.substr(0, u_dot);
        s_name_part = s_name_part.substr(u_dot + 1);
    }
    o_head.s_name = upper(trim(s_name_part));
    if (o_head.s_name.empty()) {
        s_error = "empty property name";
        return false;
    }

    for (std::size_t u_index = 1; u_index < o_segments.size(); ++u_index) {
        std::string s_segment = trim(o_segments[u_index]);
        if (s_segment.empty()) {
            continue;
        }
        std::string s_parameter_name;
        std::string s_parameter_value;
        const std::size_t u_equals = find_unquoted(s_segment, '=');
        if (std::string::npos == u_equals) {
            // vCard 2.1 spells a type as a bare parameter.
            s_parameter_name = "TYPE";
            s_parameter_value = s_segment;
        } else {
            s_parameter_name = upper(trim(s_segment.substr(0, u_equals)));
            s_parameter_value = s_segment.substr(u_equals + 1);
        }
        if (s_parameter_name.empty()) {
            continue;
        }

        parameter_o o_parameter;
        o_parameter.s_name = s_parameter_name;
        for (const std::string &s_one : split_unquoted(s_parameter_value, ',', true)) {
            const std::string s_decoded =
                decode_parameter_value(strip_quotes(trim(s_one)));
            if ("TYPE" == s_parameter_name) {
                // TYPE="voice,work" is two types; the quotes protect the comma
                // from the parameter splitter, not from the type list.
                for (const std::string &s_type : split_structured(s_decoded, ',')) {
                    const std::string s_trimmed = trim(s_type);
                    if (!s_trimmed.empty()) {
                        o_parameter.o_values.push_back(s_trimmed);
                    }
                }
            } else {
                o_parameter.o_values.push_back(s_decoded);
            }
        }
        o_head.o_parameters.push_back(std::move(o_parameter));
    }
    return true;
}

bool is_binary_property(const std::string &s_name) {
    return "PHOTO" == s_name || "LOGO" == s_name || "SOUND" == s_name ||
           "KEY" == s_name;
}

bool is_structured_property(const std::string &s_name) {
    return "N" == s_name || "ADR" == s_name || "ORG" == s_name;
}

bool is_list_property(const std::string &s_name) {
    return "NICKNAME" == s_name || "CATEGORIES" == s_name;
}

void parse_value(property_o &o_property, const std::string &s_raw_value,
                 std::vector<std::string> &o_warnings) {
    const std::string s_encoding = upper(o_property.s_encoding);
    const std::string s_value_type = upper(o_property.s_value_type);
    const std::string s_charset = o_property.first_parameter("CHARSET");
    const bool b_binary = is_binary_property(o_property.s_name);

    std::string s_text;
    if ("QUOTED-PRINTABLE" == s_encoding || "QP" == s_encoding) {
        s_text = transcode_to_utf8(quoted_printable_decode(s_raw_value), s_charset, o_warnings);
        normalise_newlines(s_text);
    } else if ("BASE64" == s_encoding || "B" == s_encoding) {
        std::string s_bytes;
        if (!util::base64_decode(s_raw_value, s_bytes)) {
            o_warnings.push_back("the " + o_property.s_name + " value is not valid base64");
        }
        o_property.s_bytes = std::move(s_bytes);
        o_property.b_has_bytes = true;
        if (!b_binary) {
            s_text = transcode_to_utf8(o_property.s_bytes, s_charset, o_warnings);
        }
    } else if ("URI" == s_value_type && starts_with_ci(s_raw_value, "data:")) {
        std::string s_media_type;
        std::string s_bytes;
        if (parse_data_uri(s_raw_value, s_media_type, s_bytes)) {
            o_property.s_bytes = std::move(s_bytes);
            o_property.b_has_bytes = true;
            if (o_property.s_media_type.empty()) {
                o_property.s_media_type = s_media_type;
            }
        } else {
            o_warnings.push_back("the " + o_property.s_name + " data URI could not be decoded");
        }
        s_text = s_raw_value;
    } else {
        s_text = transcode_to_utf8(s_raw_value, s_charset, o_warnings);
        normalise_newlines(s_text);
        if ("URI" != s_value_type) {
            s_text = unescape_text(s_text);
        }
    }

    o_property.s_value = std::move(s_text);

    if (o_property.b_has_bytes) {
        if (o_property.s_media_type.empty()) {
            o_property.s_media_type = media_type_from_type_values(o_property.o_types);
        }
        if (o_property.s_media_type.empty()) {
            o_property.s_media_type = sniff_media_type(o_property.s_bytes);
        }
        if (o_property.s_media_type.empty() && b_binary) {
            o_property.s_media_type = "application/octet-stream";
            o_warnings.push_back("the " + o_property.s_name +
                                 " media type is not declared and could not be recognised");
        }
    }

    if (is_structured_property(o_property.s_name)) {
        for (const std::string &s_component : split_structured(o_property.s_value, ';')) {
            o_property.o_components.push_back(unescape_text(s_component));
        }
    } else if (is_list_property(o_property.s_name)) {
        for (const std::string &s_component : split_structured(o_property.s_value, ',')) {
            const std::string s_one = trim(unescape_text(s_component));
            if (!s_one.empty()) {
                o_property.o_components.push_back(s_one);
            }
        }
    }

    if ("PREF" == s_value_type) {
        o_property.b_preferred = true;
    }
    for (const std::string &s_type : o_property.o_types) {
        if ("PREF" == upper(s_type)) {
            o_property.b_preferred = true;
        }
    }
    const std::string s_pref = o_property.first_parameter("PREF");
    if (!s_pref.empty()) {
        o_property.b_preferred = true;
        o_property.i_preference = std::atoi(s_pref.c_str());
        if (0 == o_property.i_preference) {
            o_property.i_preference = 1;
        }
    }
}

void finalize_contact(contact_o &o_contact) {
    const property_o *o_version = o_contact.first_property("VERSION");
    if (nullptr == o_version || o_version->s_value.empty()) {
        o_contact.s_version = "3.0";
        o_contact.o_warnings.push_back("VERSION is missing; the card was read as 3.0");
    } else {
        o_contact.s_version = o_version->s_value;
        if ("2.1" != o_contact.s_version && "3.0" != o_contact.s_version &&
            "4.0" != o_contact.s_version) {
            o_contact.o_warnings.push_back("unrecognised VERSION " + o_contact.s_version +
                                           "; the card was read with the 4.0 rules");
        }
    }

    const std::size_t u_fn_count = o_contact.properties_named("FN").size();
    if (0 == u_fn_count) {
        o_contact.o_warnings.push_back("FN is missing; the display name was derived from N");
    } else if (1 < u_fn_count) {
        o_contact.o_warnings.push_back("the card carries " + std::to_string(u_fn_count) +
                                       " FN properties; the first is shown");
    }

    if (0 == o_contact.properties_named("N").size() && "4.0" != o_contact.s_version) {
        o_contact.o_warnings.push_back("N is missing");
    }

    if (1 >= o_contact.o_properties.size()) {
        o_contact.o_warnings.push_back("the card carries no contact properties");
    }
}

}  // namespace

bool parse_vcards(const std::string &s_text, const std::string &s_path, file_o &o_file,
                  std::string &s_error) {
    o_file = file_o();
    o_file.s_path = s_path;
    o_file.u_size = s_text.size();

    if (std::string::npos != s_text.find('\0')) {
        s_error = s_path + " contains a NUL byte and is not a vCard";
        return false;
    }

    std::string s_document = s_text;
    if (3 <= s_document.size() && static_cast<unsigned char>(s_document[0]) == 0xef &&
        static_cast<unsigned char>(s_document[1]) == 0xbb &&
        static_cast<unsigned char>(s_document[2]) == 0xbf) {
        s_document.erase(0, 3);
    }

    const std::vector<std::string> o_lines = unfold_lines(split_physical_lines(s_document));

    contact_o o_contact;
    bool b_in_card = false;
    bool b_saw_begin = false;
    bool b_saw_end = false;

    for (const std::string &s_line_raw : o_lines) {
        const std::string s_line = trim(s_line_raw);
        if (s_line.empty()) {
            continue;
        }

        if (equals_ci(s_line, "BEGIN:VCARD")) {
            if (b_in_card) {
                s_error = s_path + ": BEGIN:VCARD inside a card";
                return false;
            }
            b_in_card = true;
            b_saw_begin = true;
            o_contact = contact_o();
            continue;
        }
        if (equals_ci(s_line, "END:VCARD")) {
            if (!b_in_card) {
                s_error = s_path + ": END:VCARD without a matching BEGIN:VCARD";
                return false;
            }
            finalize_contact(o_contact);
            o_file.o_contacts.push_back(std::move(o_contact));
            o_contact = contact_o();
            b_in_card = false;
            b_saw_end = true;
            continue;
        }
        if (starts_with_ci(s_line, "BEGIN:")) {
            o_file.o_warnings.push_back("unsupported line outside the vCard structure: " +
                                        s_line.substr(0, 60));
            continue;
        }

        if (!b_in_card) {
            o_file.o_warnings.push_back("text outside a card was ignored: " +
                                        s_line.substr(0, 60));
            continue;
        }

        const std::size_t u_colon = find_unquoted(s_line_raw, ':');
        if (std::string::npos == u_colon) {
            o_contact.o_warnings.push_back("a malformed line has no colon and was skipped: " +
                                           s_line.substr(0, 60));
            continue;
        }

        head_o o_head;
        std::string s_head_error;
        if (!parse_head(s_line_raw.substr(0, u_colon), o_head, s_head_error)) {
            o_contact.o_warnings.push_back("a malformed property was skipped: " + s_head_error);
            continue;
        }

        property_o o_property;
        o_property.s_group = o_head.s_group;
        o_property.s_name = o_head.s_name;
        o_property.o_parameters = std::move(o_head.o_parameters);
        o_property.s_encoding = o_property.first_parameter("ENCODING");
        o_property.s_value_type = o_property.first_parameter("VALUE");
        o_property.s_language = o_property.first_parameter("LANGUAGE");
        o_property.s_media_type = o_property.first_parameter("MEDIATYPE");
        for (const std::string &s_type : o_property.parameter_values("TYPE")) {
            const std::string s_upper = upper(s_type);
            if (!s_upper.empty()) {
                o_property.o_types.push_back(s_upper);
            }
        }

        parse_value(o_property, s_line_raw.substr(u_colon + 1), o_contact.o_warnings);
        o_contact.o_properties.push_back(std::move(o_property));
    }

    if (b_in_card) {
        s_error = s_path + ": BEGIN:VCARD without a matching END:VCARD";
        return false;
    }
    if (!b_saw_begin) {
        s_error = s_path + " is not a vCard: no BEGIN:VCARD was found";
        return false;
    }
    if (!b_saw_end) {
        s_error = s_path + " is not a vCard: no END:VCARD was found";
        return false;
    }
    if (o_file.o_contacts.empty()) {
        s_error = s_path + " contains no vCard";
        return false;
    }

    return true;
}

bool read_vcards(const std::string &s_path, file_o &o_file, std::string &s_error) {
    std::string s_text;
    if (!util::read_file(s_path, s_text, s_error)) {
        return false;
    }
    return parse_vcards(s_text, s_path, o_file, s_error);
}

// --- model accessors ---------------------------------------------------------

bool property_o::has_parameter(const std::string &s_parameter_name) const {
    for (const parameter_o &o_parameter : o_parameters) {
        if (equals_ci(o_parameter.s_name, s_parameter_name)) {
            return true;
        }
    }
    return false;
}

std::vector<std::string> property_o::parameter_values(
    const std::string &s_parameter_name) const {
    std::vector<std::string> o_values;
    for (const parameter_o &o_parameter : o_parameters) {
        if (equals_ci(o_parameter.s_name, s_parameter_name)) {
            o_values.insert(o_values.end(), o_parameter.o_values.begin(),
                            o_parameter.o_values.end());
        }
    }
    return o_values;
}

std::string property_o::first_parameter(const std::string &s_parameter_name) const {
    for (const parameter_o &o_parameter : o_parameters) {
        if (equals_ci(o_parameter.s_name, s_parameter_name) && !o_parameter.o_values.empty()) {
            return o_parameter.o_values.front();
        }
    }
    return std::string();
}

bool property_o::has_type(const std::string &s_type) const {
    for (const std::string &s_one : o_types) {
        if (equals_ci(s_one, s_type)) {
            return true;
        }
    }
    return false;
}

std::string property_o::types_text() const {
    std::string s_result;
    for (const std::string &s_one : o_types) {
        if (!s_result.empty()) {
            s_result += ", ";
        }
        s_result += s_one;
    }
    return s_result;
}

bool property_o::is_uri() const {
    return equals_ci(s_value_type, "uri");
}

bool address_o::is_empty() const {
    return s_post_office_box.empty() && s_extended_address.empty() && s_street.empty() &&
           s_locality.empty() && s_region.empty() && s_postal_code.empty() &&
           s_country.empty();
}

std::vector<std::string> address_o::formatted_lines() const {
    std::vector<std::string> o_lines;
    if (!s_post_office_box.empty()) {
        o_lines.push_back(s_post_office_box);
    }
    if (!s_extended_address.empty()) {
        o_lines.push_back(s_extended_address);
    }
    if (!s_street.empty()) {
        o_lines.push_back(s_street);
    }
    std::string s_city_line;
    if (!s_locality.empty()) {
        s_city_line += s_locality;
    }
    if (!s_region.empty()) {
        if (!s_city_line.empty()) {
            s_city_line += ", ";
        }
        s_city_line += s_region;
    }
    if (!s_postal_code.empty()) {
        if (!s_city_line.empty()) {
            s_city_line += " ";
        }
        s_city_line += s_postal_code;
    }
    if (!s_city_line.empty()) {
        o_lines.push_back(s_city_line);
    }
    if (!s_country.empty()) {
        o_lines.push_back(s_country);
    }
    if (o_lines.empty() && !s_label.empty()) {
        o_lines.push_back(s_label);
    }
    return o_lines;
}

std::string address_o::formatted() const {
    std::string s_result;
    for (const std::string &s_line : formatted_lines()) {
        if (!s_result.empty()) {
            s_result += ", ";
        }
        s_result += s_line;
    }
    return s_result;
}

std::vector<property_o> contact_o::properties_named(const std::string &s_name) const {
    std::vector<property_o> o_result;
    for (const property_o &o_property : o_properties) {
        if (equals_ci(o_property.s_name, s_name)) {
            o_result.push_back(o_property);
        }
    }
    return o_result;
}

const property_o *contact_o::first_property(const std::string &s_name) const {
    for (const property_o &o_property : o_properties) {
        if (equals_ci(o_property.s_name, s_name)) {
            return &o_property;
        }
    }
    return nullptr;
}

std::string contact_o::first_text(const std::string &s_name) const {
    const property_o *o_property = first_property(s_name);
    return (nullptr == o_property) ? std::string() : o_property->s_value;
}

std::vector<std::string> contact_o::name_components() const {
    const property_o *o_property = first_property("N");
    if (nullptr == o_property) {
        return std::vector<std::string>();
    }
    return o_property->o_components;
}

std::string contact_o::formatted_name() const {
    for (const property_o &o_property : o_properties) {
        if (equals_ci(o_property.s_name, "FN") && !o_property.s_value.empty()) {
            return o_property.s_value;
        }
    }

    const std::vector<std::string> o_parts = name_components();
    const auto part = [&o_parts](std::size_t u_index) -> std::string {
        return (u_index < o_parts.size()) ? o_parts[u_index] : std::string();
    };

    std::string s_result;
    const auto append = [&s_result](const std::string &s_part) {
        if (s_part.empty()) {
            return;
        }
        if (!s_result.empty()) {
            s_result += " ";
        }
        s_result += s_part;
    };
    append(part(3));  // prefix
    append(part(1));  // given
    append(part(2));  // additional
    append(part(0));  // family
    append(part(4));  // suffix
    if (s_result.empty()) {
        s_result = organization();
    }
    return s_result;
}

std::string contact_o::organization() const {
    const property_o *o_property = first_property("ORG");
    if (nullptr == o_property) {
        return std::string();
    }
    if (!o_property->o_components.empty()) {
        return o_property->o_components[0];
    }
    return o_property->s_value;
}

std::string contact_o::organization_unit() const {
    const property_o *o_property = first_property("ORG");
    if (nullptr == o_property || 2 > o_property->o_components.size()) {
        return std::string();
    }
    return o_property->o_components[1];
}

std::string contact_o::title() const {
    return first_text("TITLE");
}

std::string contact_o::role() const {
    return first_text("ROLE");
}

std::string contact_o::note() const {
    return first_text("NOTE");
}

std::string contact_o::birthday() const {
    return first_text("BDAY");
}

std::string contact_o::anniversary() const {
    return first_text("ANNIVERSARY");
}

std::vector<std::string> contact_o::nicknames() const {
    const property_o *o_property = first_property("NICKNAME");
    if (nullptr == o_property) {
        return std::vector<std::string>();
    }
    if (!o_property->o_components.empty()) {
        return o_property->o_components;
    }
    if (o_property->s_value.empty()) {
        return std::vector<std::string>();
    }
    return split_structured(o_property->s_value, ',');
}

std::vector<property_o> contact_o::telephones() const {
    return properties_named("TEL");
}

std::vector<property_o> contact_o::emails() const {
    return properties_named("EMAIL");
}

std::vector<property_o> contact_o::urls() const {
    return properties_named("URL");
}

std::vector<property_o> contact_o::impps() const {
    return properties_named("IMPP");
}

std::vector<address_o> contact_o::addresses() const {
    std::vector<address_o> o_addresses;
    const std::string s_fallback_label = first_text("LABEL");
    for (const property_o &o_property : o_properties) {
        if (!equals_ci(o_property.s_name, "ADR")) {
            continue;
        }
        address_o o_address;
        o_address.o_types = o_property.o_types;
        o_address.b_preferred = o_property.b_preferred;
        o_address.s_label = s_fallback_label;
        const std::vector<std::string> &o_parts = o_property.o_components;
        if (0 < o_parts.size()) {
            o_address.s_post_office_box = o_parts[0];
        }
        if (1 < o_parts.size()) {
            o_address.s_extended_address = o_parts[1];
        }
        if (2 < o_parts.size()) {
            o_address.s_street = o_parts[2];
        }
        if (3 < o_parts.size()) {
            o_address.s_locality = o_parts[3];
        }
        if (4 < o_parts.size()) {
            o_address.s_region = o_parts[4];
        }
        if (5 < o_parts.size()) {
            o_address.s_postal_code = o_parts[5];
        }
        if (6 < o_parts.size()) {
            o_address.s_country = o_parts[6];
        }
        o_addresses.push_back(std::move(o_address));
    }
    return o_addresses;
}

photo_o contact_o::photo() const {
    const property_o *o_property = first_property("PHOTO");
    if (nullptr == o_property) {
        o_property = first_property("LOGO");
    }
    if (nullptr == o_property) {
        return photo_o();
    }

    photo_o o_photo;
    o_photo.b_present = true;
    if (o_property->b_has_bytes && !o_property->s_bytes.empty()) {
        o_photo.b_embedded = true;
        o_photo.s_media_type = o_property->s_media_type;
        o_photo.s_bytes = o_property->s_bytes;
        return o_photo;
    }

    if (starts_with_ci(o_property->s_value, "data:")) {
        o_photo.s_note = "the photo data URI could not be decoded";
        return o_photo;
    }
    if (starts_with_ci(o_property->s_value, "http://") ||
        starts_with_ci(o_property->s_value, "https://") ||
        starts_with_ci(o_property->s_value, "file:")) {
        o_photo.s_uri = o_property->s_value;
        o_photo.s_note = "the photo is a URI and was not fetched";
        return o_photo;
    }
    if (o_property->s_value.empty()) {
        o_photo.s_note = "the photo property has no value";
    } else {
        o_photo.s_note = "the photo value could not be decoded as an image";
    }
    return o_photo;
}

std::size_t contact_o::property_count() const {
    return o_properties.size();
}

}  // namespace gnome_vcard::vcard
