#pragma once

#include "vcard/vcard_model.h"

#include <string>

namespace gnome_vcard::vcard {

// Read a vCard file from disk.  Returns false only when the file cannot be read
// or is not a vCard; a card that is readable but incomplete is returned with its
// warnings in file.o_contacts[i].o_warnings.
bool read_vcards(const std::string &s_path, file_o &o_file, std::string &s_error);

// Read a vCard document already in memory.  s_path is recorded for the report
// and is not opened.
bool parse_vcards(const std::string &s_text, const std::string &s_path, file_o &o_file,
                  std::string &s_error);

}  // namespace gnome_vcard::vcard
