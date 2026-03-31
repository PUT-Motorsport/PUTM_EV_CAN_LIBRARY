import cantools
import cantools.database
import cantools.database.can.c_source
import os
import hashlib
import json
from pathlib import Path
import datetime
import sys

# --- Configuration & Paths ---
BASE_DIR = Path(__file__).resolve().parent
GENERATED_DIR = BASE_DIR / 'generated'
DBC_DIR = BASE_DIR / 'dbc'
METADATA_FILE = BASE_DIR / 'codegen_metadata.json'

def ensure_output_dir():
    Path(GENERATED_DIR).mkdir(exist_ok=True)

def get_file_hash(filepath):
    hasher = hashlib.md5()
    try:
        with open(filepath, 'rb') as f:
            hasher.update(f.read())
        return hasher.hexdigest()
    except FileNotFoundError:
        return None

def should_generate(dbc_file):
    current_hash = get_file_hash(dbc_file)
    if not current_hash:
        return False
    
    output_h = GENERATED_DIR / f'{dbc_file.stem}.h'
    output_c = GENERATED_DIR / f'{dbc_file.stem}.c'
    
    # Force generation if files are missing
    if not output_h.exists() or not output_c.exists():
        return True
        
    if METADATA_FILE.exists():
        try:
            with open(METADATA_FILE, 'r') as f:
                metadata = json.load(f)
                return metadata.get(dbc_file.name) != current_hash
        except:
            return True
    return True

def update_metadata(dbc_file):
    metadata = {}
    if METADATA_FILE.exists():
        try:
            with open(METADATA_FILE, 'r') as f:
                metadata = json.load(f)
        except: pass
    metadata[dbc_file.name] = get_file_hash(dbc_file)
    metadata['last_generated'] = datetime.datetime.now().isoformat()
    with open(METADATA_FILE, 'w') as f:
        json.dump(metadata, f, indent=4)

def generate_c_code(dbc_file):
    """Generates C/H code using cantools generate function."""
    db_name = dbc_file.stem
    try:
        # 1. Load Database
        db = cantools.database.load_file(dbc_file)
        
        # 2. Generate Code
        gen_result = cantools.database.can.c_source.generate(
            database=db,
            database_name=db_name,
            header_name=f"{db_name}.h",
            source_name=f"{db_name}.c",
            fuzzer_source_name=None
        )

        header_content = gen_result[0]
        source_content = gen_result[1]

        # 3. Write to files
        with open(GENERATED_DIR / f'{db_name}.h', 'w', encoding='utf-8') as f:
            f.write(header_content)
        with open(GENERATED_DIR / f'{db_name}.c', 'w', encoding='utf-8') as f:
            f.write(source_content)
            
        update_metadata(dbc_file)
        return True
    except Exception as e:
        print(f"[ERROR] Error generating {dbc_file.name}: {e}", file=sys.stderr)
        return False

if __name__ == '__main__':
    ensure_output_dir()
    print(f"--- CAN Code Generator ---")
    print(f"Mode: General (All DBC files in {DBC_DIR.name}/)")

    # Find all .dbc files in the dbc directory automatically
    files_to_process = list(DBC_DIR.glob('*.dbc'))

    if not files_to_process:
        print(f"[WARN] No .dbc files found in {DBC_DIR}")
        # Create empty files if necessary to satisfy CMake, 
        # but better to ensure DBCs are in the right folder.

    for dbc_path in files_to_process:
        if should_generate(dbc_path):
            print(f"[GEN] Generating code for {dbc_path.name}...")
            if generate_c_code(dbc_path):
                print(f"[OK] Success: {dbc_path.stem}.c/h")
        else:
            print(f"[OK] {dbc_path.name} is up to date.")