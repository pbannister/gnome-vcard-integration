# Task: Implement the VCARD Handler feature

## TASK-DESCRIPTION
- Create: `sources/integration/handler_integrator.h`
- Create: `sources/integration/handler_integrator.cpp`
- Create: `sources/tools/vcard_integrate.cpp`
- Create: `sources/tools/icons/vcard-handler.svg`
- Create: `scripts/program-install.sh`
- Create: `scripts/release-stage.sh`
- Create: `tests/handler_integrator_test.cpp`
- Create: `tests/90-vcard-handler.sh`
- Create: `tests/91-install-handler.sh`
- Modify: `sources/CMakeLists.txt`
- Modify: `Makefile`

The handler implements the requirements in
`prompts/features/05-vcard-handler.md`.
It writes the MIME definition, the desktop entry, the handler icon and the two
type icons, and the record that makes an uninstall possible. A definition the
project did not write is backed up and restored byte for byte. The registration
is reversible with `vcard-integrate handler uninstall`, and `make install`
registers the handler after the programs are in place unless
`VCARD_INTEGRATION_NO_HANDLER=1` is set.

## TASK-OUTPUT
Produce these complete files in this order, then the `VERIFICATION:` line:
1. `sources/integration/handler_integrator.h`
2. `sources/integration/handler_integrator.cpp`
3. `sources/tools/vcard_integrate.cpp`
4. `sources/tools/icons/vcard-handler.svg`
5. `scripts/program-install.sh`
6. `scripts/release-stage.sh`
7. `tests/handler_integrator_test.cpp`
8. `tests/90-vcard-handler.sh`
9. `tests/91-install-handler.sh`
10. `sources/CMakeLists.txt`
11. `Makefile`

## TASK-CONTEXT
<note>
`documents/11-mime-type-association.md` records how the desktop resolves a file
type to an application, and exactly what this project writes. shared-mime-info
already defines `text/vcard` and aliases `text/x-vcard` to it, so the project
installs its own definition at the user level only to give the type this
project's icon.
</note>
<constraint>
Every test that registers the handler runs against a throwaway `HOME` and
`XDG_DATA_HOME`; no test may change the machine's own desktop.
</constraint>

## TASK-FILES

| Operation | Path |
|---|---|
| create | `sources/integration/handler_integrator.h` |
| create | `sources/integration/handler_integrator.cpp` |
| create | `sources/tools/vcard_integrate.cpp` |
| create | `sources/tools/icons/vcard-handler.svg` |
| create | `scripts/program-install.sh` |
| create | `scripts/release-stage.sh` |
| create | `tests/handler_integrator_test.cpp` |
| create | `tests/90-vcard-handler.sh` |
| create | `tests/91-install-handler.sh` |
| modify | `sources/CMakeLists.txt` |
| modify | `Makefile` |

## TASK-VERIFY
- Run: `make test` from the repository root.
- Expected: exit status 0, with `90-vcard-handler` and `91-install-handler` reporting `ok`.

## TASK-FEATURES
- `prompts/features/05-vcard-handler.md`

## TASK-ACCEPTANCE
- `VCARD-HANDLER-R001`
- `VCARD-HANDLER-R002`
- `VCARD-HANDLER-R003`
- `VCARD-HANDLER-R009`
- `VCARD-HANDLER-R011`
- `VCARD-HANDLER-R014`
- `VCARD-HANDLER-R016`
- `VCARD-HANDLER-R021`

OUTPUT: the complete files in the order listed, ending with the `VERIFICATION:` line.
