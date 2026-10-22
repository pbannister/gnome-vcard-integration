# Record: real vCard committed and purged — 2026-10-22

## What happened

A real vCard was committed as `3e44627` and pushed to the public repository.
The same commit was the target of the `v2026-10-21-master` tag and release.
The binary release tarball did not contain `dataflow.in`, and the release notes
named no one, but the commit tree and the GitHub source archive did contain the
card.

## Root cause

The privacy boundary was prose, not a gate. `dataflow.in/vcard/` had no
allowlist, so a direct commit could add an unreviewed fixture and the next push
published it. The card had been supplied for a parser bug report and was kept
as an example instead of being replaced by a synthetic fixture that reproduced
its structure.

## Fix (applied, verified)

- Removed the card from the tree and rewrote `master` to drop `3e44627` while
  keeping the later commits; force-pushed the clean history.
- Deleted the exposed tag and release, and republished the clean release as
  `v2026-10-22-master` on `c388c1e`.
- Made the repository private during the rewrite, verified the clean tree and
  tag, and restored public visibility.
- Verified the card is absent from HEAD, the tag, and reachable history, and
  that the release assets are clean.
- Ran `make test`: 23 tests passed, including the new fixture privacy guard.
- GitHub still serves the orphaned commit by SHA. The owner confirmed the data
  is consented/public business data and accepted that residual until GitHub
  garbage-collects it.

## Lesson

- A privacy boundary in prose is not a gate; an allowlist and a test are the
  gate.
- A bug report is not a fixture. Reproduce the structure with invented values
  and keep the supplied card outside the repository.

## Safeguards for any retry

- Only the allowlisted synthetic `.vcf` files may live under
  `dataflow.in/vcard/`; `tests/04-fixture-privacy.sh` fails on a new file.
- Review the content of an allowlisted fixture when it changes; the allowlist
  checks names, not contents.
- Do not push a commit that adds or changes a fixture without that review.
- Keep a supplied real card outside the working tree.

## Current state / next steps

- The repository is public; the current tree, the `v2026-10-22-master` tag, and
  the latest release are clean.
- The orphaned commit remains reachable by SHA until GitHub's garbage
  collection; no further action was requested.
- The live pages were built from the clean `0a91c20` commit and link
  `releases/latest`, which now names the clean release.

## Commits

- `3e44627` the removed commit that carried the real card (not reachable from a ref)
- `c388c1e` the clean master after the rewrite, and the clean release tag
- `77aedc1` the skeleton rule for generated release tags
- `48b128a` the fixture privacy guard and the allowlist
