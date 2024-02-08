/* Copyright 2020 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
 * Copyright 2019 Sunjun Kim
 * Copyright 2020 Ploopy Corporation
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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
#include QMK_KEYBOARD_H


// Tap Dance keycodes
enum td_keycodes {
    MSE_BTN5_LAYR_1,  // Our example key: Forward mouse button when held switch to layer 1,  forward mouse button when tapped.
    MSE_BTN4_DRAG     // Hold mse btn4 toggles dragscroll
};

// Define a state for when we're holding down button 4
// this enters precison mode but also allows us to switch to another layer
bool btn4_held = false;
bool precision_mode = false;

// Define a type containing as many tapdance states as you need
// Define a type containing as many tapdance states as you need
typedef enum {
    TD_NONE,
    TD_UNKNOWN,
    TD_SINGLE_TAP,
    TD_SINGLE_HOLD,
    TD_DOUBLE_SINGLE_TAP,
    TD_DOUBLE_HOLD,
    TD_DOUBLE_TAP
} td_state_t;

// Create a global instance of the tapdance state type
static td_state_t td_state; 

// Declare your tapdance functions:

// Function to determine the current tapdance state
td_state_t cur_dance(tap_dance_state_t *state);

// `finished` and `reset` functions for each tapdance keycode
void mseBtn4_finished(tap_dance_state_t *state, void *user_data);
void mseBtn4_reset(tap_dance_state_t *state, void *user_data);
void mseBtn5_finished(tap_dance_state_t *state, void *user_data);
void mseBtn5_reset(tap_dance_state_t *state, void *user_data);

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT( /* Base */
        KC_BTN1, KC_BTN3, KC_BTN2,
          TD(MSE_BTN4_DRAG), TD(MSE_BTN5_LAYR_1)
    ),
    [1] = LAYOUT(
        DRAG_SCROLL, _______, DPI_CONFIG,
          _______, _______
    )
};

// Determine the tapdance state to return
td_state_t cur_dance(tap_dance_state_t *state) {
    if (state->count == 1) {
        // Interrupted means some other button was pressed in the tapping term
        if (state->interrupted || !state->pressed) {
            xprintf("TD_SINGLE_TAP\n");
            return TD_SINGLE_TAP;
        } else {
            xprintf("TD_SINGLE_HOLD\n");
            return TD_SINGLE_HOLD;
        }
    }

    if (state->count == 2) {
        if (state->interrupted) {
            xprintf("TD_DOUBLE_SINGLE_TAP\n");
            return TD_DOUBLE_SINGLE_TAP;
        } else if (state->pressed) {
            xprintf("TD_DOUBLE_HOLD\n");
            return TD_DOUBLE_HOLD;
        } else {
            xprintf("TD_DOUBLE_TAP\n");
            return TD_DOUBLE_TAP;
        }

    } else {
        xprintf("TD_UNKNOWN\n");
        return TD_UNKNOWN; // Any number higher than the maximum state value you return above
    }
}

void mseBtn4_finished(tap_dance_state_t *state, void *user_data) {
    td_state = cur_dance(state);
    switch (td_state) {
        case TD_SINGLE_HOLD:
            xprintf("Hold for button 4 finished\n");
            is_drag_scroll = true;
            btn4_held = true;
        break;
        case TD_DOUBLE_TAP:
            if (!precision_mode) {
                pmw33xx_set_cpi(0, (dpi_array[keyboard_config.dpi_config] / 2) );
                precision_mode = true;
            } else {
                pmw33xx_set_cpi(0, dpi_array[keyboard_config.dpi_config] );
                precision_mode = false;
            }
        break;
        default:
        break;
    }
}

void mseBtn4_reset(tap_dance_state_t *state, void *user_data) {
    switch (td_state) {
        case TD_SINGLE_TAP:
            xprintf("reset button 4 sending tap code\n");
            tap_code16(KC_BTN4);
        break;
        case TD_SINGLE_HOLD:
            xprintf("Hold for button 4 reset\n");
            is_drag_scroll = false;
            btn4_held = false;
        break;
        default:
        break;
    }
}

// Handle the possible states for each tapdance keycode you define:
void mseBtn5_finished(tap_dance_state_t *state, void *user_data) {
    td_state = cur_dance(state);
    switch (td_state) {
        case TD_SINGLE_HOLD:
            xprintf("Turning on layer 1 for button 5\n");
            layer_on(1);
        break;
        default:
        break;
    }
}

void mseBtn5_reset(tap_dance_state_t *state, void *user_data) {
    switch (td_state) {
        case TD_SINGLE_TAP:
            tap_code16(KC_BTN5);
        break;
        case TD_SINGLE_HOLD:
            layer_off(1);
        break;
        default:
        break;
    }
}

// Define `ACTION_TAP_DANCE_FN_ADVANCED()` for each tapdance keycode, passing in `finished` and `reset` functions
tap_dance_action_t tap_dance_actions[] = {
    [MSE_BTN5_LAYR_1]  = ACTION_TAP_DANCE_FN_ADVANCED(NULL, mseBtn5_finished, mseBtn5_reset),
    [MSE_BTN4_DRAG]   = ACTION_TAP_DANCE_FN_ADVANCED(NULL, mseBtn4_finished, mseBtn4_reset)
};
