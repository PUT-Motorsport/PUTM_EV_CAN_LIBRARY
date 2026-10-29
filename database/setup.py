"""
PUTM EV CAN - DBC -> C code generator.

Regeneration is driven by the DBC VERSION string:

    VERSION "DDMMYYYY"        e.g. VERSION "29102026"   -> 29 Oct 2026
    VERSION "DDMMYYYY_N"      e.g. VERSION "29102026_1" -> 2nd revision that day

A DBC file is regenerated when:
  * generated .c/.h files are missing,
  * its VERSION differs from the one recorded in codegen_metadata.json,
  * --force is given.

The MD5 hash of the DBC is still recorded, but only to WARN when the content
changed while VERSION was not bumped (the code is NOT regenerated then).

Usage:
    python setup.py              # normal run (used by CMake)
    python setup.py --force      # regenerate everything
    python setup.py --check      # only report status, exit 1 if anything is outdated
    python setup.py --bump FILE  # set VERSION of FILE to today (adds _N if needed)
"""

import argparse
import datetime
import hashlib
import json
import re
import sys
from pathlib import Path

import cantools
import cantools.database
import cantools.database.can.c_source

# --- Configuration & Paths ---
BASE_DIR = Path(__file__).resolve().parent
GENERATED_DIR = BASE_DIR / 'generated'
DBC_DIR = BASE_DIR / 'dbc'
METADATA_FILE = BASE_DIR / 'codegen_metadata.json'
VERSION_HEADER = GENERATED_DIR / 'dbc_versions.h'

METADATA_SCHEMA = 2

VERSION_LINE_RE = re.compile(rb'^\s*VERSION\s+"([^"]*)"', re.MULTILINE)
VERSION_FMT_RE = re.compile(r'^(\d{2})(\d{2})(\d{4})(?:_(\d+))?$')


# ---------------------------------------------------------------------------
# Version handling
# ---------------------------------------------------------------------------
class DbcVersion:
    """Parsed DBC version: DDMMYYYY[_N]."""

    def __init__(self, raw: str):
        m = VERSION_FMT_RE.match(raw.strip())
        if not m:
            raise ValueError(
                f'invalid VERSION "{raw}" - expected "DDMMYYYY" or "DDMMYYYY_N" (e.g. "29102026_1")')
        day, month, year, rev = m.groups()
        try:
            self.date = datetime.date(int(year), int(month), int(day))
        except ValueError as e:
            raise ValueError(f'invalid date in VERSION "{raw}": {e}') from None
        self.revision = int(rev) if rev is not None else 0
        if rev is not None and self.revision < 1:
            raise ValueError(f'invalid revision in VERSION "{raw}" - use _1, _2, ...')
        self.raw = raw.strip()

    def key(self):
        return (self.date, self.revision)

    def __eq__(self, other):
        return isinstance(other, DbcVersion) and self.key() == other.key()

    def __lt__(self, other):
        return self.key() < other.key()

    def __str__(self):
        return self.raw

    def human(self):
        s = self.date.strftime('%d.%m.%Y')
        return f'{s} rev {self.revision}' if self.revision else s


def read_dbc_version(dbc_file: Path) -> DbcVersion:
    data = dbc_file.read_bytes()
    m = VERSION_LINE_RE.search(data)
    if not m:
        raise ValueError('missing VERSION line - add e.g. VERSION "29102026" at the top of the file')
    return DbcVersion(m.group(1).decode('ascii', errors='replace'))


def get_file_hash(filepath: Path):
    try:
        return hashlib.md5(filepath.read_bytes()).hexdigest()
    except FileNotFoundError:
        return None


# ---------------------------------------------------------------------------
# Metadata
# ---------------------------------------------------------------------------
def load_metadata() -> dict:
    if not METADATA_FILE.exists():
        return {}
    try:
        with open(METADATA_FILE, 'r', encoding='utf-8') as f:
            data = json.load(f)
    except (OSError, json.JSONDecodeError):
        print('[WARN] codegen_metadata.json is corrupted - all DBC files will be regenerated.')
        return {}
    if data.get('schema') != METADATA_SCHEMA:
        # Old hash-only format -> forces one regeneration, then migrated.
        return {}
    return data


