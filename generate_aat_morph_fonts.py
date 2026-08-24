#!/usr/bin/env python3
"""Convert morx fonts to mort and build equivalent test fonts.

The fonts exercise all five metamorphosis subtable types.  They intentionally
use the same glyphs, state machines, and expected results so an implementation
can compare the obsolete and extended table formats directly.

Requires fontTools.  Run this script from any directory; generated fonts are
written below ``fonts/`` next to the other text-rendering-tests fixtures.  Pass
``--convert INPUT OUTPUT`` to use the morx-2.0-to-mort converter independently.
"""

import argparse
from pathlib import Path
import struct

from fontTools.ttLib import TTFont
from fontTools.ttLib.tables.DefaultTable import DefaultTable


ROOT = Path(__file__).resolve().parent
SOURCE_FONT = ROOT / "fonts" / "TestMORXTwo.ttf"
OUTPUTS = {
    "mort": ROOT / "fonts" / "TestAATMort.ttf",
    "morx": ROOT / "fonts" / "TestAATMorx.ttf",
}


def u16(value):
    return struct.pack(">H", value)


def s16(value):
    return struct.pack(">h", value)


def u32(value):
    return struct.pack(">I", value)


def align(data, size):
    return data + bytes(-len(data) % size)


def lookup8(first_glyph, values):
    return (
        u16(8)
        + u16(first_glyph)
        + u16(len(values))
        + b"".join(u16(value) for value in values)
    )


def lookup6(font, mapping):
    """Encode a sparse AAT lookup without assigning values to gaps."""
    pairs = sorted(
        (font.getGlyphID(source), font.getGlyphID(target))
        for source, target in mapping.items()
    )
    count = len(pairs)
    power = 1 << (count.bit_length() - 1) if count else 0
    search_range = power * 4
    entry_selector = power.bit_length() - 1 if power else 0
    range_shift = count * 4 - search_range
    header = (
        u16(6)
        + u16(4)
        + u16(count)
        + u16(search_range)
        + u16(entry_selector)
        + u16(range_shift)
    )
    return header + b"".join(u16(glyph) + u16(value) for glyph, value in pairs)


def class_values(font, mapping):
    values = [1] * len(font.getGlyphOrder())  # 1 is the out-of-bounds class.
    for glyph, glyph_class in mapping.items():
        values[font.getGlyphID(glyph)] = glyph_class
    return values


def flags(action, names):
    value = action.get("reserved", 0)
    for name, bit in names.items():
        if action.get(name, False):
            value |= bit
    return value


def transitions_key(action, kind):
    if kind == 0:
        return (
            action["new_state"],
            flags(
                action,
                {"mark_first": 0x8000, "dont_advance": 0x4000, "mark_last": 0x2000},
            )
            | action.get("verb", 0),
        )
    if kind == 1:
        return (
            action["new_state"],
            flags(action, {"set_mark": 0x8000, "dont_advance": 0x4000}),
            action.get("mark_lookup"),
            action.get("current_lookup"),
        )
    if kind == 2:
        return (
            action["new_state"],
            flags(action, {"set_component": 0x8000, "dont_advance": 0x4000}),
            tuple(
                (item["delta"], item.get("store", False))
                for item in action.get("actions", ())
            ),
        )
    if kind == 5:
        return (
            action["new_state"],
            flags(
                action,
                {
                    "set_mark": 0x8000,
                    "dont_advance": 0x4000,
                    "current_is_kashida": 0x2000,
                    "marked_is_kashida": 0x1000,
                    "current_before": 0x0800,
                    "marked_before": 0x0400,
                },
            ),
            tuple(action.get("current_insert", ())),
            tuple(action.get("marked_insert", ())),
        )
    raise AssertionError(kind)


def unique_transitions(states, kind):
    entries = []
    entry_ids = {}
    state_entries = []
    for state in states:
        row = []
        for action in state:
            key = transitions_key(action, kind)
            entry_id = entry_ids.get(key)
            if entry_id is None:
                entry_id = len(entries)
                entry_ids[key] = entry_id
                entries.append(action)
            row.append(entry_id)
        state_entries.append(row)
    return entries, state_entries


