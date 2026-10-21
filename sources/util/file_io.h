#pragma once

#include <string>

namespace gnome_vcard::util {

// Read a whole file into memory.  A vCard is small, and the reader needs the
// whole document to unfold a line that spans a physical line break, so the file
// is held rather than streamed.
bool read_file(const std::string &s_path, std::string &s_data, std::string &s_error);

// Write a whole file, creating no directories.  Returns false and fills s_error
// on failure.
bool write_file(const std::string &s_path, const std::string &s_data, std::string &s_error);

// Whether a path names an existing regular file.
bool file_exists(const std::string &s_path);

// Standard base64 (RFC 4648, with padding), used to embed an inline photo in the
// rendered SVG as a data URI.
std::string base64_encode(const std::string &s_data);

// Decode standard base64.  Whitespace is ignored, because a folded or
// wrapped value carries it.  Returns false when the input has a character the
// alphabet does not contain or a padding error.
bool base64_decode(const std::string &s_encoded, std::string &s_data);

}  // namespace gnome_vcard::util
