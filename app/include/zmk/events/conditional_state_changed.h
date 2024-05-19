/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <zephyr/kernel.h>
#include <zmk/event_manager.h>

#define CONDITION_SLOT_COUNT 10

struct zmk_conditional_state_changed {
    uint8_t slot;
    uint8_t state;
    int64_t timestamp;
};

ZMK_EVENT_DECLARE(zmk_conditional_state_changed);

static inline int raise_layer_state_changed(uint8_t slot, uint8_t state) {
    return raise_zmk_conditional_state_changed((struct zmk_conditional_state_changed){
        .slot = slot, .state = state, .timestamp = k_uptime_get()});
}