def save_metadata(metadata: dict):
    metadata['schema'] = METADATA_SCHEMA
    new_text = json.dumps(metadata, indent=4, sort_keys=True) + '\n'
    # Write only on change (keeps git diff clean when nothing was regenerated)
    if METADATA_FILE.exists() and METADATA_FILE.read_text(encoding='utf-8') == new_text:
        return
    with open(METADATA_FILE, 'w', encoding='utf-8') as f:
        f.write(new_text)


# ---------------------------------------------------------------------------
# Decision logic
# ---------------------------------------------------------------------------
def check_dbc(dbc_file: Path, metadata: dict):
    """Returns (needs_generation: bool, reason: str, version: DbcVersion)."""
    version = read_dbc_version(dbc_file)  # raises ValueError on bad format
    current_hash = get_file_hash(dbc_file)

    output_h = GENERATED_DIR / f'{dbc_file.stem}.h'
    output_c = GENERATED_DIR / f'{dbc_file.stem}.c'
    if not output_h.exists() or not output_c.exists():
        return True, 'generated files missing', version

    entry = metadata.get('files', {}).get(dbc_file.name)
    if not entry or 'version' not in entry:
        return True, 'no recorded version', version

    try:
        stored = DbcVersion(entry['version'])
    except ValueError:
        return True, f'recorded version "{entry["version"]}" is invalid', version

    if version == stored:
        if entry.get('hash') and entry['hash'] != current_hash:
            print(f'[WARN] {dbc_file.name}: content changed but VERSION is still "{version}"!')
            print(f'       Bump VERSION (e.g. "{suggest_next_version(version)}") to regenerate, '
                  f'or run with --force.')
        return False, f'up to date (VERSION {version} = {version.human()})', version

    if version < stored:
        print(f'[WARN] {dbc_file.name}: VERSION went BACKWARDS {stored} -> {version}. '
              f'Regenerating anyway.')
        return True, f'version downgraded {stored} -> {version}', version

    return True, f'version changed {stored} -> {version}', version


def suggest_next_version(current: DbcVersion = None) -> str:
    today = datetime.date.today()
    base = today.strftime('%d%m%Y')
    if current is None or current.date != today:
        return base
    return f'{base}_{current.revision + 1}'


# ---------------------------------------------------------------------------
# Generation
# ---------------------------------------------------------------------------
def ensure_output_dir():
    GENERATED_DIR.mkdir(exist_ok=True)


def generate_c_code(dbc_file: Path) -> bool:
    """Generates C/H code using cantools generate function."""
    db_name = dbc_file.stem
    try:
        db = cantools.database.load_file(dbc_file)
        header_content, source_content, *_ = cantools.database.can.c_source.generate(
            database=db,
            database_name=db_name,
            header_name=f'{db_name}.h',
            source_name=f'{db_name}.c',
            fuzzer_source_name=None,
        )
        with open(GENERATED_DIR / f'{db_name}.h', 'w', encoding='utf-8') as f:
            f.write(header_content)
        with open(GENERATED_DIR / f'{db_name}.c', 'w', encoding='utf-8') as f:
            f.write(source_content)
        return True
    except Exception as e:
        print(f'[ERROR] Error generating {dbc_file.name}: {e}', file=sys.stderr)
        return False


