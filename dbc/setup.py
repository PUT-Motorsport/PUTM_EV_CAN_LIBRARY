#!/usr/bin/env python3
"""
@file setup.py
@brief DBC to C code generator for PUTM_EV_CAN_LIBRARY
@details Automatically generates C structures and encode/decode functions
         from CAN database (.dbc) files with STM32 optimization
"""

import cantools
import os
import hashlib
import json
from pathlib import Path

# Bazowy katalog skryptu (czyli PUTM_EV_CAN_LIBRARY/dbc)
BASE_DIR = Path(__file__).resolve().parent

# Ścieżki względem katalogu skryptu
DBC_FILE = BASE_DIR / 'PUTM_CAN_1.dbc'
GENERATED_DIR = BASE_DIR / 'generated'
OUTPUT_H = GENERATED_DIR / 'can_message_defs.h'
OUTPUT_C = GENERATED_DIR / 'can_message_defs.c'
METADATA_FILE = BASE_DIR / 'codegen_metadata.json'
HEADER_GUARD = 'CAN_MESSAGE_DEFS_H'

def ensure_output_dir():
    """Ensure output directory exists"""
    Path(GENERATED_DIR).mkdir(exist_ok=True)

def snake_to_camel(name):
    """Convert snake_case to CamelCase"""
    return ''.join(word.capitalize() for word in name.split('_'))

def get_file_hash(filepath):
    """Calculate MD5 hash of file content"""
    hasher = hashlib.md5()
    try:
        with open(filepath, 'rb') as f:
            buf = f.read()
            hasher.update(buf)
        return hasher.hexdigest()
    except FileNotFoundError:
        return None

def check_and_update_hash(dbc_file, metadata_file):
    """Check if DBC file has changed since last run"""
    current_hash = get_file_hash(dbc_file)
    if not current_hash:
        print(f"Error: DBC file {dbc_file} not found")
        return False

    metadata = {}
    if os.path.exists(metadata_file):
        try:
            with open(metadata_file, 'r') as f:
                metadata = json.load(f)
        except json.JSONDecodeError:
            pass

    last_hash = metadata.get('dbc_hash')

    if current_hash == last_hash:
        print("DBC file unchanged, skipping code generation")
        return False

    metadata['dbc_hash'] = current_hash
    with open(metadata_file, 'w') as f:
        json.dump(metadata, f, indent=4)
        
    return True

def determine_c_type(signal):
    """Determine appropriate C type for CAN signal"""
    if signal.is_float:
        return "float"
    elif signal.minimum is not None and signal.minimum < 0:
        return "int64_t"
    else:
        if signal.length <= 8:
            return "uint8_t"
        elif signal.length <= 16:
            return "uint16_t"
        elif signal.length <= 32:
            return "uint32_t"
        else:
            return "uint64_t"
    
    # Override for boolean signals
    if signal.minimum == 0 and signal.maximum == 1 and signal.length == 1:
        return "bool"

def generate_message_struct(msg):
    """Generate structure definition for a CAN message"""
    struct_name = f"PUTM_CAN_{snake_to_camel(msg.name)}"
    
    content = f"/**\n * @struct {struct_name}\n"
    content += f" * @brief CAN message: {msg.name} (ID: 0x{msg.frame_id:X})\n"
    if msg.comment:
        content += f" * @details {msg.comment}\n"
    content += " */\n"
    content += f"typedef struct {{\n"
    
    for signal in msg.signals:
        c_type = determine_c_type(signal)
        comment = signal.comment or f"Signal: {signal.name}"
        content += f"    {c_type} {signal.name}; /**< {comment} */\n"
    
    content += f"}} {struct_name};\n\n"
    return content

def generate_function_declarations(msg):
    """Generate encode/decode function declarations"""
    struct_name = f"PUTM_CAN_{snake_to_camel(msg.name)}"
    
    content = f"/**\n * @brief Encode {msg.name} message to CAN frame\n"
    content += f" * @param[out] data_out Output buffer (8 bytes)\n"
    content += f" * @param[in] msg Message structure to encode\n */\n"
    content += f"void {msg.name}_encode(uint8_t data_out[8], const {struct_name}* msg);\n\n"
    
    content += f"/**\n * @brief Decode CAN frame to {msg.name} message\n"
    content += f" * @param[in] data_in Input buffer (8 bytes)\n"
    content += f" * @param[out] msg Message structure to populate\n */\n"
    content += f"void {msg.name}_decode(const uint8_t data_in[8], {struct_name}* msg);\n\n"
    
    return content

