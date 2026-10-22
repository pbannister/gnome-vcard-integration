# Record: Adopt the shared rules

## Outcome

This project now pins the shared rules at `2026-10-22.13`. `rules.lock` holds the
version and a SHA-256 per vendored file, `scripts/rules-check.sh` verifies the
copy, and `tests/16-rules-pin.sh` runs it in the suite.

The adoption was `breaking`: the contract gains a non-overridable safety
precedence level, modification becomes authorization-based, and nine capability
conventions apply.

The validator found only three things, because this project had tracked the rules
closely. It is the closest any project has come to adopting without conformance
work.

## Decisions

- **The test helper was split before it could be lost.** `tests/lib/test_helpers.sh`
  is a vendored path, and this project had extended it with `build_program`, the
  repository root, and the build and fixture directories. Those moved into
  `tests/lib/project_helpers.sh`, which the shared helper sources — the extension
  point added in `2026-10-22.13` for exactly this, after its sibling project lost
  nineteen tests the same way.
- **`AGENTS.md` gained a Shared Rules section.** This project already had an
  entry point, written before the rules were pinned; it now says where the rules
  come from, that vendored files are not editable here, and that this project's
  test additions live in `project_helpers.sh`.
- **Two glossary references were updated** to `01-universal-rules.md`, the
  name introduced by `2026-10-22.9`.
- **`PHASES.md` is declared** in `prompts/10-project.md`.
- **`scripts/skeleton-diff.sh` carried a stale path.** Its file list still named
  `prompts/common/02-universal-rules.md`, so its own self-test failed once the
  suite ran. One line fixed it. This is the cost of keeping two drift mechanisms:
  every skeleton-lineage project with this script fails the same way after the
  rename, and each needs the same local edit. `rules-check` supersedes it, and
  retiring the pair (`scripts/skeleton-diff.sh` and `tests/08-skeleton-diff.sh`)
  would end the class rather than patch instances.

## Verification

- `rules-conform`: nothing to conform.
- `rules-check`: ok (v2026-10-22.13, 39 files, 0 deviations).
- `make test`: 24 test scripts, all passed.

## Behavioral evidence

**Not run — and this is the project where it matters most.** All six eval cases
are runnable here: this project carries both `scripts/status.sh` and
`scripts/site-state-fetch.sh`, which are the props the others lack. It is the
first project where the full behavioral bar applies.

So the portability gap recorded in
`00-project-rules/documents/eval-portability.md` is not academic: there is now a
project that can produce a complete behavioral result, and none has been
produced. If one is run anywhere, run it here.

## What this adoption cost the library

Nothing — it uses the extension point from `.13`, which is the return on the
previous adoption's defect.

## Commits

- Adoption commit — the vendored rules, the split helper, and this record.
