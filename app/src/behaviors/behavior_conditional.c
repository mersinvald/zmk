#define DT_DRV_COMPAT zmk_conditional

// Dependencies
#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/events/conditional_state_changed.h>
#include <zmk/behavior.h>
#include <zmk/behavior_queue.h>
#include <zmk/keys.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

struct behavior_conditional_config {
    int slot;
    uint32_t count;
    struct zmk_behavior_binding bindings[];
};

struct behavior_conditional_data {
    uint8_t slot;
    uint8_t condition;
};

struct behavior_conditional_data this_conditional_data;

// Initialization Function
static int conditional_init(const struct device *dev) {
    static bool init_first_run = true;
    struct behavior_conditional_config *config = (struct behavior_conditional_config *) dev->config;

    if (init_first_run) {
        if (config->slot < 0 || config->slot > CONDITION_SLOT_COUNT) {
            LOG_ERR("Slot is out-of-bounds");
            return -EINVAL;
        }
        this_conditional_data.slot = (uint8_t) config->slot;
        this_conditional_data.condition = 0;
    }

    init_first_run = false;
};

static uint32_t condition_to_encoded_keycode(uint8_t condition) {
    switch condition {
        case 0: return KP_N0,
        case 1: return KP_N1,
        case 2: return KP_N2,
        case 3: return KP_N3,
        case 4: return KP_N4,
        case 5: return KP_N5,
        case 6: return KP_N6,
        case 7: return KP_N7,
        case 8: return KP_N8,
        case 9: return KP_N9,
        default: return 0,
    }
}

static int on_conditional_binding_pressed(struct zmk_behavior_binding *binding,
                                       struct zmk_behavior_binding_event event) {
    uint8_t condition = this_conditional_data.condition;
    struct behavior_conditional_config *config = (struct behavior_conditional_config *) binding->behavior_dev->config;
    struct zmk_behavior_binding binding = bindings[condition];
    zmk_behavior_queue_add(event->position, binding, true, 10);
    zmk_behavior_queue_add(event->position, binding, false, 10);
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_conditional_binding_released(struct zmk_behavior_binding *binding,
                                        struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

// API Structure
static const struct behavior_driver_api conditional_set_driver_api = {
    .binding_pressed = on_conditional_binding_pressed,
    .binding_released = on_conditional_binding_released,
};

static int conditional_state_changed_listener(const zmk_event_t *eh);

ZMK_LISTENER(DT_DRV_COMPAT, conditional_state_changed_listener);
ZMK_SUBSCRIPTION(DT_DRV_COMPAT, zmk_conditional_state_changed);

static int tap_dance_position_state_changed_listener(const zmk_event_t *eh){
    const struct zmk_conditional_state_changed *event = as_zmk_conditional_state_changed(eh);

    if (this_conditional_data.slot == event->slot) {
        this_conditional_data.condition = event->condition;
    }

    return ZMK_EV_EVENT_CAPTURED;
}

#define TRANSFORMED_BEHAVIORS(n)                                                                   \
    {LISTIFY(DT_PROP_LEN(n, bindings), ZMK_KEYMAP_EXTRACT_BINDING, (, ), n)},


#define KP_INST(n)                                                                                       \
    static struct behavior_conditional_config behavior_conditional_config_##n = {                        \
        .slot = DT_INST_PROP(n, slot),                                                                   \
        .count = DT_PROP_LEN(n, bindings),                                                            \
        .bindings = TRANSFORMED_BEHAVIORS(n)                                                          \
    };                                                                                                   \
    BEHAVIOR_DT_INST_DEFINE(n, behavior_conditional_init, NULL, NULL, &behavior_conditional_config_##n,  \
                            POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                            \
                            &behavior_conditional_driver_api);

DT_INST_FOREACH_STATUS_OKAY(KP_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
