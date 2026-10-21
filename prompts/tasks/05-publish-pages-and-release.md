# Task: Publish the page set and a release

## TASK-DESCRIPTION
- Create: `scripts/release-publish.sh`
- Create: `tests/81-release-publish.sh`
- Create: `.vscode/settings.json`
- Modify: `Makefile`
- Modify: `README.md`
- Modify: `TODO.md`
- Modify: `PHASES.md`
- Modify: `site.in/index.txt`
- Modify: `prompts/tasks/00-tasks.md`

The task publishes phase 4: the standard page set goes live through the
homelab publisher, and the built artifacts become a GitHub release.

`scripts/release-publish.sh` tags the current commit, pushes the branch and
the tag, and creates or updates the GitHub release with the packaged tarball
and its checksum. It builds from the commit being released, refuses a dirty
tree, refuses a version that does not name the commit, reuses a tag already on
the commit, refuses a tag on another commit, skips a redundant push, and
re-uploads the assets when the release already exists.
`tests/81-release-publish.sh` exercises the success path and the refusals
against a scratch repository with a local bare remote and a stub `gh`, so it
never publishes anything.
The editor configuration is recorded with a workspace-relative CMake source
path, as the contract requires of committed tool configuration.

The homelab registration and the live deploy are the cross-project steps the
owner authorized when ratifying this task:
- add the project to `sources/projects.yaml` in `~/work/00-homelab`;
- run the homelab's `make deploy`, which fetches this project's `site.out/`,
  re-runs the leak gate, and publishes it.

The status pages and `PHASES.md` record the completed phase; the README gains
the repository, release, and published-pages links.

## TASK-OUTPUT
Produce these complete files in this order, then the `VERIFICATION:` line:
1. `scripts/release-publish.sh`
2. `tests/81-release-publish.sh`
3. `.vscode/settings.json`
4. `Makefile`
5. `README.md`
6. `TODO.md`
7. `PHASES.md`
8. `site.in/index.txt`
9. `prompts/tasks/00-tasks.md`

## TASK-CONTEXT
<note>
The capability rules are in `prompts/capabilities/04-release-and-publishing.md`:
a release must be the artifact of the commit it names, the build happens inside
the publish path, the tag is reused or checked, and a publish is re-runnable.
The page requirements are in `prompts/features/02-project-pages.md`.
</note>
<constraint>
The homelab deploy is the only publisher. Do not push pages to the web server
from this project. Before the live deploy, verify SSH access to the target,
confirm the homelab working tree contains only the registry change, and record
the rollback: remove the registry entry and redeploy.
</constraint>

## TASK-FILES

| Operation | Path |
|---|---|
| create | `scripts/release-publish.sh` |
| create | `tests/81-release-publish.sh` |
| create | `.vscode/settings.json` |
| modify | `Makefile` |
| modify | `README.md` |
| modify | `TODO.md` |
| modify | `PHASES.md` |
| modify | `site.in/index.txt` |
| modify | `prompts/tasks/00-tasks.md` |

## TASK-VERIFY
- Run: `make test` from the repository root.
- Expected: exit status 0, with `81-release-publish` reporting `ok`.
- Run: `make site` and `make check` from the repository root.
- Expected: exit status 0, and `leak-gate: clean: site.out`.
- Run: `TAG=v0.1.0 make release-publish` from a clean tree.
- Expected: exit status 0, a tag `v0.1.0` on the released commit, and `gh release view v0.1.0` listing the tarball and `SHA256SUMS`.
- Run: `curl -fsS https://labs.bannister.us/projects/gnome-vcard-integration/`.
- Expected: HTTP 200, a page titled `GNOME VCARD Integration`, and the homelab navigation bar.

## TASK-FEATURES
- `prompts/features/02-project-pages.md`

## TASK-ACCEPTANCE
- `PROJECT-PAGES-R001`
- `PROJECT-PAGES-R016`
- `PROJECT-PAGES-R017`
- `PROJECT-PAGES-R019`

OUTPUT: the complete files in the order listed, ending with the `VERIFICATION:` line.
