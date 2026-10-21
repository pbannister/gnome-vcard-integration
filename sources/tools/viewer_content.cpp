#include "tools/viewer_content.h"

#include "util/file_io.h"
#include "util/process.h"
#include "version/version.h"

#include <cstdlib>
#include <string>
#include <vector>
#include <unistd.h>

namespace gnome_vcard::viewer {
namespace {

std::string program_directory() {
    char o_buffer[4096];
    const ssize_t i_length = ::readlink("/proc/self/exe", o_buffer, sizeof(o_buffer) - 1);
    if (0 < i_length) {
        o_buffer[i_length] = '\0';
        const std::string s_path(o_buffer);
        const std::size_t u_slash = s_path.find_last_of('/');
        if (std::string::npos != u_slash) {
            return s_path.substr(0, u_slash);
        }
    }
    return std::string();
}

// The inspector is installed beside the viewer.  A viewer run from a build tree
// or from a prefix both find it this way; PATH is the fallback for an unusual
// layout.
std::string find_inspect_tool() {
    const std::string s_directory = program_directory();
    if (!s_directory.empty()) {
        const std::string s_beside = s_directory + "/vcard-inspect";
        if (util::file_exists(s_beside)) {
            return s_beside;
        }
    }

    const char *s_path = ::getenv("PATH");
    if (nullptr == s_path) {
        return std::string();
    }
    std::string s_rest(s_path);
    while (!s_rest.empty()) {
        const std::size_t u_colon = s_rest.find(':');
        const std::string s_one = (std::string::npos == u_colon) ? s_rest : s_rest.substr(0, u_colon);
        if (!s_one.empty()) {
            const std::string s_candidate = s_one + "/vcard-inspect";
            if (util::file_exists(s_candidate)) {
                return s_candidate;
            }
        }
        if (std::string::npos == u_colon) {
            break;
        }
        s_rest = s_rest.substr(u_colon + 1);
    }
    return std::string();
}

bool is_shell_safe(char c_next) {
    if (('A' <= c_next && 'Z' >= c_next) || ('a' <= c_next && 'z' >= c_next) ||
        ('0' <= c_next && '9' >= c_next)) {
        return true;
    }
    switch (c_next) {
        case '_':
        case '-':
        case '.':
        case '/':
        case ':':
        case '@':
        case '+':
        case '=':
        case ',':
        case '%':
            return true;
        default:
            return false;
    }
}

}  // namespace

std::string shell_quote(const std::string &s_text) {
    if (s_text.empty()) {
        return "''";
    }
    bool b_safe = true;
    for (const char c_next : s_text) {
        if (!is_shell_safe(c_next)) {
            b_safe = false;
            break;
        }
    }
    if (b_safe) {
        return s_text;
    }

    std::string s_quoted = "'";
    for (const char c_next : s_text) {
        if ('\'' == c_next) {
            s_quoted += "'\\''";
        } else {
            s_quoted.push_back(c_next);
        }
    }
    s_quoted.push_back('\'');
    return s_quoted;
}

std::string inspect_command(const std::string &s_tool_path, const std::string &s_file_path) {
    return shell_quote(s_tool_path) + " " + shell_quote(s_file_path);
}

bool run_inspect(const std::string &s_file_path, std::string &s_command, std::string &s_output,
                 int &i_status, std::string &s_error) {
    s_command.clear();
    s_output.clear();
    s_error.clear();
    i_status = 0;

    const std::string s_tool = find_inspect_tool();
    if (s_tool.empty()) {
        s_error = "vcard-inspect was not found beside vcard-viewer or on PATH";
        return false;
    }

    s_command = inspect_command(s_tool, s_file_path);
    bool b_exited = false;
    if (!util::run_command({s_tool, s_file_path}, s_output, i_status, b_exited, s_error)) {
        return false;
    }
    if (!b_exited) {
        s_error = "vcard-inspect did not exit";
        return false;
    }
    return true;
}

std::string version_line() {
    return "Version " + std::string(version::version_string()) + " (build " +
           std::to_string(version::version_build_counter()) + ")";
}

}  // namespace gnome_vcard::viewer
