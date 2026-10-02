# scripts

One-off maintenance scripts used while migrating the legacy code; not part of the
analysis pipeline.

- `archive_copy.sh SRC DEST` — copies code files (no outputs, nothing over 1 MiB)
  from a legacy directory into `archive/` ([`archive/README.md`](../archive/README.md)).
- `migrate_paths.py FILE...` — rewrites legacy absolute site paths in migrated
  files to `GXANA_*` lookups (`uv run python scripts/migrate_paths.py <files>`);
  exits 1 when a path has no mapping. Tested by `tests/test_migrate_paths.py`.
