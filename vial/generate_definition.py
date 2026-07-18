#!/usr/bin/env python3
"""
Generates an LZMA-compressed keyboard definition source for Vial compatibility.

Fills in custom USB vendor and product ids, and populates the "User" keys with
macros from AAKBD `layers_vial.c`.

Usage: python3 generate_definition.py vial.json output.c \
           [--vendor-id=0xXXXX] [--product-id=0xXXXX]
           [--macros-cpp=file.i]
"""

import lzma
import json
import sys
import re
import os


def parse_macro_enum(filepath):
    """Extract enum macro member names from preprocessed C (comments stripped)."""
    with open(filepath) as f:
        text = f.read()

    m = re.search(r'enum\s+macro\s*\{', text)
    if not m:
        return []

    start = m.end()
    depth = 1
    i = start
    while i < len(text) and depth > 0:
        c = text[i]
        if c == '{':
            depth += 1
        elif c == '}':
            depth -= 1
        i += 1
    body = text[start:i-1]

    result = []
    for token in re.findall(r'[A-Za-z_]\w*', body):
        if token not in ('enum', 'macro') and token != 'COUNT_OF_MACROS':
            result.append(token)
    return result


def main():
    args = iter(sys.argv[1:])
    infile = None
    outfile = None
    vendor_id = None
    product_id = None
    macros_file = None

    for a in args:
        if a.startswith('--vendor-id='):
            vendor_id = a.split('=', 1)[1]
        elif a.startswith('--product-id='):
            product_id = a.split('=', 1)[1]
        elif a.startswith('--macros-cpp='):
            macros_file = a.split('=', 1)[1]
        elif infile is None:
            infile = a
        elif outfile is None:
            outfile = a

    if not infile or not outfile:
        print(f"Usage: {sys.argv[0]} <vial.json> <output.c>\n"
              "    [--vendor-id=0xXXXX] [--product-id=0xXXXX]\n"
              "    [--macros-cpp=build/macros.i]", file=sys.stderr)
        return 1

    if macros_file:
        if not os.path.isfile(macros_file):
            print(f"Error: macros-cpp file not found: {macros_file}", file=sys.stderr)
            return 1
        if os.path.getsize(macros_file) == 0:
            print(f"Error: macros-cpp file is empty: {macros_file}", file=sys.stderr)
            return 1

    with open(infile) as f:
        content = f.read()

    if vendor_id is not None:
        vendor_id = vendor_id.rstrip('Uu')
        content = re.sub(r'("vendorId"\s*:\s*)"[^"]*"', r'\1"{}"'.format(vendor_id), content)
    if product_id is not None:
        product_id = product_id.rstrip('Uu')
        content = re.sub(r'("productId"\s*:\s*)"[^"]*"', r'\1"{}"'.format(product_id), content)

    user_entries = []
    if macros_file:
        for name in parse_macro_enum(macros_file):
            if len(user_entries) >= 31:
                break
            display_title = name.replace('MACRO_', '', 1).replace('_', ' ').title().replace('Led ', 'LED ').replace('Ipad', 'iPad').replace('Macos', 'macOS').replace('Rgb', 'RGB').replace('Ps2 ', 'PS/2 ')
            user_entries.append({
                'name': display_title,
                #'title': display_title,
                'shortName': display_title.replace(' ', '\n'),
            })

    keycodes = [{'name': 'Fn/Globe', 'shortName': 'Fn'}] + user_entries
    keycodes_json = json.dumps(keycodes)
    if '"customKeycodes"' in content:
        content = re.sub(r'"customKeycodes"\s*:\s*\[.*?\]',
            lambda m: '"customKeycodes": ' + keycodes_json, content, flags=re.DOTALL)
    else:
        content = re.sub(r'("layouts")',
            lambda m: '"customKeycodes": ' + keycodes_json + ',\n    ' + m.group(1), content)

    data = json.dumps(json.loads(content), separators=(',', ':'))
    compressed = lzma.compress(data.encode('utf-8'))

    with open(outfile, 'w') as f:
        f.write("// Auto-generated from {}\n".format(infile))
        f.write("// Do not edit manually.\n\n")
        f.write("#include <stdint.h>\n")
        f.write('#include "progmem.h"\n\n')
        f.write("const uint8_t keyboard_definition[{}] PROGMEM = {{\n".format(len(compressed)))
        for i in range(0, len(compressed), 12):
            chunk = compressed[i:i+12]
            f.write("    {},\n".format(', '.join('0x{:02X}'.format(b) for b in chunk)))
        f.write("};\n")
        f.write("#include <stdint.h>\n")
        f.write(
            "const uint16_t keyboard_definition_size PROGMEM __attribute__((aligned(2))) = {};\n".format(
                len(compressed)))
        f.write(
            "_Static_assert({} <= 65535, \"keyboard_definition_size exceeds uint16_t\");\n".format(
                len(compressed)))

    print("Generated {} ({} bytes compressed)".format(outfile, len(compressed)), file=sys.stderr)
    return 0


if __name__ == '__main__':
    sys.exit(main())
