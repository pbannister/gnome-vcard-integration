#include "util/process.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

namespace gnome_vcard::util {

bool command_exists(const std::string &s_program) {
    if (s_program.empty()) {
        return false;
    }
    if (std::string::npos != s_program.find('/')) {
        return 0 == ::access(s_program.c_str(), X_OK);
    }
    const char *s_path = ::getenv("PATH");
    if (nullptr == s_path) {
        return false;
    }
    std::string s_rest(s_path);
    while (!s_rest.empty()) {
        const std::size_t u_colon = s_rest.find(':');
        const std::string s_one =
            (std::string::npos == u_colon) ? s_rest : s_rest.substr(0, u_colon);
        if (!s_one.empty()) {
            const std::string s_candidate = s_one + "/" + s_program;
            if (0 == ::access(s_candidate.c_str(), X_OK)) {
                return true;
            }
        }
        if (std::string::npos == u_colon) {
            break;
        }
        s_rest = s_rest.substr(u_colon + 1);
    }
    return false;
}

bool run_command(const std::vector<std::string> &o_arguments, std::string &s_output,
                 int &i_status, bool &b_exited, std::string &s_error) {
    s_output.clear();
    i_status = 0;
    b_exited = false;
    s_error.clear();

    if (o_arguments.empty()) {
        s_error = "no program to run";
        return false;
    }

    int o_pipe[2];
    if (0 != ::pipe(o_pipe)) {
        s_error = std::string("cannot create a pipe: ") + std::strerror(errno);
        return false;
    }

    const pid_t i_child = ::fork();
    if (0 > i_child) {
        s_error = std::string("cannot fork: ") + std::strerror(errno);
        ::close(o_pipe[0]);
        ::close(o_pipe[1]);
        return false;
    }

    if (0 == i_child) {
        ::close(o_pipe[0]);
        if (o_pipe[1] != STDOUT_FILENO) {
            ::dup2(o_pipe[1], STDOUT_FILENO);
            ::close(o_pipe[1]);
        }
        ::dup2(STDOUT_FILENO, STDERR_FILENO);

        std::vector<char *> o_raw;
        o_raw.reserve(o_arguments.size() + 1);
        for (const std::string &s_argument : o_arguments) {
            o_raw.push_back(const_cast<char *>(s_argument.c_str()));
        }
        o_raw.push_back(nullptr);
        ::execvp(o_raw[0], o_raw.data());
        // The child cannot report through s_error; it exits with the exec
        // failure status and the parent reports that the program did not start.
        ::_exit(127);
    }

    ::close(o_pipe[1]);
    char o_buffer[4096];
    for (;;) {
        const ssize_t i_count = ::read(o_pipe[0], o_buffer, sizeof(o_buffer));
        if (0 == i_count) {
            break;
        }
        if (0 > i_count) {
            if (EINTR == errno) {
                continue;
            }
            break;
        }
        s_output.append(o_buffer, static_cast<std::size_t>(i_count));
    }
    ::close(o_pipe[0]);

    int i_wait_status = 0;
    if (i_child != ::waitpid(i_child, &i_wait_status, 0)) {
        s_error = std::string("cannot wait for the program: ") + std::strerror(errno);
        return false;
    }
    b_exited = true;
    if (WIFEXITED(i_wait_status)) {
        i_status = WEXITSTATUS(i_wait_status);
        return true;
    }
    if (WIFSIGNALED(i_wait_status)) {
        i_status = 128 + WTERMSIG(i_wait_status);
        return true;
    }
    return false;
}

}  // namespace gnome_vcard::util
