#pragma once

#include <string>
#include <utility>
#include <vector>

namespace gnome_vcard::integration {

// The MIME types this project handles.  shared-mime-info defines text/vcard and
// aliases the legacy text/x-vcard to it; both are registered so a producer that
// writes either resolves to this handler.
constexpr const char *MIME_TYPE_CARD = "text/vcard";
constexpr const char *MIME_TYPE_CARD_LEGACY = "text/x-vcard";

// The identity of the integration.  These names are the file names the desktop
// looks for, so they are constants rather than parameters.
constexpr const char *HANDLER_DESKTOP_ID = "vcard-handler.desktop";
constexpr const char *HANDLER_NAME = "vCard Viewer";
constexpr const char *HANDLER_ICON_NAME = "vcard-handler";
constexpr const char *HANDLER_WM_CLASS = "vcard-viewer";
constexpr const char *HANDLER_MANIFEST = "vcard-handler.manifest";
constexpr const char *MIME_PACKAGE_NAME = "vcard.xml";
constexpr const char *STATE_DIRECTORY_NAME = "gnome-vcard-integration";

// The directories this integration reads and writes, resolved from the
// environment (XDG_DATA_HOME, XDG_CONFIG_HOME, HOME).  Resolving them once makes
// every path a test can assert on explicit.
struct paths_o {
    std::string s_home;
    std::string s_data_home;
    std::string s_config_home;
    std::string s_state_directory;
    std::string s_applications_directory;
    std::string s_mime_directory;      // data home + /mime
    std::string s_mime_packages;       // data home + /mime/packages
    std::string s_icons_directory;     // data home + /icons/hicolor
    std::string s_handler_desktop;     // applications + /vcard-handler.desktop
    std::string s_handler_manifest;    // state directory + /vcard-handler.manifest
    std::string s_mime_package;        // mime/packages/vcard.xml
    std::string s_mime_backup;         // state directory + /backup/vcard.xml
};

// What `handler status` reports; also what a person needs to see after an
// install or uninstall.
struct status_o {
    bool b_desktop_present = false;
    std::string s_desktop_path;
    // The MIME type and the desktop file ID currently recorded as its default,
    // as `xdg-mime query default` reports it.
    std::vector<std::pair<std::string, std::string>> o_defaults;
    bool b_mime_package_present = false;
    bool b_mime_package_ours = false;
    bool b_card_icon_present = false;
    bool b_legacy_icon_present = false;
    bool b_manifest_present = false;
    bool b_mime_package_created_by_us = false;
};

class handler_integrator_c {
public:
    // Resolve the environment once.  No directory is created here.
    static handler_integrator_c from_environment(const std::string &s_tool_path = std::string());

    const paths_o &paths() const;

    // Read the current state without changing anything.
    bool status(status_o &o_status, std::string &s_error) const;

    // Register the handler.  Idempotent: running it twice leaves one handler and
    // repairs a damaged one.  o_log collects one human-readable line per action.
    bool install(std::vector<std::string> &o_log, std::string &s_error);

    // Reverse an install: restore the previous defaults, remove the entry, the
    // icons, and the MIME definition this run owns, and remove the record.
    bool uninstall(std::vector<std::string> &o_log, std::string &s_error);

    // Whether a manifest records an install, which is what uninstall needs.
    bool has_record() const;

private:
    handler_integrator_c() = default;

    bool read_manifest(std::vector<std::pair<std::string, std::string>> &o_entries) const;
    std::string manifest_value(const std::vector<std::pair<std::string, std::string>> &o_entries,
                               const std::string &s_key) const;
    std::string tool_path() const;
    std::string icon_source() const;

    paths_o o_paths_;
    std::string s_tool_path_;
};

// The MIME type definition XML this project installs.  Public so a test can
// assert on it without running an install.
std::string mime_definition_xml();

// The desktop entry text, with <tool> replaced by the resolved tool path.
std::string handler_desktop_text(const std::string &s_tool_path, const std::string &s_version);

// Quote one path for an `Exec=` key, per the Desktop Entry Specification: a path
// with a space or a reserved character is wrapped in double quotes, and the
// characters that are special inside the quotes are escaped.
std::string exec_quote(const std::string &s_path);

}  // namespace gnome_vcard::integration
