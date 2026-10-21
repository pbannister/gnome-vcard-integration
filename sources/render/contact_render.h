#pragma once

#include "vcard/vcard_model.h"

#include <string>

namespace gnome_vcard::render {

// What a caller can turn off.  The accent is a fixed part of this project's
// card design, not a vCard property.
struct options_o {
    bool b_include_photo = true;
};

// Draw one contact as a standalone SVG document.  Every string from the card is
// XML-escaped, and only mailto:, tel:, http:, and https: become links.
std::string render_contact_svg(const vcard::contact_o &o_contact, const options_o &o_options);

// Whether a media type is a raster image this project will embed.  SVG is
// deliberately excluded: a photo is untrusted input, and an SVG can carry a
// script.
bool is_embeddable_image(const std::string &s_media_type);

}  // namespace gnome_vcard::render
