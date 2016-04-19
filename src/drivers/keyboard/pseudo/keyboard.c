/*
 * Copyright 2014 Google Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#include <assert.h>
#include <ctype.h>
#include <libpayload.h>
#include <stdint.h>

#include "base/die.h"
#include "base/init_funcs.h"
#include "base/keycodes.h"
#include "base/state_machine.h"
#include "base/time.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/keyboard/pseudo/keyboard.h"

static void pk_state_machine_setup(PseudoKeyboard *keyboard)
{
	int i;

	// Mainboard needs to define keyboard_init function and return back
	// filled state machine desc.
	mainboard_keyboard_init(&keyboard->desc);

	if (keyboard->desc.total_states_count == 0)
		return;

	// Initialize state machine for pseudo keyboard.
	keyboard->pk_sm = sm_init(keyboard->desc.total_states_count);

	// Add start state.
	sm_add_start_state(keyboard->pk_sm, keyboard->desc.start_state);

	// Add intermediate states.
	for (i = 0; i < keyboard->desc.int_states_count; i++)
		sm_add_nonfinal_state(keyboard->pk_sm,
				      keyboard->desc.int_states_arr[i]);

	// Add final states.
	for (i = 0; i < keyboard->desc.final_states_count; i++)
		sm_add_final_state(keyboard->pk_sm,
				   keyboard->desc.final_states_arr[i].state_id);

	// Add valid transitions.
	for (i = 0; i < keyboard->desc.trans_count; i++)
		sm_add_transition(keyboard->pk_sm,
				  keyboard->desc.trans_arr[i].src,
				  keyboard->desc.trans_arr[i].inp,
				  keyboard->desc.trans_arr[i].dst);
}

static const struct pk_final_state *pk_find_final_state(
	PseudoKeyboard *keyboard, int id)
{
	for (int i = 0; i < keyboard->desc.final_states_count; i++)
		if (keyboard->desc.final_states_arr[i].state_id == id)
			return &keyboard->desc.final_states_arr[i];

	die("Error in final state logic\n");
}

/*
 * Key codes are expected as follows:
 * ASCII characters : ASCII values (0 - 127)
 * Key UP          : 128
 * Key DOWN        : 129
 * Key Right       : 130
 * Key Left        : 131
 *
 * Modifiers is basically a bitmask to indicate what modifiers are set.
 *
 * This function returns the number of codes read into codes array.
 */
static size_t read_key_codes(PseudoKeyboard *keyboard, Modifier *modifiers,
			     uint16_t *codes, size_t max_codes)
{
	assert(modifiers && codes && max_codes);

	int i = 0;

	sm_reset_state(keyboard->pk_sm);

	// We have only max_codes space in codes to fill up key codes.
	while (i < max_codes) {

		int input, ret, output;
		uint64_t start = time_us(0);
		// If no input is received for 500 msec, return.
		uint64_t timeout_us = 500 * 1000;

		do {
			uint64_t button_press = time_us(0);
			uint64_t button_timeout = 100 * 1000;

			// Mainboard needs to define function to read input.
			input = mainboard_read_input();

			if (input == PseudoKb_NoInput)
				continue;

			// Input should be seen for at least 100 ms/
			do {
				if (mainboard_read_input() != input) {
					input = PseudoKb_NoInput;
					break;
				}
			} while (time_us(button_press) < button_timeout);

			/*
			 * If input is received, wait until input changes to
			 * avoid duplicate entries for same input.
			 */
			if (input != PseudoKb_NoInput) {
				while (mainboard_read_input() == input)
				{;}
				break;
			}
		} while (time_us(start) < timeout_us);

		// If timeout without input, return.
		if (input == PseudoKb_NoInput)
			break;

		// Run state machine to move to next state.
		ret = sm_run(keyboard->pk_sm, input, &output);

		if (ret == STATE_NOT_FINAL)
			continue;

		if (ret == STATE_NO_TRANSITION) {
			sm_reset_state(keyboard->pk_sm);
			continue;
		}

		assert(output < keyboard->desc.total_states_count);
		const struct pk_final_state *ptr;
		ptr = pk_find_final_state(keyboard, output);
		*modifiers |= ptr->mod;
		codes[i++] = ptr->keycode;
	}

	return i;
}

