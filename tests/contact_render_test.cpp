//
// contact_render_test: exercise the SVG contact-card renderer.
//
// Usage: contact-render-test FIXTURE_DIRECTORY
//
#include "render/contact_render.h"
#include "vcard/vcard_reader.h"

#include "test_harness.h"

#include <iostream>
#include <string>

namespace {

gnome_vcard::vcard::contact_o contact_from(const std::string &s_document) {
    gnome_vcard::vcard::file_o o_file;
    std::string s_error;
    if (!gnome_vcard::vcard::parse_vcards(s_document, "memory.vcf", o_file, s_error)) {
        vcard_test::fail(__FILE__, __LINE__, "the in-memory card did not parse: " + s_error);
        return gnome_vcard::vcard::contact_o();
    }
    if (o_file.o_contacts.empty()) {
        vcard_test::fail(__FILE__, __LINE__, "the in-memory card produced no contact");
        return gnome_vcard::vcard::contact_o();
    }
    return o_file.o_contacts.front();
}

void test_fixture_card(const std::string &s_directory) {
    gnome_vcard::vcard::file_o o_file;
    std::string s_error;
    EXPECT_TRUE(gnome_vcard::vcard::read_vcards(s_directory + "/simple.vcf", o_file, s_error));
    if (o_file.o_contacts.empty()) {
        return;
    }

    gnome_vcard::render::options_o o_options;
    const std::string s_svg =
        gnome_vcard::render::render_contact_svg(o_file.o_contacts.front(), o_options);

    EXPECT_CONTAINS(s_svg, "<svg");
    EXPECT_CONTAINS(s_svg, "Ada Lovelace");
    EXPECT_CONTAINS(s_svg, "Analytical Engine Society");
    EXPECT_CONTAINS(s_svg, "Mathematician");
    EXPECT_CONTAINS(s_svg, "mailto:ada@example.invalid");
    EXPECT_CONTAINS(s_svg, "tel:+44200000000");
    EXPECT_CONTAINS(s_svg, "https://example.invalid/ada");
    EXPECT_CONTAINS(s_svg, "12 Example Street");
    EXPECT_CONTAINS(s_svg, "data:image/png;base64,");
    EXPECT_CONTAINS(s_svg, "first published algorithm");
    EXPECT_CONTAINS(s_svg, "&lt;angle&gt;");
    EXPECT_NOT_CONTAINS(s_svg, "<script>");
    EXPECT_NOT_CONTAINS(s_svg, "href=\"javascript:");

    gnome_vcard::render::options_o o_no_photo;
    o_no_photo.b_include_photo = false;
    const std::string s_without_photo =
        gnome_vcard::render::render_contact_svg(o_file.o_contacts.front(), o_no_photo);
    EXPECT_NOT_CONTAINS(s_without_photo, "data:image/png;base64,");

    const std::string s_again =
        gnome_vcard::render::render_contact_svg(o_file.o_contacts.front(), o_options);
    EXPECT_EQ(s_svg, s_again);
}

void test_hostile_card() {
    const std::string s_document =
        "BEGIN:VCARD\n"
        "VERSION:4.0\n"
        "FN:<script>alert(1)</script>\n"
        "N:Hostile;Card;;;\n"
        "EMAIL:hostile@example.invalid\n"
        "TEL:+1 555 0100\n"
        "URL:javascript:alert(1)\n"
        "ADR:;;1 <Street>;Town;;;Country\n"
        "NOTE:first line\\nsecond line\n"
        "END:VCARD\n";
    const gnome_vcard::vcard::contact_o o_contact = contact_from(s_document);

    gnome_vcard::render::options_o o_options;
    const std::string s_svg = gnome_vcard::render::render_contact_svg(o_contact, o_options);

    EXPECT_CONTAINS(s_svg, "&lt;script&gt;");
    EXPECT_NOT_CONTAINS(s_svg, "<script>");
    EXPECT_NOT_CONTAINS(s_svg, "href=\"javascript:");
    EXPECT_CONTAINS(s_svg, "mailto:hostile@example.invalid");
    EXPECT_CONTAINS(s_svg, "tel:+15550100");
    EXPECT_CONTAINS(s_svg, "1 &lt;Street&gt;");
    EXPECT_CONTAINS(s_svg, "first line");
    EXPECT_CONTAINS(s_svg, "second line");
}

void test_remote_photo() {
    const std::string s_document =
        "BEGIN:VCARD\n"
        "VERSION:3.0\n"
        "FN:Remote Photo\n"
        "PHOTO;VALUE=uri:https://example.invalid/photo.png\n"
        "END:VCARD\n";
    const gnome_vcard::vcard::contact_o o_contact = contact_from(s_document);

    gnome_vcard::render::options_o o_options;
    const std::string s_svg = gnome_vcard::render::render_contact_svg(o_contact, o_options);

    EXPECT_NOT_CONTAINS(s_svg, "data:image/png;base64,");
    EXPECT_NOT_CONTAINS(s_svg, "example.invalid/photo.png");
    EXPECT_CONTAINS(s_svg, "not embedded");
}

}  // namespace

int main(int i_argument_count, char *o_arguments[]) {
    if (2 > i_argument_count) {
        std::cerr << "usage: contact-render-test FIXTURE_DIRECTORY\n";
        return 2;
    }

    test_fixture_card(o_arguments[1]);
    test_hostile_card();
    test_remote_photo();

    return vcard_test::finish("contact-render-test");
}
