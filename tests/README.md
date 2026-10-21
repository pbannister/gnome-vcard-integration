# README for tests

- `scripts/tests-run.sh` runs every `tests/*.sh` in lexical order, runs them
  all even when one fails, and reports a `PASS`/`FAIL` summary; it exits
  nonzero when any test failed.
- Tests are numbered in reserved bands, so lexical order is execution order and
  areas do not collide:
    - `00`–`09` — repository plumbing, the site build, and prompt-contract validation.
    - `10`–`19` — readers and parsers.
    - `20`–`29` — locators and resolution.
    - `30`–`49` — containers and formats.
    - `50`–`59` — live-state.
    - `60`–`79` — command-line and integration.
    - `80`–`99` — lifecycle and release.
- Shared helpers live in `tests/lib/`, are sourced by a test, and are never
  executed by the runner.
- Tool-gated tests skip with `skip_unless_tool <tool>` (from
  `tests/lib/test_helpers.sh`) and the standard `SKIP: missing tool: ...`
  message.
- Test tiers are defined in `prompts/02-workflow.md` §4.1: portable,
  tool-gated, and live-state.
- A test never touches the real system: sandbox `HOME` and XDG paths into a
  temporary directory, serve HTTP on `127.0.0.1`, and use no outside network.
- A test that changes live state does not belong in `make test`.
- `tests/09-prompt-contract.sh` validates the prompt corpus; `tests/10-prompt-validator.sh` is its adversarial corpus, injecting malformed fixtures and asserting each is rejected. `tests/11-release-install.sh` exercises the packaging and installer scripts hermetically, including the refusals. `tests/12-status.sh` checks the orientation command `scripts/status.sh`. `tests/13-corpus-coverage.sh` requires every numbered check in the validator to have a fixture in the adversarial corpus. `tests/14-context-budget.sh` checks that the always-loaded context stays inside the budget `prompts/02-workflow.md` declares.
- `tests/15-vcard-reader.sh` runs the C++ reader test against the synthetic fixtures; `tests/40-contact-render.sh` runs the renderer test.
- `tests/60-command-line-tools.sh` drives `vcard-inspect`, `vcard-render`, and `vcard-integrate`; `tests/90-vcard-handler.sh` and `tests/91-install-handler.sh` register and reverse the handler in a throwaway `HOME`; `tests/95-viewer.sh` opens the GTK4 window on a private Xvfb display.
- `tests/10-prompt-validator.sh` declares the check each rejection exercises; `tests/13-corpus-coverage.sh` keeps the two in step, so a new check cannot land without a case that exercises it.
- Behavioral evaluation of a real agent lives in `evals/`, not here: it is non-deterministic, needs a model, and is scored by hand.
- Keep spec-derived sample inputs in `dataflow.in/` and use them as fixtures.

## Canonical Files

The following filenames are canonical and must not be renamed or duplicated without an explicit task:

- `lib/test_helpers.sh`
