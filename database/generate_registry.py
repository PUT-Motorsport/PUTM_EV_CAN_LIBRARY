import cantools
import re
from pathlib import Path

BASE_DIR = Path(__file__).resolve().parent
DBC_DIR = BASE_DIR / 'dbc'
SRC_DIR = BASE_DIR.parent / 'src'
OUTPUT_FILE = SRC_DIR / 'dbc_registry.cpp'

def camel_to_snake(name):
    name = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
    name = re.sub('([a-z0-9])([A-Z])', r'\1_\2', name).lower()
    return re.sub(r'_+', '_', name)

def generate_registry():
    print("--- Starting DBC Registry Generation ---")
    
    dbc_files = list(DBC_DIR.glob('*.dbc'))
    if not dbc_files:
        print(f"[ERROR] No .dbc files found in {DBC_DIR}")
        return

    messages = []
    
    for dbc_path in dbc_files:
        prefix = dbc_path.stem.upper()
        print(f"[INFO] Processing {dbc_path.name} (Prefix: {prefix})...")
        
        try:
            db = cantools.database.load_file(dbc_path)
            msg_count = 0
            for msg in db.messages:
                msg_lc = camel_to_snake(msg.name)
                msg_up = msg_lc.upper()
                messages.append((prefix, msg_up, msg_lc))
                msg_count += 1
            print(f"[INFO] Extracted {msg_count} messages from {dbc_path.name}")
        except Exception as e:
            print(f"[ERROR] Failed to parse {dbc_path.name}: {e}")

    messages = sorted(list(set(messages)))

    cpp_content = """/**
 * @file dbc_registry.cpp
 * @brief Automatically generated DBC Registry.
 * @note DO NOT EDIT. Run database/generate_registry.py to update.
 */

#include "message_handler.hpp"
"""
    for dbc_path in dbc_files:
        cpp_content += f'#include "{dbc_path.stem}.h"\n'

    cpp_content += """
namespace putm_ev_can {

template<typename MsgT,
         int(*Pack)(uint8_t*, const MsgT*, size_t),
         int(*Unpack)(MsgT*, const uint8_t*, size_t)>
struct DbcThunk {
    static int pack_generic(uint8_t* d, const void* s, size_t n) {
        return Pack(d, reinterpret_cast<const MsgT*>(s), n);
    }
    static int unpack_generic(void* d, const uint8_t* s, size_t n) {
        return Unpack(reinterpret_cast<MsgT*>(d), s, n);
    }
};

#define DBC_ENTRY(PREFIX, MSG_UP, msg_lc) \\
    { PREFIX##_##MSG_UP##_FRAME_ID, \\
      PREFIX##_##MSG_UP##_LENGTH, \\
      (void*)&DbcThunk<PREFIX##_##msg_lc##_t, \\
                PREFIX##_##msg_lc##_pack, \\
                PREFIX##_##msg_lc##_unpack>::pack_generic, \\
      (void*)&DbcThunk<PREFIX##_##msg_lc##_t, \\
                PREFIX##_##msg_lc##_pack, \\
                PREFIX##_##msg_lc##_unpack>::unpack_generic }

static const DbcEntry DBC_REGISTRY[] = {
"""

    entries = []
    for prefix, msg_up, msg_lc in messages:
        entries.append(f"    DBC_ENTRY({prefix}, {msg_up}, {msg_lc})")
        
    cpp_content += ",\n".join(entries)
    cpp_content += "\n};\n\n"

    cpp_content += """const DbcEntry* find_dbc_entry(PUTM_CAN::CanId id) {
    for (const auto& e : DBC_REGISTRY) {
        if (e.id == id) return &e;
    }
    return nullptr;
}

} // namespace putm_ev_can
"""

    try:
        with open(OUTPUT_FILE, 'w', encoding='utf-8') as f:
            f.write(cpp_content)
        print(f"[SUCCESS] Generated {OUTPUT_FILE.name} with {len(messages)} frames across {len(dbc_files)} buses.")
    except Exception as e:
        print(f"[ERROR] Failed to write {OUTPUT_FILE.name}: {e}")

if __name__ == '__main__':
    generate_registry()