def generate_stm32_encode_function(msg):
    """Generate encode function optimized for STM32"""
    struct_name = f"PUTM_CAN_{snake_to_camel(msg.name)}"
    
    content = f"/**\n * @brief Encode {msg.name} for STM32 FDCAN\n"
    content += f" * @param[out] data_out Output buffer\n"
    content += f" * @param[in] msg Message to encode\n"
    content += f" */\n"
    content += f"void {msg.name}_encode(uint8_t data_out[8], const {struct_name}* msg)\n{{\n"
    content += f"    // Clear buffer\n    memset(data_out, 0, 8);\n\n"
    content += f"    // STM32 FDCAN optimized encoding\n"
    content += f"    // Message ID: 0x{msg.frame_id:X} - {msg.name}\n"
    
    for signal in msg.signals:
        content += f"    // {signal.name}: start={signal.start}, length={signal.length}\n"
    
    content += f"    // TODO: Implement bit packing for STM32 FDCAN\n"
    content += f"}}\n\n"
    return content

def generate_stm32_decode_function(msg):
    """Generate decode function optimized for STM32"""
    struct_name = f"PUTM_CAN_{snake_to_camel(msg.name)}"
    
    content = f"/**\n * @brief Decode {msg.name} from STM32 FDCAN\n"
    content += f" * @param[in] data_in Input buffer\n"
    content += f" * @param[out] msg Message to populate\n"
    content += f" */\n"
    content += f"void {msg.name}_decode(const uint8_t data_in[8], {struct_name}* msg)\n{{\n"
    content += f"    // STM32 FDCAN optimized decoding\n"
    content += f"    // Message ID: 0x{msg.frame_id:X} - {msg.name}\n"
    
    for signal in msg.signals:
        content += f"    // {signal.name}: start={signal.start}, length={signal.length}\n"
        content += f"    // msg->{signal.name} = (data_in[{signal.start // 8}] >> {signal.start % 8}) & 0x{(1 << signal.length) - 1:02X};\n"
    
    content += f"    // TODO: Implement bit unpacking for STM32 FDCAN\n"
    content += f"}}\n\n"
    return content

def generate_c_code(dbc_file, output_h, output_c):
    """Generate C code from DBC file optimized for STM32"""
    try:
        db = cantools.database.load_file(dbc_file)
    except FileNotFoundError:
        print(f"Error: DBC file {dbc_file} not found")
        return False
    except Exception as e:
        print(f"Error parsing DBC file: {e}")
        return False

    # Generate header file
    h_content = f"""/**
 * @file can_message_defs.h
 * @brief CAN message definitions - Auto-generated from {dbc_file}
 * @warning This file is automatically generated - do not edit manually
 * @details Generated for STM32 FDCAN compatibility
 * 
 * @generated_by DBC Code Generator
 */

#ifndef {HEADER_GUARD}
#define {HEADER_GUARD}

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {{
#endif

"""

    # Structure declarations
    for msg in db.messages:
        h_content += generate_message_struct(msg)
    
    # Function declarations
    for msg in db.messages:
        h_content += generate_function_declarations(msg)

    h_content += """
#ifdef __cplusplus
}
#endif

#endif /* CAN_MESSAGE_DEFS_H */
"""

    with open(output_h, 'w') as f:
        f.write(h_content)

    # Generate source file
    c_content = f"""/**
 * @file can_message_defs.c
 * @brief CAN encode/decode implementations for STM32 FDCAN
 * @warning Implement bit packing/unpacking according to your DBC file
 * 
 * @generated_by DBC Code Generator
 */

#include "can_message_defs.h"

"""

    for msg in db.messages:
        c_content += generate_stm32_encode_function(msg)
        c_content += generate_stm32_decode_function(msg)

    with open(output_c, 'w') as f:
        f.write(c_content)

    return True

if __name__ == '__main__':
    ensure_output_dir()
    
    if not os.path.exists(DBC_FILE):
        print(f"Error: DBC file {DBC_FILE} not found in {os.getcwd()}")
        exit(1)

    # Sprawdź, czy pliki wygenerowane istnieją
    generated_missing = not (OUTPUT_H.exists() and OUTPUT_C.exists())

    # Generuj jeśli:
    #  - DBC się zmienił, albo
    #  - brakuje plików w 'generated'
    if check_and_update_hash(DBC_FILE, METADATA_FILE) or generated_missing:
        if generated_missing:
            print("Generated files missing — regenerating code.")
        else:
            print("Generating STM32-optimized CAN code from DBC...")

        if generate_c_code(DBC_FILE, OUTPUT_H, OUTPUT_C):
            print("CAN code generation successful")
            print(f"   Header: {OUTPUT_H}")
            print(f"   Source: {OUTPUT_C}")
        else:
            print("Code generation failed")
            exit(1)
    else:
        print("✅ DBC unchanged, using existing generated code")