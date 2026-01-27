import cantools
import cantools.database
# Import modułu generatora
import cantools.database.can.c_source
import os
import hashlib
import json
from pathlib import Path
import datetime
import sys
import re

# --- Konfiguracja ścieżek ---
BASE_DIR = Path(__file__).resolve().parent
# Szukamy CMakeLists.txt poziom wyżej
CMAKE_FILE = BASE_DIR.parent / 'CMakeLists.txt' 
GENERATED_DIR = BASE_DIR / 'generated'
METADATA_FILE = BASE_DIR / 'codegen_metadata.json'

def get_backend_from_cmake():
    """Odczytuje PUTM_CAN_BACKEND z pliku CMakeLists.txt."""
    if not CMAKE_FILE.exists():
        print(f"Ostrzeżenie: Nie znaleziono {CMAKE_FILE}. Używam domyślnego: STM32.")
        return "STM32"
    
    with open(CMAKE_FILE, 'r', encoding='utf-8') as f:
        content = f.read()
        match = re.search(r'set\s*\(\s*PUTM_CAN_BACKEND\s+([A-Za-z0-9_]+)\s*\)', content)
        if match:
            return match.group(1)
    return "STM32"

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
    """Generuje kod C/H używając funkcji generate (poprawione argumenty)."""
    db_name = dbc_file.stem
    try:
        # 1. Wczytanie bazy danych
        db = cantools.database.load_file(dbc_file)
        
        # 2. Generowanie kodu
        # UWAGA: Usunięto parametr fuzzer_header_name, który powodował błąd.
        gen_result = cantools.database.can.c_source.generate(
            database=db,
            database_name=db_name,
            header_name=f"{db_name}.h",
            source_name=f"{db_name}.c",
            fuzzer_source_name=None
        )

        # gen_result to krotka. Pierwsze dwa elementy to zawsze header i source.
        header_content = gen_result[0]
        source_content = gen_result[1]

        # 3. Zapis do plików
        with open(GENERATED_DIR / f'{db_name}.h', 'w', encoding='utf-8') as f:
            f.write(header_content)
        with open(GENERATED_DIR / f'{db_name}.c', 'w', encoding='utf-8') as f:
            f.write(source_content)
            
        update_metadata(dbc_file)
        return True
    except Exception as e:
        print(f"❌ Błąd generowania {dbc_file.name}: {e}", file=sys.stderr)
        import traceback
        traceback.print_exc()
        return False

if __name__ == '__main__':
    ensure_output_dir()
    backend = get_backend_from_cmake()
    print(f"--- CAN Code Generator ---")
    print(f"Backend: {backend}")

    # Lista plików zależna od backendu
    files_to_process = [BASE_DIR / 'PUTM_CAN_1.dbc']
    if backend == "ROS2":
        files_to_process.append(BASE_DIR / 'PUTM_CAN_2.dbc')

    for dbc_path in files_to_process:
        if not dbc_path.exists():
            print(f"❌ BŁĄD: Nie znaleziono {dbc_path.name}")
            continue

        if should_generate(dbc_path):
            print(f"⚙️ Generowanie kodu dla {dbc_path.name}...")
            if generate_c_code(dbc_path):
                print(f"✅ Sukces: {dbc_path.stem}.c/h")
        else:
            print(f"✅ {dbc_path.name} jest aktualny.")