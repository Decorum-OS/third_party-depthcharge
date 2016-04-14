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

#ifndef __DRIVERS_KEYBOARD_PSEUDO_KEYBOARD_H__
#define __DRIVERS_KEYBOARD_PSEUDO_KEYBOARD_H__

#include "base/state_machine.h"
#include "drivers/keyboard/keyboard.h"

typedef enum Modifier {
	PseudoKb_Modifier_None = 0x0,
	PseudoKb_Modifier_Ctrl = 0x1,
	PseudoKb_Modifier_Alt = 0x2,
	PseudoKb_Modifier_Shift = 0x4
} Modifier;

enum {
	PseudoKb_KeyCodeStart = 0,
	PseudoKb_KeyCodeUp = 128,
	PseudoKb_KeyCodeDown = 129,
	PseudoKb_KeyCodeRight = 130,
	PseudoKb_KeyCodeLeft = 131,
	PseudoKb_KeyCodeEnd = 131,
};

// Structure defining final states of pseudo keyboard state machine.
struct pk_final_state {
	int state_id;
	Modifier mod;
	uint16_t keycode;
};

// Structure defining transition for pseudo keyboard states.
struct pk_trans {
	int src;
	int inp;
	int dst;
};

// Structure for state machine descriptor filled in by mainboard.
struct pk_sm_desc {
	size_t total_states_count;
	int start_state;
	size_t int_states_count;
	const int *int_states_arr;
	size_t final_states_count;
	const struct pk_final_state *final_states_arr;
	size_t trans_count;
	const struct pk_trans *trans_arr;
};

// Special input value defining no input present.
enum {
	PseudoKb_NoInput = -1
};

// Mainboard defined functions.
void mainboard_keyboard_init(struct pk_sm_desc *desc);
int mainboard_read_input(void);

enum {
	PseudoKb_FifoSize = 16
};

typedef struct {
	KeyboardOps ops;

	int initialized;

	// State machine data for pseudo keyboard.
	struct sm_data *pk_sm;
	struct pk_sm_desc desc;

	uint16_t key_fifo[PseudoKb_FifoSize];
	// Elements are added at the head and removed from the tail.
	size_t fifo_tail;
	size_t fifo_head;
} PseudoKeyboard;

extern PseudoKeyboard pseudo_keyboard;

#endif /* __DRIVERS_KEYBOARD_PSEUDO_KEYBOARD_H__ */
