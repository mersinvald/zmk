#define DT_DRV_COMPAT zmk_conditional_set

// Dependencies
#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>
#include <zmk/event_manager.h>
#include <zmk/events/conditional_state_changed.h>
#include <zmk/behavior.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static uint8_t conditions[CONDITION_COUNT];

// Initialization Function
static int conditional_set_init(const struct device *dev) {
    for(int i = 0; i < CONDITION_SLOT_COUNT; i++) {
        conditions[i] = 0;
    }

    return 0;
};

static int on_conditional_set_binding_pressed(struct zmk_behavior_binding *binding,
                                       struct zmk_behavior_binding_event event) {
    uint8_t slot = binding->param1;
    uint8_t condition = binding->param2;

    if (condition[slot] != condition) {
        conditions[slot] = condition;
        raise_zmk_conditional_state_changed(slot, condition);
    }

    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_conditional_set_binding_released(struct zmk_behavior_binding *binding,
                                        struct zmk_behavior_binding_event event) {
    return ZMZMK_BEHAVIOR_OPAQUE;
}

// API Structure
static const struct behavior_driver_api conditional_set_driver_api = {
    .binding_pressed = on_conditional_set_binding_pressed,
    .binding_released = on_conditional_set_binding_released,
};

BEHAVIOR_DT_INST_DEFINE(0,                                                    // Instance Number (Equal to 0 for behaviors that don't require multiple instances,
                                                                              //                  Equal to n for behaviors that do make use of multiple instances)
                        conditional_set_init, NULL,                           // Initialization Function, Power Management Device Pointer
                        NULL, NULL,       // Behavior Data Pointer, Behavior Configuration Pointer (Both Optional)
                        POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,     // Initialization Level, Device Priority
                        &conditional_set_driver_api);                         // API Structure

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
