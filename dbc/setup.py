"""
@file generuj_kod_c.py
@brief Uiwersalny generator kodu C z plików DBC przy użyciu 'cantools'.
@details Automatycznie generuje kompletne funkcje 'pack' i 'unpack'
         dla plików .dbc.
         
         Instrukcje:
         Windows:
           1. py -m pip install cantools
           2. py generuj_kod_c.py
         
         Linux (Ubuntu):
           1. pip3 install cantools
           2. python3 generuj_kod_c.py
"""
# Importujemy 'cantools' jako główną bibliotekę.
# To jest preferowana, uniwersalna metoda.
import cantools
import os
import hashlib
import json
from pathlib import Path
import datetime
import sys

# --- Konfiguracja ---
BASE_DIR = Path(__file__).resolve().parent
DBC_FILE = BASE_DIR / 'PUTM_CAN_1.dbc'
GENERATED_DIR = BASE_DIR / 'generated'

# Nazwa bazy danych (pobrana z nazwy pliku DBC)
DATABASE_NAME = DBC_FILE.stem

# Ścieżki wyjściowe
OUTPUT_H = GENERATED_DIR / f'{DATABASE_NAME}.h'
OUTPUT_C = GENERATED_DIR / f'{DATABASE_NAME}.c'
METADATA_FILE = BASE_DIR / 'codegen_metadata.json'

# --- Funkcje pomocnicze ---

def ensure_output_dir():
    """Upewnia się, że katalog 'generated' istnieje."""
    Path(GENERATED_DIR).mkdir(exist_ok=True)

def get_file_hash(filepath):
    """Oblicza hash MD5 pliku."""
    hasher = hashlib.md5()
    try:
        with open(filepath, 'rb') as f:
            buf = f.read()
            hasher.update(buf)
        return hasher.hexdigest()
    except FileNotFoundError:
        return None

def check_and_update_hash(dbc_file, metadata_file):
    """
    Sprawdza hash MD5 pliku DBC.
    Zwraca True, jeśli plik się zmienił lub jeśli plik metadanych nie istnieje.
    Zwraca False, jeśli hash się zgadza (brak zmian).
    """
    current_hash = get_file_hash(dbc_file)
    if not current_hash:
        print(f"BŁĄD: Nie znaleziono pliku DBC: {dbc_file}", file=sys.stderr)
        return False # Nie można kontynuować bez pliku

    metadata = {}
    if os.path.exists(metadata_file):
        try:
            with open(metadata_file, 'r') as f:
                metadata = json.load(f)
        except json.JSONDecodeError:
            pass # Uszkodzony JSON, wygeneruj ponownie

    last_hash = metadata.get('dbc_hash')

    if current_hash == last_hash:
        return False # Hash się zgadza, brak zmian

    # Hash się różni, zaktualizuj plik metadanych
    print(f"Plik DBC {dbc_file.name} uległ zmianie. Aktualizowanie hasha...")
    metadata['dbc_hash'] = current_hash
    metadata['last_generated'] = datetime.datetime.now().isoformat()
    with open(metadata_file, 'w') as f:
        json.dump(metadata, f, indent=4)
        
    return True # Plik się zmienił

def generate_c_code(dbc_file, output_h, output_c, database_name):
    """
    Generuje kompletny kod C z pliku DBC używając 'cantools'.
    """
    try:
        # 1. Wczytaj plik .dbc
        #    Używamy pełnej ścieżki: cantools.db.load_file
        db = cantools.db.load_file(dbc_file, floating_point_numbers=True)
        
        # 2. Utwórz instancję generatora C
        #    Używamy pełnej ścieżki: cantools.gen.CGenerator
        generator = cantools.gen.CGenerator(db, database_name)

        # 3. Wygeneruj zawartość plików .c i .h jako stringi
        header_code = generator.generate_header()
        source_code = generator.generate_source()

        # 4. Zapisz wygenerowany kod do plików
        with open(output_h, 'w', encoding='utf-8') as f:
            f.write(header_code)

        with open(output_c, 'w', encoding='utf-8') as f:
            f.write(source_code)
            
        return True

    # --- POPRAWKA TUTAJ ---
    # Zmieniamy 'cantools.db.ParseError' na 'cantools.ParseError'
    except cantools.ParseError as e:
        print(f"BŁĄD parsowania pliku DBC: {e}", file=sys.stderr)
        return False
    except Exception as e:
        print(f"Nieoczekiwany błąd podczas generowania kodu: {e}", file=sys.stderr)
        return False

# --- Główny blok wykonawczy ---

if __name__ == '__main__':
    # Upewnij się, że 'cantools' jest zaimportowane
    if 'cantools' not in sys.modules:
        print("BŁĄD: Biblioteka 'cantools' nie jest zainstalowana.", file=sys.stderr)
        print("Uruchom: pip install cantools (lub py -m pip install cantools)", file=sys.stderr)
        exit(1)

    ensure_output_dir()
    
    if not os.path.exists(DBC_FILE):
        print(f"BŁĄD: Plik DBC {DBC_FILE} nie istnieje.", file=sys.stderr)
        exit(1)

    # Sprawdź, czy pliki wygenerowane istnieją
    generated_missing = not (OUTPUT_H.exists() and OUTPUT_C.exists())

    # Generuj jeśli:
    #  - Plik DBC się zmienił (check_and_update_hash zwróci True), LUB
    #  - brakuje plików w 'generated'
    if check_and_update_hash(DBC_FILE, METADATA_FILE) or generated_missing:
        if generated_missing:
            print("Brak wygenerowanych plików. Uruchamiam generator kodu...")
        else:
            print(f"Generowanie kodu C z {DBC_FILE.name}...")

        if generate_c_code(DBC_FILE, OUTPUT_H, OUTPUT_C, DATABASE_NAME):
            print("✅ Pomyślnie wygenerowano kod C.")
            print(f"   Nagłówek: {OUTPUT_H.relative_to(BASE_DIR.parent)}")
            print(f"   Źródło:   {OUTPUT_C.relative_to(BASE_DIR.parent)}")
            print("\n-------------------------------------------------")
            print("  WAŻNE:")
            print("  Upewnij się, że Twój kod C/C++ dołącza plik:")
            print(f'  #include "generated/{DATABASE_NAME}.h"')
            print("-------------------------------------------------")
        else:
            print("❌ BŁĄD: Generowanie kodu nie powiodło się.")
            exit(1)
    else:
        print(f"✅ Plik {DBC_FILE.name} bez zmian, kod jest aktualny.")