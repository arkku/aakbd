#!/usr/bin/env python3
"""
Read a vial.json and output LAYOUT_BIT_* defines, vial_default_layout_options,
and dynamic_keymap_layout_updated with matrix positions from the keymap.

AI-generated for AAKBD <https://github.com/arkku/aakbd> Vial compatibility.

Usage: python3 generate_keymap_layout_bits.py vial.json
"""

import json
import sys
import re
from collections import defaultdict


def bit_count(n_options):
    return max(1, (n_options - 1).bit_length()) if n_options > 0 else 1


def parse_keymap(keymap):
    """Parse KLE keymap, return list of (row, col, label_idx, value) for tagged entries."""
    entries = []
    for row_data in keymap:
        x = 0
        w = h = 1
        for item in row_data:
            if isinstance(item, dict):
                x += item.get('x', 0)
                w = item.get('w', 1)
                h = item.get('h', 1)
            else:
                w = 1
                h = 1
                if '\n\n\n' in item:
                    pos, tag = item.split('\n\n\n')
                    rc = pos.split(',')
                    if len(rc) == 2:
                        row, col = int(rc[0]), int(rc[1])
                        idx, val = tag.split(',')
                        entries.append((row, col, int(idx), int(val), x, w, h))
                x += w
    return entries


def generate_update_condition(values, bits):
    """Given the set of values a position appears for,
    and the number of bits in the label, generate a C expression
    for when the position should be active.

    Returns (expression_string, is_inverted) or (None, False) if unclear.
    """
    all_values = set(range(1 << bits))
    if values == all_values:
        return None  # always present, no update needed

    # For single-bit labels
    if bits == 1:
        if values == {1}:
            return "opts & (1 << LAYOUT_BIT_{code})", False
        elif values == {0}:
            return "!(opts & (1 << LAYOUT_BIT_{code}))", True
        else:
            return None

    # For multi-bit (2 bits), check each sub-bit
    # bit0 = LSB, bit1 = MSB
    # Check if all values where this position appears share a common bit pattern
    b0_set = {v & 1 for v in values}
    b1_set = {(v >> 1) & 1 for v in values}

    conditions = []
    for bit_idx, (bit_set, code_bit) in enumerate([(b0_set, 0), (b1_set, 1)]):
        if bit_set == {1}:
            conditions.append(f"opts & (1 << LAYOUT_BIT_{{code}}_{bit_idx})")
        elif bit_set == {0}:
            conditions.append(f"!(opts & (1 << LAYOUT_BIT_{{code}}_{bit_idx}))")

    if len(conditions) == 1:
        return conditions[0], '!' in conditions[0]
    elif len(conditions) == 2:
        return f"({conditions[0]} && {conditions[1]})", False
    return None