// Gives # of unused slots in fifo to put elements.
static inline size_t key_fifo_size(PseudoKeyboard *keyboard)
{
	return ARRAY_SIZE(keyboard->key_fifo) - keyboard->fifo_head;
}

// Gives # of used unread slots in fifo.
static inline size_t key_fifo_occupied(PseudoKeyboard *keyboard)
{
	return keyboard->fifo_head - keyboard->fifo_tail;
}

// Tells if all slots in fifo are used.
static inline int key_fifo_full(PseudoKeyboard *keyboard)
{
	return !key_fifo_size(keyboard);
}

static void key_fifo_put(PseudoKeyboard *keyboard, uint16_t key)
{
	if (key_fifo_full(keyboard)) {
		printf("%s: dropped a character\n",__func__);
		return;
	}

	keyboard->key_fifo[keyboard->fifo_head++] = key;
}

static uint16_t key_fifo_get(PseudoKeyboard *keyboard)
{
	assert(key_fifo_occupied(keyboard));

	uint16_t key = keyboard->key_fifo[keyboard->fifo_tail++];
	return key;
}

static void key_fifo_clear(PseudoKeyboard *keyboard)
{
	keyboard->fifo_tail = keyboard->fifo_head = 0;
}

static void pk_more_keys(PseudoKeyboard *keyboard)
{
	// No more keys until you finish the ones you've got.
	if (key_fifo_occupied(keyboard))
		return;

	key_fifo_clear(keyboard);

	// Get ascii codes.
	uint16_t key_codes[PseudoKb_FifoSize];
	Modifier modifiers = PseudoKb_Modifier_None;
	/*
	 * Every board that uses pseudo keyboard is expected to implement its
	 * own read_key_codes since input methods and input pins can vary.
	 */
	size_t count = read_key_codes(keyboard, &modifiers, key_codes,
				      PseudoKb_FifoSize);

	assert(count <= PseudoKb_FifoSize);

	// Look at all the keys and fill the FIFO.
	for (size_t pos = 0; pos < count; pos++) {
		uint16_t code = key_codes[pos];

		// Check for valid keycode.
		if ((code < PseudoKb_KeyCodeStart) ||
		    (code > PseudoKb_KeyCodeEnd))
			continue;

		// Check for ascii values of alphabets.
		if (isalpha(code)) {
			// Convert alpha characters into control characters.
			if (modifiers & PseudoKb_Modifier_Ctrl)
				code &= 0x1f;
			key_fifo_put(keyboard, code);
			continue;
		}

		// Handle special keys.
		switch (code) {
		case PseudoKb_KeyCodeUp:
			key_fifo_put(keyboard, KEY_UP);
			break;
		case PseudoKb_KeyCodeDown:
			key_fifo_put(keyboard, KEY_DOWN);
			break;
		case PseudoKb_KeyCodeRight:
			key_fifo_put(keyboard, KEY_RIGHT);
			break;
		case PseudoKb_KeyCodeLeft:
			key_fifo_put(keyboard, KEY_LEFT);
			break;
		default:
			key_fifo_put(keyboard, code);
		}
	}
}

static int pk_have_char(KeyboardOps *me)
{
	PseudoKeyboard *keyboard = container_of(me, PseudoKeyboard, ops);

	if (!keyboard->initialized)
		pk_state_machine_setup(keyboard);

	// Get more keys if we need them.
	pk_more_keys(keyboard);

	return key_fifo_occupied(keyboard);
}

static int pk_get_char(KeyboardOps *me)
{
	PseudoKeyboard *keyboard = container_of(me, PseudoKeyboard, ops);

	if (!keyboard->initialized)
		pk_state_machine_setup(keyboard);

	while (!pk_have_char(me))
	{;}

	return key_fifo_get(keyboard);
}

PseudoKeyboard pseudo_keyboard = {
	.ops = {
		.get_char = &pk_get_char,
		.have_char = &pk_have_char
	}
};
