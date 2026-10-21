//
// viewer_content_test: exercise the non-GTK content the viewer's Discovered
// and About tabs show.
//
// Usage: viewer-content-test FIXTURE_DIRECTORY
//
#include "tools/viewer_content.h"
#include "version/version.h"

#include "test_harness.h"

#include <iostream>
#include <string>

int main(int i_argument_count, char *o_arguments[]) {
    if (2 > i_argument_count) {
        std::cerr << "usage: viewer-content-test FIXTURE_DIRECTORY\n";
        return 2;
    }
    const std::string s_directory = o_arguments[1];

    using gnome_vcard::viewer::inspect_command;
    using gnome_vcard::viewer::shell_quote;

    EXPECT_EQ(std::string("/usr/bin/vcard-inspect"), shell_quote("/usr/bin/vcard-inspect"));
    EXPECT_EQ(std::string("card.vcf"), shell_quote("card.vcf"));
    EXPECT_EQ(std::string("'/home/a b/vcard-inspect'"), shell_quote("/home/a b/vcard-inspect"));
    EXPECT_EQ(std::string("'/home/a'\\''b/vcard-inspect'"),
              shell_quote("/home/a'b/vcard-inspect"));
    EXPECT_EQ(std::string("''"), shell_quote(""));
    EXPECT_EQ(std::string("/x/vcard-inspect /y/card.vcf"),
              inspect_command("/x/vcard-inspect", "/y/card.vcf"));
    EXPECT_EQ(std::string("/x/vcard-inspect '/y/a b.vcf'"),
              inspect_command("/x/vcard-inspect", "/y/a b.vcf"));

    EXPECT_CONTAINS(gnome_vcard::viewer::version_line(),
                    gnome_vcard::version::version_string());
    EXPECT_CONTAINS(gnome_vcard::viewer::version_line(),
                    std::to_string(gnome_vcard::version::version_build_counter()));
    EXPECT_CONTAINS(std::string(gnome_vcard::viewer::PROJECT_NAME), "GNOME VCARD");
    EXPECT_CONTAINS(std::string(gnome_vcard::viewer::PROJECT_REPOSITORY), "github.com/");
    EXPECT_EQ(std::string("vCard"), gnome_vcard::viewer::TAB_VCARD);
    EXPECT_EQ(std::string("Discovered"), gnome_vcard::viewer::TAB_DISCOVERED);
    EXPECT_EQ(std::string("About"), gnome_vcard::viewer::TAB_ABOUT);

    std::string s_command;
    std::string s_output;
    std::string s_error;
    int i_status = 0;
    EXPECT_TRUE(gnome_vcard::viewer::run_inspect(s_directory + "/simple.vcf", s_command,
                                                 s_output, i_status, s_error));
    EXPECT_EQ(0, i_status);
    EXPECT_CONTAINS(s_command, "vcard-inspect");
    EXPECT_CONTAINS(s_command, "simple.vcf");
    EXPECT_CONTAINS(s_output, "cards: 1");
    EXPECT_CONTAINS(s_output, "formatted name: Ada Lovelace");

    // A file the inspector refuses still ran: the Discovered tab shows its
    // output and its exit status rather than a viewer error.
    EXPECT_TRUE(gnome_vcard::viewer::run_inspect(s_directory + "/absent.vcf", s_command,
                                                 s_output, i_status, s_error));
    EXPECT_TRUE(0 != i_status);
    EXPECT_CONTAINS(s_output, "vcard-inspect:");

    return vcard_test::finish("viewer-content-test");
}