def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <vial.json>", file=sys.stderr)
        sys.exit(1)

    with open(sys.argv[1]) as f:
        data = json.load(f)

    labels = data.get("layouts", {}).get("labels", [])
    if not labels:
        print("No layout labels found.", file=sys.stderr)
        sys.exit(1)

    # Parse labels
    parsed = []
    for item in labels:
        if isinstance(item, list):
            name = item[0]
            opts = item[1:]
        else:
            name = item
            opts = ["Off", "On"]
        parsed.append((name, opts, bit_count(len(opts))))

    total_bits = sum(bit_count(len(opts)) for _, opts, _ in parsed)

    print(f"// Total: {total_bits} bits, mask = 0x{(1 << total_bits) - 1:0{(total_bits + 3) // 4}X}")
    print()

    bit = 0
    entries = []
    for name, opts, _ in reversed(parsed):
        n = len(opts)
        bits = bit_count(n)
        if bits == 1:
            print(f"// Bit {bit}: {name} ({opts[0]}=0, {opts[1]}=1)")
        else:
            opts_str = ", ".join(f"{i}={o}" for i, o in enumerate(opts))
            print(f"// Bits {bit}-{bit + bits - 1}: {name} ({opts_str})")
        entries.append((bit, bits, name, n))
        bit += bits

    print()
    print("// LAYOUT_BIT_* defines (in firmware keymap.c):")
    defines = []
    for item, (bit, bits, name, n) in zip(labels, entries):
        code = re.sub(r'[^A-Za-z0-9]', '_', name).upper()
        code = re.sub(r'_+', '_', code).strip('_')
        if bits == 1:
            defines.append((f"LAYOUT_BIT_{code}", str(bit)))
        else:
            for i in range(bits):
                defines.append((f"LAYOUT_BIT_{code}_{i}", str(bit + i)))
    if defines:
        width = max(len(d[0]) for d in defines)
        for name, val in defines:
            print(f"#define {name:<{width}} {val}")

    print()
    parts = []
    for item, (bit, bits, name, n) in zip(labels, entries):
        code = re.sub(r'[^A-Za-z0-9]', '_', name).upper()
        code = re.sub(r'_+', '_', code).strip('_')
        if bits == 1:
            parts.append(f"    (({code} ? 1 : 0) << LAYOUT_BIT_{code})")
        else:
            for i in range(bits):
                parts.append(f"    (({code}_{i} ? 1 : 0) << LAYOUT_BIT_{code}_{i})")
    if parts:
        print("const uint16_t vial_default_layout_options PROGMEM = (")
        print(" |\n".join(parts))
        print(");")

    # --- dynamic_keymap_layout_updated from keymap entries ---
    print("""
static void
update_keycode (uint8_t keycode, uint8_t row, uint8_t col, bool should_be_active) {
    const uint16_t current = dynamic_keymap_get_qmk_keycode(0, row, col);
    if (should_be_active) {
        if (current == KC_NO) {
            dynamic_keymap_set_qmk_keycode(0, row, col, aakbd_to_qmk(keycode));
        }
    } else if (current != KC_NO) {
        dynamic_keymap_set_qmk_keycode(0, row, col, KC_NO);
    }
}""")

    keymap = data.get("layouts", {}).get("keymap", [])
    tagged = parse_keymap(keymap)

    # Group by label: for each matrix pos within each label, collect values
    # label_idx → (row,col) → set of values
    pos_by_label = defaultdict(lambda: defaultdict(set))
    for row, col, idx, val, _, _, _ in tagged:
        pos_by_label[idx][(row, col)].add(val)

    # For each label, determine which positions change
    print("""
void
dynamic_keymap_layout_updated (const uint16_t old_opts, const uint16_t opts) {""")

    any_updates = False
    for idx in sorted(pos_by_label.keys(), key=int):
        label = labels[int(idx)]
        lname = label[0] if isinstance(label, list) else label
        n_opts = len(label) - 1 if isinstance(label, list) else 2
        bits = bit_count(n_opts)

        # Get the bit range for this label
        bit_start, _, _, _ = entries[int(idx)]

        for (row, col), values in sorted(pos_by_label[idx].items()):
            if len(values) == n_opts:
                continue  # appears for all values → always present

            # Build condition
            if bits == 1:
                bit_def = f"LAYOUT_BIT_{re.sub(r'[^A-Za-z0-9]', '_', lname).upper()}"
                bit_def = re.sub(r'_+', '_', bit_def).strip('_')
                if values == {1}:
                    cond = f"opts & (1 << {bit_def})"
                elif values == {0}:
                    cond = f"!(opts & (1 << {bit_def}))"
                else:
                    continue
            else:
                # Multi-bit: try to find a simple pattern
                cond = None
                sub_conditions = []
                for sub in range(bits):
                    bit_def = f"LAYOUT_BIT_{re.sub(r'[^A-Za-z0-9]', '_', lname).upper()}_{sub}"
                    bit_def = re.sub(r'_+', '_', bit_def).strip('_')
                    sub_set = {v & (1 << sub) for v in values}
                    if sub_set == {1 << sub}:
                        sub_conditions.append(f"opts & (1 << {bit_def})")
                    elif sub_set == {0}:
                        sub_conditions.append(f"!(opts & (1 << {bit_def}))")
                    else:
                        sub_conditions.append(None)

                if all(c is not None for c in sub_conditions):
                    if len(sub_conditions) == 1:
                        cond = sub_conditions[0]
                    else:
                        cond = " && ".join(f"({c})" for c in sub_conditions if c is not None)
                elif any(c is not None for c in sub_conditions):
                    active = [c for c in sub_conditions if c is not None]
                    if active:
                        cond = " || ".join(f"({c})" for c in active)
                    else:
                        cond = None
                # If no single sub-bit matches, check if active for any non-zero value
                if cond is None:
                    missing = set(range(n_opts)) - values
                    code = re.sub(r'[^A-Za-z0-9]', '_', lname).upper()
                    code = re.sub(r'_+', '_', code).strip('_')
                    all_bits = " | ".join(
                        f"(1 << LAYOUT_BIT_{code}_{i})" for i in range(bits))
                    all_bits_or = f"opts & ({all_bits})"
                    if missing == {0}:
                        # Active when any bit is set (non-zero value)
                        cond = all_bits_or
                    elif missing == {n_opts - 1} and n_opts <= (1 << bits):
                        # Only the max value is missing (all bits set) → active when NOT all bits
                        all_bits_and = " && ".join(
                            f"(opts & (1 << LAYOUT_BIT_{code}_{i}))" for i in range(bits))
                        cond = f"!({all_bits_and})"
                    elif 0 not in values:
                        # Active when any bit is set (excludes value 0)
                        cond = all_bits_or
                    else:
                        cond = f"/* TODO: values {sorted(values)} for ({row},{col}) */ 1"

            print(f"    update_keycode(KC_FIXME, {row}, {col}, {cond});")
            any_updates = True

    if not any_updates:
        print("    // No layout-dependent key updates needed")
    print("}")


if __name__ == "__main__":
    main()
