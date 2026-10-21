#pragma once

#include <string>

namespace gnome_vcard::viewer {

// The project identity the About tab shows.
constexpr const char *PROJECT_NAME = "GNOME VCARD Integration";
constexpr const char *PROJECT_REPOSITORY = "https://github.com/pbannister/gnome-vcard-integration";
constexpr const char *PROJECT_DESCRIPTION =
    "Reads vCard files and gives them a place on the GNOME desktop.";

// The tab labels, in display order.
constexpr const char *TAB_VCARD = "vCard";
constexpr const char *TAB_DISCOVERED = "Discovered";
constexpr const char *TAB_ABOUT = "About";

// Quote one argument for a POSIX shell command line.  A path with no character
// the shell treats specially is returned as written; anything else is wrapped
// in single quotes, with an embedded single quote written as '\''.
std::string shell_quote(const std::string &s_text);

// The command line the Discovered tab shows: the tool path and the file path,
// each quoted for the shell.
std::string inspect_command(const std::string &s_tool_path, const std::string &s_file_path);

// Run `vcard-inspect FILE` and capture the exact command and its combined
// standard output and standard error.  Returns false only when the inspector
// cannot be found or started; a file the inspector refuses is a successful run
// with a nonzero exit status and an output that explains why.
bool run_inspect(const std::string &s_file_path, std::string &s_command, std::string &s_output,
                 int &i_status, std::string &s_error);

// The About tab's version line, for example
// "Version 2026-10-21-master-abc1234 (build 81)".
std::string version_line();

}  // namespace gnome_vcard::viewer
