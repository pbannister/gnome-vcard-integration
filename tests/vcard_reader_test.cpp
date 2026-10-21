//
// vcard_reader_test: exercise the vCard reader against the synthetic fixtures
// and a few in-memory documents.
//
// Usage: vcard-reader-test FIXTURE_DIRECTORY
//
#include "vcard/vcard_reader.h"

#include "test_harness.h"

#include <iostream>
#include <string>

namespace {

std::string fixture_path(const std::string &s_directory, const std::string &s_name) {
    return s_directory + "/" + s_name;
}

void test_simple_vcard(const std::string &s_directory) {
    gnome_vcard::vcard::file_o o_file;
    std::string s_error;
    EXPECT_TRUE(gnome_vcard::vcard::read_vcards(
        fixture_path(s_directory, "simple.vcf"), o_file, s_error));
    EXPECT_EQ(std::string(), s_error);
    EXPECT_EQ(static_cast<std::size_t>(1), o_file.o_contacts.size());
    if (o_file.o_contacts.empty()) {
        return;
    }

    const gnome_vcard::vcard::contact_o &o_contact = o_file.o_contacts.front();
    EXPECT_EQ(std::string("4.0"), o_contact.s_version);
    EXPECT_EQ(std::string("Ada Lovelace"), o_contact.formatted_name());
    EXPECT_EQ(std::string("Analytical Engine Society"), o_contact.organization());
    EXPECT_EQ(std::string("Mathematics"), o_contact.organization_unit());
    EXPECT_EQ(std::string("Mathematician"), o_contact.title());
    EXPECT_EQ(std::string("Programmer"), o_contact.role());
    EXPECT_EQ(static_cast<std::size_t>(5), o_contact.name_components().size());
    EXPECT_EQ(static_cast<std::size_t>(2), o_contact.telephones().size());
    EXPECT_EQ(static_cast<std::size_t>(2), o_contact.emails().size());
    EXPECT_EQ(static_cast<std::size_t>(1), o_contact.urls().size());
    EXPECT_EQ(static_cast<std::size_t>(1), o_contact.addresses().size());
    EXPECT_TRUE(o_contact.o_warnings.empty());

    const std::vector<gnome_vcard::vcard::property_o> o_telephones = o_contact.telephones();
    if (!o_telephones.empty()) {
        const gnome_vcard::vcard::property_o &o_telephone = o_telephones.front();
        EXPECT_EQ(std::string("+44 20 0000 0000"), o_telephone.s_value);
        EXPECT_TRUE(o_telephone.has_type("VOICE"));
        EXPECT_TRUE(o_telephone.has_type("WORK"));
        EXPECT_TRUE(o_telephone.b_preferred);
        EXPECT_EQ(1, o_telephone.i_preference);
    }
    const std::vector<gnome_vcard::vcard::address_o> o_addresses = o_contact.addresses();
    if (!o_addresses.empty()) {
        const gnome_vcard::vcard::address_o &o_address = o_addresses.front();
        EXPECT_EQ(std::string("12 Example Street"), o_address.s_street);
        EXPECT_EQ(std::string("London"), o_address.s_locality);
        EXPECT_EQ(std::string("SW1A 1AA"), o_address.s_postal_code);
        EXPECT_EQ(std::string("United Kingdom"), o_address.s_country);
        EXPECT_CONTAINS(o_address.formatted(), "12 Example Street");
    }

    const gnome_vcard::vcard::photo_o o_photo = o_contact.photo();
    EXPECT_TRUE(o_photo.b_present);
    EXPECT_TRUE(o_photo.b_embedded);
    EXPECT_EQ(std::string("image/png"), o_photo.s_media_type);
    EXPECT_TRUE(0 < o_photo.s_bytes.size());
    EXPECT_EQ(static_cast<unsigned char>(0x89),
              static_cast<unsigned char>(o_photo.s_bytes.front()));

    EXPECT_CONTAINS(o_contact.note(), "first published algorithm");
    EXPECT_CONTAINS(o_contact.note(), "Second line");
    EXPECT_CONTAINS(o_contact.note(), "<angle>");
    EXPECT_TRUE(o_contact.note().find('\n') != std::string::npos);
    EXPECT_EQ(static_cast<std::size_t>(2), o_contact.nicknames().size());
    EXPECT_EQ(std::string("1815-12-10"), o_contact.birthday());
    EXPECT_EQ(static_cast<std::size_t>(1), o_contact.impps().size());
}

void test_legacy_vcard(const std::string &s_directory) {
    gnome_vcard::vcard::file_o o_file;
    std::string s_error;
    EXPECT_TRUE(gnome_vcard::vcard::read_vcards(
        fixture_path(s_directory, "legacy.vcf"), o_file, s_error));
    EXPECT_EQ(static_cast<std::size_t>(1), o_file.o_contacts.size());
    if (o_file.o_contacts.empty()) {
        return;
    }

    const gnome_vcard::vcard::contact_o &o_contact = o_file.o_contacts.front();
    EXPECT_EQ(std::string("2.1"), o_contact.s_version);
    // The fixture has no FN, so the display name is derived from N.
    EXPECT_EQ(std::string("Countess Ada Augusta Lovelace"), o_contact.formatted_name());
    // The title is quoted-printable Latin-1: "Mathématician".
    EXPECT_CONTAINS(o_contact.title(), "Math");
    EXPECT_CONTAINS(o_contact.title(), "\xc3\xa9");
    EXPECT_EQ(static_cast<std::size_t>(2), o_contact.telephones().size());
    const std::vector<gnome_vcard::vcard::property_o> o_telephones = o_contact.telephones();
    if (!o_telephones.empty()) {
        const gnome_vcard::vcard::property_o &o_telephone = o_telephones.front();
        EXPECT_TRUE(o_telephone.has_type("HOME"));
        EXPECT_TRUE(o_telephone.has_type("VOICE"));
    }
    const std::vector<gnome_vcard::vcard::property_o> o_emails = o_contact.emails();
    if (!o_emails.empty()) {
        EXPECT_TRUE(o_emails.front().b_preferred);
        EXPECT_TRUE(o_emails.front().has_type("INTERNET"));
    }
    EXPECT_CONTAINS(o_contact.note(), "first published algorithm");
    EXPECT_CONTAINS(o_contact.note(), "second line");
    EXPECT_CONTAINS(o_contact.note(), "\xe2\x80\x94");
}

void test_soft_break_card(const std::string &s_directory) {
    gnome_vcard::vcard::file_o o_file;
    std::string s_error;
    EXPECT_TRUE(gnome_vcard::vcard::read_vcards(
        fixture_path(s_directory, "soft-break.vcf"), o_file, s_error));
    EXPECT_EQ(static_cast<std::size_t>(1), o_file.o_contacts.size());
    if (o_file.o_contacts.empty()) {
        return;
    }

    const gnome_vcard::vcard::contact_o &o_contact = o_file.o_contacts.front();
    EXPECT_EQ(std::string("2.1"), o_contact.s_version);
    EXPECT_EQ(std::string("Person Example"), o_contact.formatted_name());
    EXPECT_TRUE(o_contact.o_warnings.empty());
    EXPECT_EQ(static_cast<std::size_t>(13), o_contact.property_count());
    EXPECT_EQ(static_cast<std::size_t>(2), o_contact.telephones().size());

    const std::vector<gnome_vcard::vcard::address_o> o_addresses = o_contact.addresses();
    EXPECT_EQ(static_cast<std::size_t>(1), o_addresses.size());
    if (!o_addresses.empty()) {
        const gnome_vcard::vcard::address_o &o_address = o_addresses.front();
        EXPECT_EQ(std::string("123 Example Street\nExampleville"), o_address.s_street);
        EXPECT_EQ(std::string("CO"), o_address.s_locality);
        EXPECT_EQ(std::string("00000"), o_address.s_postal_code);
        EXPECT_TRUE(o_address.s_region.empty());
        EXPECT_TRUE(o_address.b_preferred);
        EXPECT_TRUE(o_address.is_empty() == false);
        EXPECT_CONTAINS(o_address.formatted(), "Exampleville");
        EXPECT_TRUE(o_address.formatted().find('\n') != std::string::npos);
    }

    const std::string s_label = o_contact.first_text("LABEL");
    EXPECT_CONTAINS(s_label, "123 Example Street");
    EXPECT_CONTAINS(s_label, "Exampleville, CO  00000");
    EXPECT_TRUE(s_label.find('\n') != std::string::npos);

    const gnome_vcard::vcard::photo_o o_photo = o_contact.photo();
    EXPECT_TRUE(o_photo.b_present);
    EXPECT_FALSE(o_photo.b_embedded);
    EXPECT_CONTAINS(o_photo.s_uri, "https://example.invalid/photo.png");
}

void test_multiple_cards(const std::string &s_directory) {
    gnome_vcard::vcard::file_o o_file;
    std::string s_error;
    EXPECT_TRUE(gnome_vcard::vcard::read_vcards(
        fixture_path(s_directory, "multiple.vcf"), o_file, s_error));
    EXPECT_EQ(static_cast<std::size_t>(2), o_file.o_contacts.size());
    if (2 == o_file.o_contacts.size()) {
        EXPECT_EQ(std::string("Grace Hopper"), o_file.o_contacts[0].formatted_name());
        EXPECT_EQ(std::string("Katherine Johnson"), o_file.o_contacts[1].formatted_name());
    }
}

void test_incomplete_card(const std::string &s_directory) {
    gnome_vcard::vcard::file_o o_file;
    std::string s_error;
    EXPECT_TRUE(gnome_vcard::vcard::read_vcards(
        fixture_path(s_directory, "incomplete.vcf"), o_file, s_error));
    EXPECT_EQ(static_cast<std::size_t>(1), o_file.o_contacts.size());
    if (!o_file.o_contacts.empty()) {
        const gnome_vcard::vcard::contact_o &o_contact = o_file.o_contacts.front();
        EXPECT_EQ(std::string("3.0"), o_contact.s_version);
        EXPECT_EQ(std::string("Person Example"), o_contact.formatted_name());
        EXPECT_FALSE(o_contact.o_warnings.empty());
    }
}

void test_invalid_document(const std::string &s_directory) {
    gnome_vcard::vcard::file_o o_file;
    std::string s_error;
    EXPECT_FALSE(gnome_vcard::vcard::read_vcards(
        fixture_path(s_directory, "invalid.vcf"), o_file, s_error));
    EXPECT_CONTAINS(s_error, "not a vCard");
}

void test_parse_edge_cases() {
    const std::string s_card =
        "BEGIN:VCARD\r\n"
        "VERSION:3.0\r\n"
        "FN:Smith\\, John\r\n"
        "N:Smith;John;;;\r\n"
        "NOTE;ENCODING=QUOTED-PRINTABLE:Hello=20world=0D=0A=\r\n"
        " continued\r\n"
        "TEL;TYPE=\"voice,home\":+1 555 0100\r\n"
        "END:VCARD\r\n";
    gnome_vcard::vcard::file_o o_file;
    std::string s_error;
    EXPECT_TRUE(gnome_vcard::vcard::parse_vcards(s_card, "memory.vcf", o_file, s_error));
    EXPECT_EQ(static_cast<std::size_t>(1), o_file.o_contacts.size());
    if (!o_file.o_contacts.empty()) {
        const gnome_vcard::vcard::contact_o &o_contact = o_file.o_contacts.front();
        EXPECT_EQ(std::string("Smith, John"), o_contact.formatted_name());
        EXPECT_CONTAINS(o_contact.note(), "Hello world");
        EXPECT_CONTAINS(o_contact.note(), "continued");
        EXPECT_TRUE(o_contact.note().find("world\n") != std::string::npos);
        const std::vector<gnome_vcard::vcard::property_o> o_telephones = o_contact.telephones();
        if (!o_telephones.empty()) {
            EXPECT_TRUE(o_telephones.front().has_type("VOICE"));
            EXPECT_TRUE(o_telephones.front().has_type("HOME"));
        }
    }

    // A NUL byte is not a text vCard.
    std::string s_binary = "BEGIN:VCARD\nVERSION:4.0\nFN:A\nEND:VCARD\n";
    s_binary.insert(s_binary.end() - 2, '\0');
    EXPECT_FALSE(gnome_vcard::vcard::parse_vcards(s_binary, "binary.vcf", o_file, s_error));

    // An unknown charset is a warning, not a failure.
    const std::string s_charset =
        "BEGIN:VCARD\nVERSION:3.0\nFN:Test\nNOTE;CHARSET=KOI8-R:hello\nEND:VCARD\n";
    EXPECT_TRUE(gnome_vcard::vcard::parse_vcards(s_charset, "charset.vcf", o_file, s_error));
    if (!o_file.o_contacts.empty()) {
        EXPECT_FALSE(o_file.o_contacts.front().o_warnings.empty());
    }

    // An unclosed card is a structural failure.
    const std::string s_unclosed = "BEGIN:VCARD\nVERSION:4.0\nFN:Test\n";
    EXPECT_FALSE(
        gnome_vcard::vcard::parse_vcards(s_unclosed, "unclosed.vcf", o_file, s_error));
    EXPECT_CONTAINS(s_error, "END:VCARD");

    // A data: URI is decoded into bytes and keeps its media type.
    const std::string s_data_uri =
        "BEGIN:VCARD\nVERSION:4.0\nFN:Test\n"
        "PHOTO;VALUE=uri:data:image/png;base64,iVBORw0KGgo=\n"
        "END:VCARD\n";
    EXPECT_TRUE(gnome_vcard::vcard::parse_vcards(s_data_uri, "data.vcf", o_file, s_error));
    if (!o_file.o_contacts.empty()) {
        EXPECT_TRUE(o_file.o_contacts.front().photo().b_embedded);
    }

    // A bare "=" ends a quoted-printable value's physical line even when the
    // continuation has no leading whitespace.  A colon in that continuation
    // must not be mistaken for a new property.
    const std::string s_soft_break =
        "BEGIN:VCARD\nVERSION:3.0\nFN:Test\nN:Test;;;;\n"
        "NOTE;ENCODING=QUOTED-PRINTABLE:first=0D=0A=\n"
        "https://example.invalid/x\n"
        "END:VCARD\n";
    EXPECT_TRUE(gnome_vcard::vcard::parse_vcards(s_soft_break, "soft.vcf", o_file, s_error));
    if (!o_file.o_contacts.empty()) {
        EXPECT_CONTAINS(o_file.o_contacts.front().note(), "https://example.invalid/x");
        EXPECT_TRUE(o_file.o_contacts.front().o_warnings.empty());
    }

    // A line that is not quoted-printable keeps a trailing "=" as text; the
    // next line is still a property.
    const std::string s_equals_text =
        "BEGIN:VCARD\nVERSION:3.0\nFN:Test\nNOTE:ends with =\nTEL:123\nEND:VCARD\n";
    EXPECT_TRUE(gnome_vcard::vcard::parse_vcards(s_equals_text, "equals.vcf", o_file, s_error));
    if (!o_file.o_contacts.empty()) {
        EXPECT_EQ(std::string("ends with ="), o_file.o_contacts.front().note());
        EXPECT_EQ(static_cast<std::size_t>(1), o_file.o_contacts.front().telephones().size());
    }
}

}  // namespace

int main(int i_argument_count, char *o_arguments[]) {
    if (2 > i_argument_count) {
        std::cerr << "usage: vcard-reader-test FIXTURE_DIRECTORY\n";
        return 2;
    }
    const std::string s_directory = o_arguments[1];

    test_simple_vcard(s_directory);
    test_legacy_vcard(s_directory);
    test_soft_break_card(s_directory);
    test_multiple_cards(s_directory);
    test_incomplete_card(s_directory);
    test_invalid_document(s_directory);
    test_parse_edge_cases();

    return vcard_test::finish("vcard-reader-test");
}