def write_version_header(versions: dict):
    """Generates dbc_versions.h so firmware / ROS2 nodes can report the DBC version."""
    lines = [
        '/**',
        ' * @file dbc_versions.h',
        ' * @brief DBC versions used to generate this code.',
        ' * @note DO NOT EDIT. Generated by database/setup.py.',
        ' */',
        '',
        '#ifndef PUTM_DBC_VERSIONS_H',
        '#define PUTM_DBC_VERSIONS_H',
        '',
    ]
    for name in sorted(versions):
        v = versions[name]
        macro = re.sub(r'[^A-Z0-9]', '_', name.upper())
        lines += [
            f'#define {macro}_DBC_VERSION          "{v.raw}"',
            f'#define {macro}_DBC_VERSION_DATE     {v.date.strftime("%Y%m%d")}u  /* YYYYMMDD */',
            f'#define {macro}_DBC_VERSION_REVISION {v.revision}u',
            '',
        ]
    lines += ['#endif /* PUTM_DBC_VERSIONS_H */', '']
    new_content = '\n'.join(lines)
    # Rewrite only on change so the header does not trigger needless rebuilds.
    if not VERSION_HEADER.exists() or VERSION_HEADER.read_text(encoding='utf-8') != new_content:
        VERSION_HEADER.write_text(new_content, encoding='utf-8')


def bump_version(dbc_file: Path) -> int:
    data = dbc_file.read_bytes()
    m = VERSION_LINE_RE.search(data)
    try:
        current = DbcVersion(m.group(1).decode('ascii')) if m else None
    except ValueError:
        current = None
    new = suggest_next_version(current)
    new_line = f'VERSION "{new}"'.encode('ascii')
    if m:
        data = data[:m.start()] + new_line + data[m.end():]
    else:
        data = new_line + b'\r\n\r\n' + data
    dbc_file.write_bytes(data)
    print(f'[OK] {dbc_file.name}: VERSION {current or "-"} -> {new}')
    return 0


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
def main() -> int:
    parser = argparse.ArgumentParser(description='PUTM CAN DBC code generator')
    parser.add_argument('--force', action='store_true', help='regenerate all DBC files')
    parser.add_argument('--check', action='store_true',
                        help='only check, exit code 1 if anything needs regeneration')
    parser.add_argument('--bump', metavar='DBC', help='set VERSION of given DBC file to today')
    args = parser.parse_args()

    if args.bump:
        path = Path(args.bump)
        if not path.exists():
            path = DBC_DIR / args.bump
        if not path.exists():
            print(f'[ERROR] {args.bump} not found', file=sys.stderr)
            return 1
        return bump_version(path)

    ensure_output_dir()
    print('--- CAN Code Generator ---')
    print(f'Mode: version based (VERSION "DDMMYYYY[_N]") - all DBC files in {DBC_DIR.name}/')

    files_to_process = sorted(DBC_DIR.glob('*.dbc'))
    if not files_to_process:
        print(f'[WARN] No .dbc files found in {DBC_DIR}')
        return 0

    metadata = load_metadata()
    metadata.setdefault('files', {})
    errors = 0
    outdated = 0
    versions = {}

    for dbc_path in files_to_process:
        try:
            needed, reason, version = check_dbc(dbc_path, metadata)
        except ValueError as e:
            print(f'[ERROR] {dbc_path.name}: {e}', file=sys.stderr)
            errors += 1
            continue

        versions[dbc_path.stem] = version
        if args.force:
            needed, reason = True, 'forced'

        if not needed:
            print(f'[OK] {dbc_path.name} {reason}.')
            continue

        outdated += 1
        if args.check:
            print(f'[OUTDATED] {dbc_path.name}: {reason}')
            continue

        print(f'[GEN] {dbc_path.name}: {reason} - generating...')
        if generate_c_code(dbc_path):
            metadata['files'][dbc_path.name] = {
                'version': version.raw,
                'hash': get_file_hash(dbc_path),
                'generated_at': datetime.datetime.now().isoformat(timespec='seconds'),
            }
            metadata['last_generated'] = datetime.datetime.now().isoformat(timespec='seconds')
            print(f'[OK] Success: {dbc_path.stem}.c/h (VERSION {version} = {version.human()})')
        else:
            errors += 1

    # Drop entries of DBC files that no longer exist
    existing = {p.name for p in files_to_process}
    for name in list(metadata['files']):
        if name not in existing:
            del metadata['files'][name]

    if not args.check:
        write_version_header(versions)
        save_metadata(metadata)

    if errors:
        print(f'[ERROR] {errors} DBC file(s) failed.', file=sys.stderr)
        return 1
    if args.check and outdated:
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