def insertion_actions(entries, font):
    sequences = []
    indexes = {}
    for entry in entries:
        for key in ("current_insert", "marked_insert"):
            sequence = tuple(entry.get(key, ()))
            if sequence and sequence not in indexes:
                indexes[sequence] = len(sequences)
                sequences.extend(font.getGlyphID(glyph) for glyph in sequence)
    return sequences, indexes


def legacy_state(font, kind, classes, states, *, substitutions=(), ligature=None):
    entries, state_entries = unique_transitions(states, kind)
    class_count = len(states[0])
    if class_count == 0 or class_count > 256 or len(entries) > 256:
        raise ValueError("mort state tables are limited to 1-256 classes and entries")
    if any(not 0 <= glyph_class <= 255 for glyph_class in classes):
        raise ValueError("mort class values are limited to 8 bits")

    extras_size = {0: 0, 1: 2, 2: 6, 5: 2}[kind]
    header_size = 8 + extras_size
    class_data = u16(0) + u16(len(classes)) + bytes(classes)
    class_offset = header_size
    state_offset = len(align(bytes(class_offset) + class_data, 2))
    state_data = bytes(entry for row in state_entries for entry in row)
    entry_offset = len(align(bytes(state_offset) + state_data, 2))

    entry_size = {0: 4, 1: 8, 2: 4, 5: 8}[kind]
    tail_offset = entry_offset + len(entries) * entry_size
    extra = b""
    tail = b""

    if kind == 1:
        substitution_offsets = []
        substitution_data = b""
        for lookup in substitutions:
            substitution_offsets.append((tail_offset + len(substitution_data)) // 2)
            values = [0] * len(classes)
            for source, target in lookup.items():
                values[font.getGlyphID(source)] = font.getGlyphID(target)
            substitution_data += b"".join(u16(value) for value in values)
        extra = u16(tail_offset)
        tail = substitution_data

    elif kind == 2:
        action_sequences = []
        action_offsets = {}
        for entry in entries:
            sequence = transitions_key(entry, kind)[2]
            if sequence and sequence not in action_offsets:
                action_offsets[sequence] = None
                action_sequences.append(sequence)
        action_offset = len(align(bytes(tail_offset), 4))
        action_count = sum(len(sequence) for sequence in action_sequences)
        action_size = action_count * 4
        component_offset = action_offset + action_size
        components = list(ligature["components"])
        component_stride = len(components) * 2
        ligature_offset = component_offset + action_count * component_stride
        action_data = b""
        component_data = b""
        action_number = 0
        for sequence in action_sequences:
            action_offsets[sequence] = action_offset + len(action_data)
            if action_offsets[sequence] > 0x3FFF:
                raise ValueError("mort ligature action offset exceeds 14 bits")
            rebased = False
            for index, (delta, store) in enumerate(sequence):
                section_offset = component_offset + action_number * component_stride
                legacy_delta = section_offset // 2 + delta
                if not -(1 << 29) <= legacy_delta < (1 << 29):
                    raise ValueError(
                        "mort ligature action offset exceeds 30 signed bits"
                    )
                value = legacy_delta & 0x3FFFFFFF
                if store:
                    value |= 0x40000000
                last = index == len(sequence) - 1
                if last:
                    value |= 0x80000000
                action_data += u32(value)
                add_base = (store or last) and not rebased
                for component in components:
                    legacy_component = component * 2
                    if add_base:
                        legacy_component += ligature_offset
                    if not 0 <= legacy_component <= 0xFFFF:
                        raise ValueError(
                            "mort ligature component offset exceeds 16 bits"
                        )
                    component_data += u16(legacy_component)
                rebased |= store or last
                action_number += 1
        extra = u16(action_offset) + u16(component_offset) + u16(ligature_offset)
        tail = bytes(action_offset - tail_offset) + action_data
        tail += component_data
        tail += b"".join(u16(font.getGlyphID(glyph)) for glyph in ligature["ligatures"])

    elif kind == 5:
        glyphs, insertion_indexes = insertion_actions(entries, font)
        extra = u16(tail_offset)
        tail = b"".join(u16(glyph) for glyph in glyphs)

    encoded_entries = b""
    for entry in entries:
        new_state = state_offset + entry["new_state"] * class_count
        if kind == 0:
            value = flags(
                entry,
                {"mark_first": 0x8000, "dont_advance": 0x4000, "mark_last": 0x2000},
            ) | entry.get("verb", 0)
            encoded_entries += u16(new_state) + u16(value)
        elif kind == 1:
            value = flags(entry, {"set_mark": 0x8000, "dont_advance": 0x4000})
            mark = entry.get("mark_lookup")
            current = entry.get("current_lookup")
            encoded_entries += u16(new_state) + u16(value)
            encoded_entries += s16(0 if mark is None else substitution_offsets[mark])
            encoded_entries += s16(
                0 if current is None else substitution_offsets[current]
            )
        elif kind == 2:
            value = flags(entry, {"set_component": 0x8000, "dont_advance": 0x4000})
            sequence = transitions_key(entry, kind)[2]
            if sequence:
                value |= action_offsets[sequence]
            encoded_entries += u16(new_state) + u16(value)
        elif kind == 5:
            value = flags(
                entry,
                {
                    "set_mark": 0x8000,
                    "dont_advance": 0x4000,
                    "current_is_kashida": 0x2000,
                    "marked_is_kashida": 0x1000,
                    "current_before": 0x0800,
                    "marked_before": 0x0400,
                },
            )
            current = tuple(entry.get("current_insert", ()))
            marked = tuple(entry.get("marked_insert", ()))
            if len(current) > 31 or len(marked) > 31:
                raise ValueError("mort insertion actions are limited to 31 glyphs")
            value |= len(current) << 5
            value |= len(marked)
            current_index = insertion_indexes.get(current, 0xFFFF)
            marked_index = insertion_indexes.get(marked, 0xFFFF)
            encoded_entries += u16(new_state) + u16(value)
            encoded_entries += u16(current_index) + u16(marked_index)

    header = (
        u16(class_count) + u16(class_offset) + u16(state_offset) + u16(entry_offset)
    )
    body = header + extra + class_data
    body = align(body, 2) + state_data
    body = align(body, 2) + encoded_entries + tail
    if len(body) > 0xFFFF:
        raise ValueError("mort state table exceeds 16 bits")
    return align(body, 4)


def extended_state(font, kind, classes, states, *, substitutions=(), ligature=None):
    entries, state_entries = unique_transitions(states, kind)
    class_count = len(states[0])
    extras_size = {0: 0, 1: 4, 2: 12, 5: 4}[kind]
    header_size = 16 + extras_size
    class_data = lookup8(0, classes)
    class_offset = header_size
    state_offset = len(align(bytes(class_offset) + class_data, 4))
    state_data = b"".join(u16(entry) for row in state_entries for entry in row)
    entry_offset = state_offset + len(state_data)

    entry_size = {0: 4, 1: 8, 2: 6, 5: 8}[kind]
    tail_offset = entry_offset + len(entries) * entry_size
    extra = b""
    tail = b""

    if kind == 1:
        lookup_data = [
            lookup8(
                font.getGlyphID(next(iter(lookup))),
                [font.getGlyphID(next(iter(lookup.values())))],
            )
            for lookup in substitutions
        ]
        substitutions_offset = tail_offset
        offsets_size = 4 * len(lookup_data)
        offsets = []
        data = b""
        for lookup in lookup_data:
            offsets.append(offsets_size + len(data))
            data += lookup
        extra = u32(substitutions_offset)
        tail = b"".join(u32(offset) for offset in offsets) + data

    elif kind == 2:
        action_sequences = []
        action_indexes = {}
        action_count = 0
        for entry in entries:
            sequence = transitions_key(entry, kind)[2]
            if sequence and sequence not in action_indexes:
                action_indexes[sequence] = action_count
                action_sequences.append(sequence)
                action_count += len(sequence)
        action_offset = tail_offset
        component_offset = action_offset + action_count * 4
        ligature_offset = component_offset + len(ligature["components"]) * 2
        action_data = b""
        for sequence in action_sequences:
            for index, (delta, store) in enumerate(sequence):
                value = delta & 0x3FFFFFFF
                if store:
                    value |= 0x40000000
                if index == len(sequence) - 1:
                    value |= 0x80000000
                action_data += u32(value)
        extra = u32(action_offset) + u32(component_offset) + u32(ligature_offset)
        tail = action_data
        tail += b"".join(u16(value) for value in ligature["components"])
        tail += b"".join(u16(font.getGlyphID(glyph)) for glyph in ligature["ligatures"])

    elif kind == 5:
        glyphs, insertion_indexes = insertion_actions(entries, font)
        extra = u32(tail_offset)
        tail = b"".join(u16(glyph) for glyph in glyphs)

    encoded_entries = b""
    for entry in entries:
        new_state = entry["new_state"]
        if kind == 0:
            value = flags(
                entry,
                {"mark_first": 0x8000, "dont_advance": 0x4000, "mark_last": 0x2000},
            ) | entry.get("verb", 0)
            encoded_entries += u16(new_state) + u16(value)
        elif kind == 1:
            value = flags(entry, {"set_mark": 0x8000, "dont_advance": 0x4000})
            mark = entry.get("mark_lookup")
            current = entry.get("current_lookup")
            encoded_entries += u16(new_state) + u16(value)
            encoded_entries += u16(0xFFFF if mark is None else mark)
            encoded_entries += u16(0xFFFF if current is None else current)
        elif kind == 2:
            value = flags(entry, {"set_component": 0x8000, "dont_advance": 0x4000})
            sequence = transitions_key(entry, kind)[2]
            if sequence:
                value |= 0x2000
            encoded_entries += u16(new_state) + u16(value)
            encoded_entries += u16(action_indexes.get(sequence, 0))
        elif kind == 5:
            value = flags(
                entry,
                {
                    "set_mark": 0x8000,
                    "dont_advance": 0x4000,
                    "current_is_kashida": 0x2000,
                    "marked_is_kashida": 0x1000,
                    "current_before": 0x0800,
                    "marked_before": 0x0400,
                },
            )
            current = tuple(entry.get("current_insert", ()))
            marked = tuple(entry.get("marked_insert", ()))
            value |= len(current) << 5
            value |= len(marked)
            encoded_entries += u16(new_state) + u16(value)
            encoded_entries += u16(insertion_indexes.get(current, 0xFFFF))
            encoded_entries += u16(insertion_indexes.get(marked, 0xFFFF))

    header = (
        u32(class_count) + u32(class_offset) + u32(state_offset) + u32(entry_offset)
    )
    body = header + extra + class_data
    body = align(body, 4) + state_data + encoded_entries + tail
    return align(body, 4)


def default_action():
    return {"new_state": 0}


def make_subtables(font):
    rearrange_classes = class_values(font, {"A": 4, "B": 5})
    rearrange_states = [[default_action() for _ in range(6)]]
    rearrange_states[0][4] = {"new_state": 0, "mark_first": True}
    rearrange_states[0][5] = {"new_state": 0, "mark_last": True, "verb": 1}

    contextual_classes = class_values(font, {"C": 4})
    contextual_states = [[default_action() for _ in range(5)]]
    contextual_states[0][4] = {"new_state": 0, "current_lookup": 0}

    ligature_classes = class_values(font, {"E": 4, "F": 5})
    ligature_states = [
        [default_action() for _ in range(6)],
        [default_action() for _ in range(6)],
    ]
    ligature_states[0][4] = {"new_state": 1, "set_component": True}
    ligature_states[1][5] = {
        "new_state": 0,
        "set_component": True,
        "actions": ({"delta": 0}, {"delta": 0}),
    }
    components = [0] * len(font.getGlyphOrder())

    insertion_classes = class_values(font, {"X": 4})
    insertion_states = [[default_action() for _ in range(5)]]
    insertion_states[0][4] = {"new_state": 0, "current_insert": ("Y",)}

    return [
        (0, 0x01, extended_state(font, 0, rearrange_classes, rearrange_states)),
        (
            1,
            0x02,
            extended_state(
                font,
                1,
                contextual_classes,
                contextual_states,
                substitutions=({"C": "D"},),
            ),
        ),
        (
            2,
            0x04,
            extended_state(
                font,
                2,
                ligature_classes,
                ligature_states,
                ligature={
                    "components": components,
                    "ligatures": ("one",),
                },
            ),
        ),
        (4, 0x08, lookup8(font.getGlyphID("G"), [font.getGlyphID("H")])),
        (5, 0x10, extended_state(font, 5, insertion_classes, insertion_states)),
    ]


def morx_table(font):
    subtables = []
    for kind, feature_flags, body in make_subtables(font):
        coverage = 0x20000000 | kind
        length = 12 + len(body)
        subtables.append(u32(length) + u32(coverage) + u32(feature_flags) + body)

    chain_length = 16 + sum(map(len, subtables))
    chain = u32(0x1F) + u32(chain_length) + u32(0) + u32(len(subtables))
    return u32(0x00020000) + u32(1) + chain + b"".join(subtables)


def action_from_morx(action, kind):
    result = {
        "new_state": action.NewState,
        "reserved": getattr(action, "ReservedFlags", 0),
    }
    if kind == 0:
        result.update(
            mark_first=action.MarkFirst,
            dont_advance=action.DontAdvance,
            mark_last=action.MarkLast,
            verb=action.Verb,
        )
    elif kind == 1:
        result.update(
            set_mark=action.SetMark,
            dont_advance=action.DontAdvance,
            mark_lookup=None if action.MarkIndex == 0xFFFF else action.MarkIndex,
            current_lookup=(
                None if action.CurrentIndex == 0xFFFF else action.CurrentIndex
            ),
        )
    elif kind == 2:
        if result["reserved"]:
            raise ValueError(
                "morx ligature reserved flags cannot be represented in mort"
            )
        result.update(
            set_component=action.SetComponent,
            dont_advance=action.DontAdvance,
            actions=tuple(
                {"delta": item.GlyphIndexDelta, "store": item.Store}
                for item in action.Actions
            ),
        )
    elif kind == 5:
        result.update(
            set_mark=action.SetMark,
            dont_advance=action.DontAdvance,
            current_is_kashida=action.CurrentIsKashidaLike,
            marked_is_kashida=action.MarkedIsKashidaLike,
            current_before=action.CurrentInsertBefore,
            marked_before=action.MarkedInsertBefore,
            current_insert=tuple(action.CurrentInsertionAction),
            marked_insert=tuple(action.MarkedInsertionAction),
        )
    else:
        raise ValueError(f"unsupported morx subtable type {kind}")
    return result


def legacy_body_from_morx(font, subtable):
    kind = subtable.MorphType
    substruct = subtable.SubStruct
    if kind == 4:
        return lookup6(font, substruct.Substitution)
    if kind not in {0, 1, 2, 5}:
        raise ValueError(f"unsupported morx subtable type {kind}")

    state_table = substruct.StateTable
    class_count = state_table.GlyphClassCount
    classes = [1] * len(font.getGlyphOrder())
    for glyph, glyph_class in state_table.GlyphClasses.items():
        classes[font.getGlyphID(glyph)] = glyph_class
    states = [
        [
            action_from_morx(state.Transitions[glyph_class], kind)
            for glyph_class in range(class_count)
        ]
        for state in state_table.States
    ]
    if not states:
        raise ValueError("morx state table has no states")

    kwargs = {}
    if kind == 1:
        kwargs["substitutions"] = state_table.PerGlyphLookups
    elif kind == 2:
        kwargs["ligature"] = {
            "components": state_table.LigComponents,
            "ligatures": state_table.Ligatures,
        }
    return legacy_state(font, kind, classes, states, **kwargs)


def mort_coverage(subtable):
    if subtable.Reserved & 0xFFFF:
        raise ValueError("morx subtable reserved field cannot be represented in mort")
    directions = {"Horizontal": 0, "Vertical": 0x8000, "Any": 0x2000}
    orders = {
        "LayoutOrder": 0,
        "ReversedLayoutOrder": 0x4000,
        "LogicalOrder": 0x1000,
        "ReversedLogicalOrder": 0x5000,
    }
    try:
        coverage = directions[subtable.TextDirection]
        coverage |= orders[subtable.ProcessingOrder]
    except KeyError as error:
        raise ValueError(f"unsupported morx coverage value {error.args[0]}") from error
    coverage |= ((subtable.Reserved >> 16) & 0xF) << 8
    return coverage | subtable.MorphType


def mort_data_from_morx(font):
    morx = font["morx"].table
    if morx.Version != 2 or morx.Reserved != 0:
        raise ValueError(
            "only morx 2.0 tables with a zero reserved field are supported"
        )
    chains = []
    for chain in morx.MorphChain:
        features = b"".join(
            u16(feature.FeatureType)
            + u16(feature.FeatureSetting)
            + u32(feature.EnableFlags)
            + u32(feature.DisableFlags)
            for feature in chain.MorphFeature
        )
        subtables = []
        for subtable in chain.MorphSubtable:
            body = legacy_body_from_morx(font, subtable)
            length = 8 + len(body)
            if length > 0xFFFF:
                raise ValueError("mort subtable exceeds 16 bits")
            subtables.append(
                u16(length)
                + u16(mort_coverage(subtable))
                + u32(subtable.SubFeatureFlags)
                + body
            )
        if len(chain.MorphFeature) > 0xFFFF or len(subtables) > 0xFFFF:
            raise ValueError("mort chain count exceeds 16 bits")
        chain_length = 12 + len(features) + sum(map(len, subtables))
        chains.append(
            u32(chain.DefaultFlags)
            + u32(chain_length)
            + u16(len(chain.MorphFeature))
            + u16(len(subtables))
            + features
            + b"".join(subtables)
        )
    return u32(0x00010000) + u32(len(chains)) + b"".join(chains)


def set_names(font, family):
    for record in font["name"].names:
        if record.nameID in {1, 3, 4, 6}:
            suffix = "Regular" if record.nameID in {1, 4} else ""
            value = family + (" " + suffix if suffix else "")
            if record.nameID == 3:
                value = "1.000;UNIC;" + family
            record.string = value.encode(record.getEncoding())


def build_morx(output):
    font = TTFont(SOURCE_FONT, recalcTimestamp=False)
    if "morx" in font:
        del font["morx"]
    if "mort" in font:
        del font["mort"]
    set_names(font, "TestAATMorx")
    table = DefaultTable("morx")
    table.data = morx_table(font)
    font["morx"] = table
    font.recalcTimestamp = False
    font.save(output, reorderTables=False)


def convert_morx_font(input_path, output_path, *, family=None):
    input_path = Path(input_path)
    output_path = Path(output_path)
    if input_path.resolve() == output_path.resolve():
        raise ValueError("input and output paths must differ")
    font = TTFont(input_path, recalcTimestamp=False)
    if "morx" not in font:
        raise ValueError(f"{input_path} has no morx table")
    data = mort_data_from_morx(font)
    del font["morx"]
    if "mort" in font:
        del font["mort"]
    if family is not None:
        set_names(font, family)
    table = DefaultTable("mort")
    table.data = data
    font["mort"] = table
    font.recalcTimestamp = False
    output_path.parent.mkdir(parents=True, exist_ok=True)
    font.save(output_path, reorderTables=False)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--convert",
        nargs=2,
        metavar=("INPUT", "OUTPUT"),
        help="convert an existing morx font to mort instead of generating fixtures",
    )
    args = parser.parse_args()
    if args.convert:
        convert_morx_font(*args.convert)
        print(args.convert[1])
        return

    build_morx(OUTPUTS["morx"])
    print(OUTPUTS["morx"].relative_to(ROOT))
    convert_morx_font(OUTPUTS["morx"], OUTPUTS["mort"], family="TestAATMort")
    print(OUTPUTS["mort"].relative_to(ROOT))


if __name__ == "__main__":
    main()
