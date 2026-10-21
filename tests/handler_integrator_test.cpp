//
// handler_integrator_test: check the handler identity and the text it writes,
// without touching a real desktop.
//
// Usage: handler-integrator-test
//
#include "integration/handler_integrator.h"

#include "test_harness.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

namespace {

void test_mime_definition() {
    const std::string s_xml = gnome_vcard::integration::mime_definition_xml();
    EXPECT_CONTAINS(s_xml, "type=\"text/vcard\"");
    EXPECT_CONTAINS(s_xml, "alias type=\"text/x-vcard\"");
    EXPECT_CONTAINS(s_xml, "glob pattern=\"*.vcf\"");
    EXPECT_CONTAINS(s_xml, "glob pattern=\"*.vcard\"");
    EXPECT_CONTAINS(s_xml, "BEGIN:VCARD");
    EXPECT_CONTAINS(s_xml, "generic-icon name=\"vcard-handler\"");
}

void test_desktop_entry() {
    const std::string s_entry =
        gnome_vcard::integration::handler_desktop_text("/usr/local/bin/vcard-integrate", "v1");
    EXPECT_CONTAINS(s_entry, "[Desktop Entry]");
    EXPECT_CONTAINS(s_entry, "Exec=/usr/local/bin/vcard-integrate handle %f");
    EXPECT_CONTAINS(s_entry, "Icon=vcard-handler");
    EXPECT_CONTAINS(s_entry, "StartupWMClass=vcard-viewer");
    EXPECT_CONTAINS(s_entry, "MimeType=text/vcard;text/x-vcard;");
    EXPECT_CONTAINS(s_entry, "X-Integrated-By=gnome-vcard-integration");
    EXPECT_CONTAINS(s_entry, "X-Integrated-At=v1");
    EXPECT_CONTAINS(s_entry, "NoDisplay=true");
}

void test_exec_quote() {
    using gnome_vcard::integration::exec_quote;
    EXPECT_EQ(std::string("/usr/bin/vcard-integrate"), exec_quote("/usr/bin/vcard-integrate"));
    EXPECT_EQ(std::string("\"/home/a b/vcard-integrate\""), exec_quote("/home/a b/vcard-integrate"));
    EXPECT_EQ(std::string("\"/home/a\\\"b/vcard-integrate\""),
              exec_quote("/home/a\"b/vcard-integrate"));
}

void test_environment_paths() {
    const std::string s_home = "/tmp/vcard-handler-integrator-test-home";
    ::setenv("HOME", s_home.c_str(), 1);
    ::setenv("XDG_DATA_HOME", (s_home + "/data").c_str(), 1);
    ::setenv("XDG_CONFIG_HOME", (s_home + "/config").c_str(), 1);

    const gnome_vcard::integration::handler_integrator_c o_integrator =
        gnome_vcard::integration::handler_integrator_c::from_environment();
    const gnome_vcard::integration::paths_o &o_paths = o_integrator.paths();

    EXPECT_EQ(s_home + "/data", o_paths.s_data_home);
    EXPECT_EQ(s_home + "/data/applications", o_paths.s_applications_directory);
    EXPECT_EQ(s_home + "/data/mime/packages/vcard.xml", o_paths.s_mime_package);
    EXPECT_EQ(s_home + "/data/applications/vcard-handler.desktop", o_paths.s_handler_desktop);
    EXPECT_EQ(s_home + "/data/gnome-vcard-integration/vcard-handler.manifest",
              o_paths.s_handler_manifest);
    EXPECT_FALSE(o_integrator.has_record());
}

}  // namespace

int main() {
    test_mime_definition();
    test_desktop_entry();
    test_exec_quote();
    test_environment_paths();

    return vcard_test::finish("handler-integrator-test");
}
