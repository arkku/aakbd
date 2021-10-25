/**
 * keycodes.h: Keycodes for remapping, layers, and macros.
 *
 * Copyright (c) 2021 Kimmo Kulovesi, https://arkku.dev/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KK_KEYCODES_H
#define KK_KEYCODES_H

#include <stdint.h>
#include "usb_keys.h"

typedef uint16_t keycode_t;

/// Plain key, no modifiers.
#define KEY(name)                   PASTE(USB_KEY_, name)

/// Plain key, on modifiers.
#define PLAIN(name)                 KEY(name)

/// A special keycode that causes the keypress to have no effect.
#define NONE                        (0xFFU)

/// A special keycode that passes through to lower layers.
#define PASS                        (0x00U)

/// Shift + key, e.g., `SHIFT(RETURN)`
#define SHIFT(key)                  (MODS_SHIFT | KEY(key))

/// Ctrl + key, e.g., `CTRL(C)`
#define CTRL(key)                   (MODS_CTRL | KEY(key))

/// Alt + key, e.g., `ALT(F4)`
#define ALT(key)                    (MODS_ALT | KEY(key))

/// AltGr + key, e.g., `ALTGR(4)`
#define ALTGR(key)                  (MODS_ALTGR | KEY(key))

/// Cmd + key, e.g., `CMD(W)`
#define CMD(key)                    (MODS_CMD | KEY(key))

/// Windows + key, e.g., `WIN(W)`
#define WIN(key)                    (MODS_WIN | KEY(key))

/// Meta + key, e.g., `META(W)`
#define META(key)                   (MODS_META | KEY(key))

/// Right Shift + key, e.g., `RIGHT_SHIFT(RETURN)`
#define RIGHT_SHIFT(key)            (MODS_RIGHT_SHIFT | KEY(key))

/// Right Ctrl + key, e.g., `RIGHT_CTRL(C)`
#define RIGHT_CTRL(key)             (MODS_RIGHT_CTRL | KEY(key))

/// Right Cmd + key, e.g., `RIGHT_CMD(W)`
#define RIGHT_CMD(key)              (MODS_RIGHT_CMD | KEY(key))

/// Ctrl + Shift + key
#define CTRL_SHIFT(key)             (MODS_CTRL_SHIFT | KEY(key))

/// Shift + Alt + key
#define SHIFT_ALT(key)              (MODS_SHIFT_ALT | KEY(key))

/// Right Shift + AltGr + key
#define SHIFT_ALTGR(key)            (MODS_SHIFT_ALTGR | KEY(key))

/// Cmd + Shift + key
#define CMD_SHIFT(key)              (MODS_CMD_SHIFT | KEY(key))

/// Ctrl + Alt + key
#define CTRL_ALT(key)               (MODS_CTRL_ALT | KEY(key))

/// Cmd + Alt + key
#define CMD_ALT(key)                (MODS_CMD_ALT | KEY(key))

/// Ctrl + Alt + Shift + key
#define CTRL_ALT_SHIFT(key)         (MODS_CTRL_ALT_SHIFT | KEY(key))

/// Cmd + Alt + Shift + key
#define CMD_ALT_SHIFT(key)          (MODS_CMD_ALT_SHIFT | KEY(key))

/// Ctrl + Alt + Cmd + key
#define CTRL_ALT_CMD(key)           (MODS_CTRL_ALT_CMD | KEY(key))

/// Ctrl + Alt + Cmd + Shift + key
#define CTRL_ALT_CMD_SHIFT(key)     (MODS_CTRL_ALT_CMD_SHIFT | KEY(key))

/// A shortcut for the combination of Ctrl + Alt + Shift + Cmd. Note that
/// the extended key `EXT(HYPER)` is in some ways better, but it can only be
/// used standalone (i.e., not in a command like `MOD_OR_KEY(HYPER(ESC))`).
#define KEY_HYPER                   CTRL_ALT_SHIFT(CMD)

/// A shortcut for the combination of Ctrl + Alt + Shift. Note that the
/// extended key `EXT(MEH)` is in some ways better, but it can only be used
/// standalone (i.e., not in a command like `MOD_OR_KEY(MEH(ESC))`).
#define KEY_MEH                     CTRL_ALT(SHIFT)

/// Ctrl + Alt + Cmd + Shift + key
#define HYPER(key)                  CTRL_ALT_CMD_SHIFT(key)

/// Ctrl + Alt + Shift + key
#define MEH(key)                    CTRL_ALT_SHIFT(key)

/// Act as Ctrl while held down, or send a plain key on release if no other
/// keys were pressed while holding, e.g., `CTRL_OR(ESC)`.
#define CTRL_OR(key)                (MOD_OR_KEY(CTRL(key)))

/// Act as Shift while held down, or send a plain key on release if no other
/// keys were pressed while holding.
#define SHIFT_OR(key)               (MOD_OR_KEY(SHIFT(key)))

/// Act as Alt while held down, or send a plain key on release if no other
/// keys were pressed while holding.
#define ALT_OR(key)                 (MOD_OR_KEY(ALT(key)))

/// Act as Win while held down, or send a plain key on release if no other
/// keys were pressed while holding.
#define WIN_OR(key)                 (MOD_OR_KEY(WIN(key)))

/// Act as Cmd while held down, or send a plain key on release if no other
/// keys were pressed while holding.
#define CMD_OR(key)                 (MOD_OR_KEY(CMD(key)))

/// Act as Meta while held down, or send a plain key on release if no other
/// keys were pressed while holding.
#define META_OR(key)                (MOD_OR_KEY(META(key)))

/// Act as AltGr while held down, or send a plain key on release if no other
/// keys were pressed while holding.
#define ALTGR_OR(key)               (MOD_OR_KEY(ALTGR(key)))

/// Act as Right Shift while held down, or send a plain key on release if no other
/// keys were pressed while holding.
#define RIGHT_SHIFT_OR(key)         (MOD_OR_KEY(RIGHT_SHIFT(key)))

/// Act as Right Shift while held down, or send a plain key on release if no other
/// keys were pressed while holding.
#define RIGHT_CTRL_OR(key)          (MOD_OR_KEY(RIGHT_CTRL(key)))

/// Act as Alt while held down, or send a plain key on release if no other
/// Act as Right Shift while held down, or send a plain key on release if no other
/// keys were pressed while holding.
#define RIGHT_CMD_OR(key)           (MOD_OR_KEY(RIGHT_CMD(key)))

/// Act as Ctrl + Shift while held down, or send a plain key on release if
/// no other keys were pressed while holding.
#define CTRL_SHIFT_OR(key)          (MOD_OR_KEY(CTRL_SHIFT(key)))

/// Act as Shift + Alt while held down, or send a plain key on release if
/// no other keys were pressed while holding.
#define SHIFT_ALT_OR(key)           (MOD_OR_KEY(SHIFT_ALT(key)))

/// Act as Right Shift + AltGr while held down, or send a plain key on release if
/// no other keys were pressed while holding.
#define SHIFT_ALTGR_OR(key)         (MOD_OR_KEY(SHIFT_ALTGR(key)))

/// Act as Cmd + Shift while held down, or send a plain key on release if
/// no other keys were pressed while holding.
#define CMD_SHIFT_OR(key)           (MOD_OR_KEY(CMD_SHIFT(key)))

/// Act as Ctrl + Alt while held down, or send a plain key on release if
/// no other keys were pressed while holding.
#define CTRL_ALT_OR(key)            (MOD_OR_KEY(CTRL_ALT(key)))

/// Act as Cmd + Alt while held down, or send a plain key on release if
/// no other keys were pressed while holding.
#define CMD_ALT_OR(key)             (MOD_OR_KEY(CMD_ALT(key)))

/// Act as Ctrl + Alt + Shift while held down, or send a plain key on release if
/// no other keys were pressed while holding.
#define CTRL_ALT_SHIFT_OR(key)      (MOD_OR_KEY(CTRL_ALT_SHIFT(key)))

/// Act as Cmd + Alt + Shift while held down, or send a plain key on release if
/// no other keys were pressed while holding.
#define CMD_ALT_SHIFT_OR(key)       (MOD_OR_KEY(CMD_ALT_SHIFT(key)))

/// Act as Ctrl + Alt + Cmd while held down, or send a plain key on release if
/// no other keys were pressed while holding.
#define CTRL_ALT_CMD_OR(key)        (MOD_OR_KEY(CTRL_ALT_CMD(key)))

/// Act as Ctrl + Alt + Cmd + Shift while held down, or send a plain key on release if
/// no other keys were pressed while holding.
#define CTRL_ALT_CMD_SHIFT_OR(key)  (MOD_OR_KEY(CTRL_ALT_CMD_SHIFT(key)))

/// Act as Ctrl + Alt + Cmd + Shift while held down, or send a plain key on release if
/// no other keys were pressed while holding.
#define HYPER_OR(key)               CTRL_ALT_CMD_SHIFT_OR(key)

/// Act as Ctrl + Alt + Shift while held down, or send a plain key on release if
/// no other keys were pressed while holding.
#define MEH_OR(key)                 CTRL_ALT_SHIFT_OR(key)

/// Toggle layer number (1 - LAYER_COUNT) on or off.
#define LAYER_TOGGLE(num)           LAYER_COMMAND(TOGGLE, ON_RELEASE, (num))

/// Toggle layer while the key is held down.
#define LAYER_TOGGLE_HOLD(num)      LAYER_COMMAND(TOGGLE, ON_HOLD, (num))

/// Toggle layer while the key is held down, and leave toggled if no other
/// key was pressed while the key was held down.
#define LAYER_TOGGLE_STICKY(num)    LAYER_COMMAND(TOGGLE, ON_HOLD_KEEP_IF_NO_KEYPRESS, (num))

/// Enable layer while the key is held down.
#define LAYER_ON_HOLD(num)          LAYER_COMMAND(ENABLE, ON_HOLD, (num))

/// Enable layer while the key is held down, and leave on if no other key
/// was pressed while the key was held down. The layer should have another key
/// to disable the layer or this change may be permanent.
#define LAYER_ON_STICKY(num)        LAYER_COMMAND(ENABLE, ON_HOLD_KEEP_IF_NO_KEYPRESS, (num))

/// Disable layer while the key is held down.
#define LAYER_OFF_HOLD(num)         LAYER_COMMAND(DISABLE, ON_HOLD, (num))

/// Disable layer while the key is held down, and leave off if no other key
/// was pressed while the key was held down. The layer should have another key
/// to enable the layer or this change may be permanent.
#define LAYER_OFF_STICKY(num)       LAYER_COMMAND(DISABLE, ON_HOLD_KEEP_IF_NO_KEYPRESS, (num))

/// Activate both Shift and a layer while the key is held down.
#define SHIFT_AND_LAYER(num)                LAYER_AND_MOD_ON_HOLD(num, SHIFT)

/// Activate both Ctrl and a layer while the key is held down.
#define CTRL_AND_LAYER(num)                 LAYER_AND_MOD_ON_HOLD(num, CTRL)

/// Activate both Alt and a layer while the key is held down.
#define ALT_AND_LAYER(num)                  LAYER_AND_MOD_ON_HOLD(num, ALT)

/// Activate both Win and a layer while the key is held down.
#define WIN_AND_LAYER(num)                  LAYER_AND_MOD_ON_HOLD(num, WIN)

/// Activate both Cmd and a layer while the key is held down.
#define CMD_AND_LAYER(num)                  LAYER_AND_MOD_ON_HOLD(num, CMD)

/// Activate both Meta and a layer while the key is held down.
#define META_AND_LAYER(num)                 LAYER_AND_MOD_ON_HOLD(num, META)

/// Activate both AltGr and a layer while the key is held down.
#define ALTGR_AND_LAYER(num)                LAYER_AND_MOD_ON_HOLD(num, RIGHT_ALT)

/// Activate both Right Shift and a layer while the key is held down.
#define RIGHT_SHIFT_AND_LAYER(num)          LAYER_AND_MOD_ON_HOLD(num, RIGHT_SHIFT)

/// Activate both Right Ctrl and a layer while the key is held down.
#define RIGHT_CTRL_AND_LAYER(num)           LAYER_AND_MOD_ON_HOLD(num, RIGHT_CTRL)

/// Activate both Right Cmd and a layer while the key is held down.
#define RIGHT_CMD_AND_LAYER(num)            LAYER_AND_MOD_ON_HOLD(num, RIGHT_CMD)

/// Activate both Ctrl + Shift and a layer while the key is held down.
#define CTRL_SHIFT_AND_LAYER(num)           LAYER_AND_MOD_ON_HOLD(num, CTRL_SHIFT)

/// Activate both Shift + Alt and a layer while the key is held down.
#define SHIFT_ALT_AND_LAYER(num)            LAYER_AND_MOD_ON_HOLD(num, SHIFT_ALT)

/// Activate both Right Shift + AltGr and a layer while the key is held down.
#define SHIFT_ALTGR_AND_LAYER(num)          LAYER_AND_MOD_ON_HOLD(num, SHIFT_ALTGR)

/// Activate both Cmd + Shift and a layer while the key is held down.
#define CMD_SHIFT_AND_LAYER(num)            LAYER_AND_MOD_ON_HOLD(num, CMD_SHIFT)

/// Activate both Ctrl + Alt and a layer while the key is held down.
#define CTRL_ALT_AND_LAYER(num)             LAYER_AND_MOD_ON_HOLD(num, CTRL_ALT)

/// Activate both Cmd + Alt and a layer while the key is held down.
#define CMD_ALT_AND_LAYER(num)              LAYER_AND_MOD_ON_HOLD(num, CMD_ALT)

/// Activate both Ctrl + Alt + Shift and a layer while the key is held down.
#define CTRL_ALT_SHIFT_AND_LAYER(num)       LAYER_AND_MOD_ON_HOLD(num, CTRL_ALT_SHIFT)

/// Activate both Cmd + Alt + Shift and a layer while the key is held down.
#define CMD_ALT_SHIFT_AND_LAYER(num)        LAYER_AND_MOD_ON_HOLD(num, CMD_ALT_SHIFT)

/// Activate both Ctrl + Alt + Cmd and a layer while the key is held down.
#define CTRL_ALT_CMD_AND_LAYER(num)         LAYER_AND_MOD_ON_HOLD(num, CTRL_ALT_CMD)

/// Activate both Ctrl + Alt + Cmd + Shift and a layer while the key is held down.
#define CTRL_ALT_CMD_SHIFT_AND_LAYER(num)   LAYER_AND_MOD_ON_HOLD(num, CTRL_ALT_CMD_SHIFT)

/// Activate both Ctrl + Alt + Cmd + Shift and a layer while the key is held down.
#define HYPER_AND_LAYER(num)                CTRL_ALT_CMD_SHIFT_AND_LAYER(num)

/// Activate both Ctrl + Alt + Shift and a layer while the key is held down.
#define MEH_AND_LAYER(num)                  CTRL_ALT_SHIFT_AND_LAYER(num)

/// Enable layer on key release (note: there should be a corresponding
/// `LAYER_DISABLE` somewhere or this will be permanent).
#define LAYER_ENABLE(num)           LAYER_COMMAND(ENABLE, ON_RELEASE, (num))

/// Disable layer on key release (note: there should be a corresponding
/// `LAYER_ENABLE` somewhere or this will be permanent).
#define LAYER_DISABLE(num)          LAYER_COMMAND(DISABLE, ON_RELEASE, (num))

/// Set the base layer to `num`. This means that the layer will be considered
/// enabled regardless of other layer states, and that no lower layer will be
/// considered. If the base layer has any undefined keys (`NONE`), the default
/// action (rather than that of a lower layer) will be used. The new base layer
/// should have a key to restore the base layer or this will be permanent.
/// Layer 1 is the base layer by default.
#define LAYER_SET_BASE(num)         LAYER_COMMAND(SET_BASE, ON_RELEASE, (num))

/// Set the base layer while the key is held down.
#define LAYER_SET_BASE_HOLD(num)    LAYER_COMMAND(SET_BASE, ON_HOLD, (num))

/// Set the base layer while the key is held down, and leave enabled if no
/// other key is pressed before releasing this key.
#define LAYER_SET_BASE_STICKY(num)  LAYER_COMMAND(SET_BASE, ON_HOLD_KEEP_IF_NO_KEYPRESS, (num))

/// Disable all other layers except this one on key release.
/// Note that the base layer will still be considered active.
#define LAYER_ONLY(num)             LAYER_COMMAND(SET_MASK, ON_RELEASE, (num))

/// Disable all other layers except this one while the key is held down.
/// Note that the base layer will still be considered active.
#define LAYER_ONLY_HOLD(num)        LAYER_COMMAND(SET_MASK, ON_HOLD, (num))

/// Disable all other layers except this one while the key is held down,
/// and don't undo on release if no other key has been pressed held.
/// Note that the base layer will still be considered active.
#define LAYER_ONLY_STICKY(num)      LAYER_COMMAND(SET_MASK, ON_HOLD_KEEP_IF_NO_KEYPRESS, (num))

/// Toggle layer if the key is released and no other key is pressed while it
/// was down, otherwise act as Shift.
#define SHIFT_OR_LAYER(num)         LAYER_TOGGLE_OR_MOD(num, SHIFT)

/// Toggle layer if the key is released and no other key is pressed while it
/// was down, otherwise act as Ctrl.
#define CTRL_OR_LAYER(num)          LAYER_TOGGLE_OR_MOD(num, CTRL)

/// Toggle layer if the key is released and no other key is pressed while it
/// was down, otherwise act as Alt.
#define ALT_OR_LAYER(num)           LAYER_TOGGLE_OR_MOD(num, ALT)

/// Toggle layer if the key is released and no other key is pressed while it
/// was down, otherwise act as Win.
#define WIN_OR_LAYER(num)           LAYER_TOGGLE_OR_MOD(num, WIN)

/// Toggle layer if the key is released and no other key is pressed while it
/// was down, otherwise act as Cmd.
#define CMD_OR_LAYER(num)           LAYER_TOGGLE_OR_MOD(num, CMD)

/// Toggle layer if the key is released and no other key is pressed while it
/// was down, otherwise act as Meta.
#define META_OR_LAYER(num)          LAYER_TOGGLE_OR_MOD(num, META)

/// Toggle layer if the key is released and no other key is pressed while it
/// was down, otherwise act as AltGr.
#define ALTGR_OR_LAYER(num)         (LAYER_TOGGLE_OR_MOD(num, ALTGR))

/// Toggle layer if the key is released and no other key is pressed while it
/// was down, otherwise act as Right Shift.
#define RIGHT_SHIFT_OR_LAYER(num)   LAYER_TOGGLE_OR_MOD(num, RIGHT_SHIFT)

/// Toggle layer if the key is released and no other key is pressed while it
/// was down, otherwise act as Right Ctrl.
#define RIGHT_CTRL_OR_LAYER(num)    LAYER_TOGGLE_OR_MOD(num, RIGHT_CTRL)

/// Toggle layer if the key is released and no other key is pressed while it
/// was down, otherwise act as Right Cmd.
#define RIGHT_CMD_OR_LAYER(num)            LAYER_TOGGLE_OR_MOD(num, RIGHT_CMD)

/// Toggle layer if the key is released and no other key is pressed while it
/// was down, otherwise act as Ctrl + Shift.
#define CTRL_SHIFT_OR_LAYER(num)           LAYER_TOGGLE_OR_MOD(num, CTRL_SHIFT)

/// Toggle layer if the key is released and no other key is pressed while it
/// was down, otherwise act as Shift + Alt.
#define SHIFT_ALT_OR_LAYER(num)            LAYER_TOGGLE_OR_MOD(num, SHIFT_ALT)

/// Toggle layer if the key is released and no other key is pressed while it
/// was down, otherwise act as Right Shift + AltGr.
#define SHIFT_ALTGR_OR_LAYER(num)          LAYER_TOGGLE_OR_MOD(num, SHIFT_ALTGR)

/// Toggle layer if the key is released and no other key is pressed while it
/// was down, otherwise act as Cmd + Shift.
#define CMD_SHIFT_OR_LAYER(num)            LAYER_TOGGLE_OR_MOD(num, CMD_SHIFT)

/// Toggle layer if the key is released and no other key is pressed while it
/// was down, otherwise act as Ctrl + Alt.
#define CTRL_ALT_OR_LAYER(num)             LAYER_TOGGLE_OR_MOD(num, CTRL_ALT)

/// Toggle layer if the key is released and no other key is pressed while it
/// was down, otherwise act as Cmd + Alt.
#define CMD_ALT_OR_LAYER(num)              LAYER_TOGGLE_OR_MOD(num, CMD_ALT)

/// Toggle layer if the key is released and no other key is pressed while it
/// was down, otherwise act as Ctrl + Alt + Shift.
#define CTRL_ALT_SHIFT_OR_LAYER(num)       LAYER_TOGGLE_OR_MOD(num, CTRL_ALT_SHIFT)

/// Toggle layer if the key is released and no other key is pressed while it
/// was down, otherwise act as Cmd + Alt + Shift.
#define CMD_ALT_SHIFT_OR_LAYER(num)        LAYER_TOGGLE_OR_MOD(num, CMD_ALT_SHIFT)

/// Toggle layer if the key is released and no other key is pressed while it
/// was down, otherwise act as Ctrl + Alt + Cmd.
#define CTRL_ALT_CMD_OR_LAYER(num)         LAYER_TOGGLE_OR_MOD(num, CTRL_ALT_CMD)

/// Toggle layer if the key is released and no other key is pressed while it
/// was down, otherwise act as Ctrl + Alt + Cmd + Shift.
#define CTRL_ALT_CMD_SHIFT_OR_LAYER(num)   LAYER_TOGGLE_OR_MOD(num, CTRL_ALT_CMD_SHIFT)

/// Toggle layer if the key is released and no other key is pressed while it
/// was down, otherwise act as Ctrl + Alt + Cmd + Shift.
#define HYPER_OR_LAYER(num)                CTRL_ALT_CMD_SHIFT_OR_LAYER(num)

/// Toggle layer if the key is released and no other key is pressed while it
/// was down, otherwise act as Ctrl + Alt + Shift.
#define MEH_OR_LAYER(num)                  CTRL_ALT_SHIFT_OR_LAYER(num)

/// Toggle layer while the key is held down. If no other key is pressed while
/// the key is held down, send a plain key press on release. ("Plain" here
/// means the key cannot have modifiers, e.g., no `SHIFT(key)`, there are not
/// enough bits in the keycode to store this information.)
#define LAYER_OR_PLAIN_KEY(layer, key)  (PLAIN_KEY_OF(key) | COMMAND(CMD_LAYER_OR_KEY) | ((layer) << 8))

/// Reset the keyboard, release all keys, clear all state to defaults.
#define KEY_EXT_RESET_KEYBOARD      EXTENDED(RESET_KEYBOARD)

/// Enter the bootloader mode for firmware update.
#define KEY_EXT_ENTER_BOOTLOADER    EXTENDED(ENTER_BOOTLOADER)

/// Reset layers to their default state. Note that there are corner cases
/// where this causes an unexpected state if you hold down a layer toggle
/// at the same time (in which case the toggle will first be reset by this
/// key, and then toggle back on release).
#define KEY_EXT_RESET_LAYERS        EXTENDED(RESET_LAYERS)

/// The Apple Fn key. To enable this you need to use Apple's USB vendor id
/// and define `ENABLE_APPLE_FN_KEY`.
#define KEY_APPLE_FN                USB_KEY_VIRTUAL_APPLE_FN

// MARK: - Advanced Keycodes

#define LAYER_TOGGLE_OR_MOD(layer, mod)     (LAYER_COMMAND(TOGGLE, IF_NO_KEYPRESS, layer) | PASTE(MODS_, mod))

#define LAYER_AND_MOD_ON_HOLD(layer, mod)   (LAYER_COMMAND(TOGGLE, ON_HOLD, layer) | PASTE(MODS_, mod))

/// Layer command, such as `LAYER_COMMAND(TOGGLE, ON_HOLD, 2)` toggles layer 2
/// while the key is held down.
#define LAYER_COMMAND(cmd, act, num)        (COMMAND(PASTE(CMD_LAYER_, cmd)) | ((PASTE(ACT_, act)) << 5) | ((num) & LAYER_NUMBER_MASK))

/// Extended keycode, e.g., `EXTENDED(ENTER_BOOTLOADER)`.
#define EXTENDED(key)               (EXTENDED_KEY_BIT | PASTE(EXT_, key))

/// Extended keycode, e.g., `EXT(ENTER_BOOTLOADER)`.
#define EXT(key)                    EXTENDED(key)

/// Macro. The corresponding macro must be defined in `macros.c`.
#define MACRO(macro)                (EXTENDED_KEY_BIT | MACRO_BIT | (macro))

// The following are modifier keys that set exactly a given set of modifiers,
// cancelling any other modifiers that were held down _before_ these. This
// can be useful when done in a layer activated by holding down a modifier,
// which you want to cancel if another modifier is pressed from the layer.

/// Shift, cancel all other modifiers held before pressing.
#define EXACTLY_SHIFT               EXACT_MODS(SHIFT_BIT)

/// Ctrl, cancel all other modifiers held before pressing.
#define EXACTLY_CTRL                EXACT_MODS(CTRL_BIT)

/// Alt, cancel all other modifiers held before pressing.
#define EXACTLY_ALT                 EXACT_MODS(ALT_BIT)

/// Alt Gr, cancel all other modifiers held before pressing.
#define EXACTLY_ALTGR               EXACT_MODS(ALTGR_BIT)

/// Cmd, cancel all other modifiers held before pressing.
#define EXACTLY_CMD                 EXACT_MODS(CMD_BIT)

/// Right Shift, cancel all other modifiers held before pressing.
#define EXACTLY_RIGHT_SHIFT         EXACT_MODS(RIGHT_SHIFT_BIT)

/// Right Ctrl, cancel all other modifiers held before pressing.
#define EXACTLY_RIGHT_CTRL          EXACT_MODS(RIGHT_CTRL_BIT)

/// Right Cmd, cancel all other modifiers held before pressing.
#define EXACTLY_RIGHT_CMD           EXACT_MODS(RIGHT_CMD_BIT)

/// Ctrl + Shift, cancel all other modifiers held before pressing.
#define EXACTLY_CTRL_SHIFT          (EXACTLY_CTRL | SHIFT_BIT)

/// Shift + Alt, cancel all other modifiers held before pressing.
#define EXACTLY_SHIFT_ALT           (EXACTLY_ALT | SHIFT_BIT)

/// Right Shift + Alt Gr, cancel all other modifiers held before pressing.
#define EXACTLY_SHIFT_ALTGR         (EXACTLY_ALTGR | SHIFT_BIT)

/// Cmd + Shift, cancel all other modifiers held before pressing.
#define EXACTLY_CMD_SHIFT           (EXACTLY_CMD | SHIFT_BIT)

/// Ctrl + Alt, cancel all other modifiers held before pressing.
#define EXACTLY_CTRL_ALT            (EXACTLY_CTRL | ALT_BIT)

/// Cmd + Alt, cancel all other modifiers held before pressing.
#define EXACTLY_CMD_ALT             (EXACTLY_CMD | ALT_BIT)

/// Ctrl + Alt + Shift, cancel all other modifiers held before pressing.
#define EXACTLY_CTRL_ALT_SHIFT      (EXACTLY_CTRL | ALT_BIT | SHIFT_BIT)

/// Cmd + Alt + Shift, cancel all other modifiers held before pressing.
#define EXACTLY_CMD_ALT_SHIFT       (EXACTLY_CMD | ALT_BIT | SHIFT_BIT)

/// Ctrl + Alt + Cmd, cancel all other modifiers held before pressing.
#define EXACTLY_CTRL_ALT_CMD        (EXACTLY_CTRL | ALT_BIT | CMD_BIT)

/// Ctrl + Alt + Shift, cancel all other modifiers held before pressing.
#define EXACTLY_MEH                 (EXACTLY_CTRL_ALT_SHIFT)

/// An exact modifiers key, that sets exactly the modifiers given. Note that
/// the modifiers are either all on the left or all on the right, due to
/// limited number of bits available.
#define EXACT_MODS(mods)            ((((mods) & 0xF0U) ? (0x10U | ((mods) | ((mods) >> 4))) : (mods)) | EXTENDED_KEY_BIT | EXACT_MODS_BIT)

/// Act as a modifier while held down, or send a key on release if no other
/// keys were pressed while holding, e.g., `MOD_OR_KEY(CTRL(ESC))`.
#define MOD_OR_KEY(key)             (key | (COMMAND(CMD_MODIFIER_OR_KEY)))

/// Construct the modifiers mask for adding to a regular key. Note that the
/// modifiers are either all on the left or all on the right, due to
/// limited number of bits available.
#define MODS_FOR_KEY(mods)          ((((mods) & 0xF0U) ? (0x10U | ((mods) | ((mods) >> 4))) : (mods)) << 8)

// The extended keyboard commands. These occupy an entirely separate 8-bit
// namespace from the normal keys. To use these keys as mapping targets, use
// the `EXTENDED(RESET_KEYBOARD)` style macro instead.
enum extended_keycode {
    /// Reset the keyboard and release all keys.
    EXT_RESET_KEYBOARD = 1,

    /// Enter the bootloader for firmware update. Disconnects the keyboard.
    EXT_ENTER_BOOTLOADER,

    /// Reset layers to default state.
    EXT_RESET_LAYERS,

    /// Send all modifiers (Shift, Ctrl, Alt, Cmd).
    EXT_HYPER_MODIFIERS,

    /// Send almost all modifiers (Shift, Ctrl, Alt).
    EXT_MEH_MODIFIERS,

    /// Toggles whether boot or report protocol is used. This can be used to
    /// work around a BIOS that doesn't request the boot protocol but needs
    /// it. Note, however, that even the report protocol is usually compatible
    /// with boot protocol as long as you don't press more than 6 non-modifier
    /// keys at once, so there might not be need for this. (Using this the
    /// other way around cannot force an OS or BIOS to accept the report
    /// protocol, and things may break if that is attempted.)
    EXT_TOGGLE_BOOT_PROTOCOL,

#if ENABLE_KEYLOCK
    /// Locks the next key down until it, or the keylock key, is pressed again.
    EXT_KEYLOCK,
#endif

#if ENABLE_SIMULATED_TYPING
    EXT_PRINT_DEBUG_INFO,

/// "Print" (i.e., type) debug info.
#define KEY_EXT_PRINT_DEBUG_INFO    EXTENDED(PRINT_DEBUG_INFO)
#endif

    // Aliases:

    EXT_HYPER = EXT_HYPER_MODIFIERS,
    EXT_MEH = EXT_MEH_MODIFIERS,
};

// Layer commands. Use these through the `LAYER_COMMAND` macro, or other
// macros that use it. These do _not_ work as keycodes on their own.

/// Toggle the layer state
#define CMD_LAYER_TOGGLE            1

/// Disable the layer
#define CMD_LAYER_DISABLE           2

/// Enable the layer
#define CMD_LAYER_ENABLE            3

/// Set the enabled layers mask
#define CMD_LAYER_SET_MASK          4

/// Set the base layer
#define CMD_LAYER_SET_BASE          5

/// Makes the key function as a modifier when held down, or as a key if clicked
/// without pressing any other keys while it was held.
#define CMD_MODIFIER_OR_KEY         6

/// Makes the key function as an ON_HOLD LAYER_TOGGLE when held down, or as a
/// a key if clicked without pressing any other keys while it was held. The
/// key cannot have modifiers (since the layer number is in the modifier bits
/// for this command).
#define CMD_LAYER_OR_KEY            7

// Action modifier for a layer command. Use these through the `LAYER_COMMAND`
// macro, e.g, `LAYER_COMMAND(TOGGLE, ON_HOLD, 2)`.

/// The command activates on key press and deactivates on release.
#define ACT_ON_HOLD                     0

/// The command activates on key release.
#define ACT_ON_RELEASE                  1

/// The command activates on key press.
#define ACT_ON_PRESS                    2

/// The command activates on release if no other key was pressed.
#define ACT_IF_NO_KEYPRESS              3

/// The command activates on key press and deactivates on release if no
/// other key was pressed.
#define ACT_ON_HOLD_KEEP_IF_NO_KEYPRESS 4

#define COMMAND(cmd)        ((cmd) << (8+5))

// MARK: - Internals

#define MODS_SHIFT                      MODS_FOR_KEY(SHIFT_BIT)
#define MODS_CTRL                       MODS_FOR_KEY(CTRL_BIT)
#define MODS_ALT                        MODS_FOR_KEY(ALT_BIT)
#define MODS_CMD                        MODS_FOR_KEY(CMD_BIT)
#define MODS_WIN                        MODS_FOR_KEY(WIN_BIT)
#define MODS_META                       MODS_FOR_KEY(META_BIT)
#define MODS_ALTGR                      MODS_FOR_KEY(ALTGR_BIT)
#define MODS_RIGHT_SHIFT                MODS_FOR_KEY(RIGHT_SHIFT_BIT)
#define MODS_RIGHT_CTRL                 MODS_FOR_KEY(RIGHT_CTRL_BIT)
#define MODS_RIGHT_CMD                  MODS_FOR_KEY(RIGHT_CMD_BIT)
#define MODS_CTRL_SHIFT                 (MODS_CTRL | MODS_SHIFT)
#define MODS_SHIFT_ALT                  (MODS_SHIFT | MODS_ALT)
#define MODS_SHIFT_ALTGR                (MODS_RIGHT_SHIFT | MODS_ALTGR)
#define MODS_CMD_SHIFT                  (MODS_CMD | MODS_SHIFT)
#define MODS_CTRL_ALT                   (MODS_CTRL | MODS_ALT)
#define MODS_CMD_ALT                    (MODS_CMD | MODS_ALT)
#define MODS_CTRL_ALT_SHIFT             (MODS_CTRL | MODS_ALT | MODS_SHIFT)
#define MODS_CMD_ALT_SHIFT              (MODS_CMD | MODS_ALT | MODS_SHIFT)
#define MODS_CTRL_ALT_CMD               (MODS_CTRL | MODS_ALT | MODS_CMD)
#define MODS_CTRL_ALT_CMD_SHIFT         (MODS_CTRL | MODS_ALT | MODS_CMD | MODS_SHIFT)
#define MODS_HYPER                      MODS_CTRL_ALT_CMD_SHIFT
#define MODS_MEH                        MODS_CTRL_ALT_SHIFT

#define PASTE_(a, b)        a##b
#define PASTE(a, b)         PASTE_(a, b)

// Keycode bit patterns:
// 0000 0000 0000 0000 - pass through (`PASS`)
// 0000 0000 kkkk kkkk - plain key k (1-254) (`KEY(...)`)
// 0000 0000 1111 1111 - no action (`NONE`)
// 0001 0000 00ee eeee - extended key e (0-63) (`EXTENDED(...)`)
// 0001 0000 010r mmmm - set exact modifiers (bitmask m, r = left/right 0/1)
// 0001 0000 1nnn nnnn - macro number n (0-127) (`MACRO(...)`)
// 0000 mmmm kkkk kkkk - modifiers (bitmask m, left side) and plain key k
// 0001 mmmm kkkk kkkk - modifiers (bitmask m, right side) and plain key k
// cccr mmmm aaan nnnn - command c (1-6), activate on a, layer number n
// 111n nnnn kkkk kkkk - layer n when held, key k on click

#define is_extended_keycode(code)           ((code) >> 8)
#define is_command_keycode(code)            (COMMAND_OF(code))
#define extended_keycode_is_macro(extcode)  ((extcode) & MACRO_BIT)
#define extended_keycode_is_exact_modifiers(extcode) (((extcode) & EXACT_MODS_BIT))

#define MODIFIERS_MASK              (0x0F00U)
#define COMMAND_KEYCODE_MASK        (0xE000U)
#define RIGHT_MOD_BIT               (0x1000U)
#define EXTENDED_KEY_BIT            RIGHT_MOD_BIT
#define LAYER_NUMBER_MASK           (0x001FU)
#define LAYER_CMD_MODIFIER_MASK     (0x00E0U)
#define LAYER_IN_LAYER_OR_KEY_MASK  (MODIFIERS_MASK | EXTENDED_KEY_BIT)
#define MACRO_BIT                   (0x0080U)
#define EXACT_MODS_BIT              (0x0040U)

#define LAYER_OF_COMMAND(code)      ((code) & LAYER_NUMBER_MASK)
#define LAYER_CMD_MODIFIER_OF(code) (((code) & LAYER_CMD_MODIFIER_MASK) >> 5)
#define LAYER_OF_LAYER_OR_KEY(code) (((code) & LAYER_IN_LAYER_OR_KEY_MASK) >> 8)
#define MODIFIERS_OF_EXTENDED(code) (((code) & MODIFIERS_MASK) >> (((code) & RIGHT_MOD_BIT) ? 4 : 8))
#define COMMAND_OF(code)            ((code) >> (8+5))
#define PLAIN_KEY_OF(code)          ((code) & 0xFFU)
#define MACRO_OF_EXTENDED(extcode)  ((extcode) & ~(MACRO_BIT))
#define EXACT_MODS_OF_EXTENDED(code) (((code) & 0x0FU) << (((code) & 0x10U) ? 4 : 0))

#endif // KK_KEYCODES_H
