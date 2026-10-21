# MIME Type Association for vCard

This document records how a Linux desktop decides which application opens a
`.vcf` file, what `shared-mime-info` already defines, and what this project
writes to become the handler.

## The chain

A double-click follows this chain:

1. The file's name and first bytes are matched against the MIME database, which
   `shared-mime-info` installs under `/usr/share/mime` and an application may
   extend under `$XDG_DATA_HOME/mime`.
2. The resulting type, `text/vcard`, is looked up in the MIME applications
   database (`mimeapps.list`, `$XDG_CONFIG_HOME/mimeapps.list`, and the desktop
   files that declare `MimeType=`).
3. The default desktop entry is launched with the file as its argument.
4. The desktop entry's `Exec=` line decides what runs.
5. The window's `StartupWMClass=` lets the dock match the running window to the
   entry, so the launcher shows the right icon and groups the window correctly.

## What the desktop already knows

A current Linux desktop with `shared-mime-info` already defines `text/vcard`:

- it is named "Electronic business card";
- it has the aliases `text/directory` and `text/x-vcard`;
- its globs include `*.vcf` and `*.vcard`;
- its magic recognises `BEGIN:VCARD`.

A freshly written `.vcf` file therefore already resolves to `text/vcard` before
this project installs anything.
What is missing is an application to open it: a stock desktop has no handler for
the type, so the double-click goes nowhere useful.

## What this project writes

The integration writes five things under `$XDG_DATA_HOME` and asks `xdg-mime` to
make one of them the default.

### The MIME definition

`mime/packages/vcard.xml`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<mime-info xmlns="http://www.freedesktop.org/standards/shared-mime-info">
  <mime-type type="text/vcard">
    <comment>Electronic business card</comment>
    <alias type="text/x-vcard"/>
    <generic-icon name="vcard-handler"/>
    <magic priority="60">
      <match type="string" value="BEGIN:VCARD" offset="0"/>
    </magic>
    <glob pattern="*.vcf"/>
    <glob pattern="*.vcard"/>
  </mime-type>
</mime-info>
```

The type is already known; this definition exists to give it this project's
`generic-icon` and to keep the association independent of a distribution's
version of `shared-mime-info`.
`update-mime-database "$XDG_DATA_HOME/mime"` compiles it.

A definition at that path that this project did not write is copied to
`$XDG_DATA_HOME/gnome-vcard-integration/backup/vcard.xml` before the new one is
written, and is restored byte for byte on uninstall.
A definition the project created is removed on uninstall.
The record says which case applies.

### The desktop entry

`applications/vcard-handler.desktop`:

```ini
[Desktop Entry]
Type=Application
Name=vCard Viewer
Comment=Inspect and display a vCard contact card
Exec=<tool> handle %f
Icon=vcard-handler
StartupWMClass=vcard-viewer
Terminal=false
NoDisplay=true
MimeType=text/vcard;text/x-vcard;
X-Integrated-By=gnome-vcard-integration
X-Integrated-At=<version>
```

- `Exec` names the installed `vcard-integrate`, so the entry survives a `make
  clean` that removes the build tree.
- `NoDisplay=true` keeps the entry out of the application menu; it is a file
  handler, not a program a person launches directly.
- `StartupWMClass=vcard-viewer` matches the window the viewer creates.
- `MimeType` names both the current type and the legacy alias, so a producer
  that writes `text/x-vcard` also resolves.

### The icons

- `icons/hicolor/scalable/apps/vcard-handler.svg` — the handler entry's icon.
- `icons/hicolor/scalable/mimetypes/text-vcard.svg` — the file icon for a card.
- `icons/hicolor/scalable/mimetypes/text-x-vcard.svg` — the file icon for the
  legacy type.

GIO resolves a file icon from the MIME type's name with `/` replaced by `-`, so
the two type icons are copies of one drawing under the names the desktop looks
for.
`gtk4-update-icon-cache -f -t "$XDG_DATA_HOME/icons/hicolor"` (or
`gtk-update-icon-cache`) refreshes the cache; a stale cache hides a new icon
because GTK trusts a cache that is not older than the theme directory.

### The MIME default

```
xdg-mime default vcard-handler.desktop text/vcard text/x-vcard
```

`xdg-mime` writes the association into `$XDG_CONFIG_HOME/mimeapps.list`.
`update-desktop-database "$XDG_DATA_HOME/applications"` compiles the desktop
entries so the new one is visible.

### The record

`gnome-vcard-integration/vcard-handler.manifest`:

```
handler_desktop=<path>
icon=<path>
icon=<path>
icon=<path>
mime_package=<path>
mime_backup=<path or empty>
mime_created=1            (only when this project created the definition)
previous_default=text/vcard	<desktop id or empty>
previous_default=text/x-vcard	<desktop id or empty>
```

The `previous_default` lines are separated by a tab between the type and the
value.
They are what makes `handler uninstall` able to hand each type back to whatever
opened it before.
Without the record, uninstall refuses rather than guessing.

## Reversal

`vcard-integrate handler uninstall`:

1. restores each recorded previous default with `xdg-mime default`;
2. removes the recorded icons and the well-known icon names;
3. restores a backed-up definition, or removes one the project created;
4. refreshes the MIME and icon databases;
5. removes the entry and the record.

Installing twice is harmless: the second install sees its own ID as the current
default, keeps the recorded previous default rather than recording itself, and
keeps ownership of a definition it created.

## What the project never does

- It never writes outside `$XDG_DATA_HOME`, `$XDG_CONFIG_HOME`, and the
  temporary directory.
- It never needs root.
- It never edits a system MIME definition in `/usr/share/mime`.
- It never changes a file's contents to make it match the type.

## Checking the result

```sh
xdg-mime query filetype sample.vcf       # text/vcard
xdg-mime query default text/vcard        # vcard-handler.desktop
vcard-integrate handler status           # the paths, the defaults, and the record
vcard-integrate handler uninstall        # put it all back
```
