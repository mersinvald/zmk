#define DT_DRV_COMPAT zmk_behavior_cond

// Dependencies
#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/events/conditional_state_changed.h>
#include <zmk/behavior.h>
#include <zmk/behavior_queue.h>
#include <zmk/keymap.h>
#include <zmk/keys.h>
#include <zmk/matrix.h>
#include <zmk/hid.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

struct conditional_config {
    int slot;
    uint32_t count;
    struct zmk_behavior_binding* bindings;
};

uint8_t conditions[CONDITION_SLOT_COUNT] = {0};

// Initialization Function
static int conditional_init(const struct device *dev) {
    struct conditional_config *config = (struct conditional_config *) dev->config;

    LOG_DBG("slot %d (state: %d) with %d bindings", config->slot, 0, config->count);

    if (config->slot < 0 || config->slot > CONDITION_SLOT_COUNT) {
        LOG_ERR("requested slot is out-of-bounds: assert(0 < %d < %d)", config->slot, CONDITION_SLOT_COUNT);
        return -EINVAL;
    }

    return 0;
};

static int on_conditional_binding_pressed(struct zmk_behavior_binding *binding,
                                       struct zmk_behavior_binding_event event) {    
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct conditional_config *cfg = dev->config;
    uint8_t condition = conditions[cfg->slot];

    if (condition >= cfg->count) {
        LOG_ERR("condition references out-of-bounds behavior: assert(%d < %d)", condition, cfg->count);
        return ZMK_BEHAVIOR_OPAQUE;
    }

    struct zmk_behavior_binding target = cfg->bindings[condition];

    zmk_behavior_queue_add(event.position, target, true, 0);
    zmk_behavior_queue_add(event.position, target, false, 0);

    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_conditional_binding_released(struct zmk_behavior_binding *binding,
                                        struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

// API Structure
static const struct behavior_driver_api conditional_driver_api = {
    .binding_pressed = on_conditional_binding_pressed,
    .binding_released = on_conditional_binding_released,
};

static int conditional_state_changed_listener(const zmk_event_t *eh);

ZMK_LISTENER(DT_DRV_COMPAT, conditional_state_changed_listener);
ZMK_SUBSCRIPTION(DT_DRV_COMPAT, zmk_conditional_state_changed);

static int conditional_state_changed_listener(const zmk_event_t *eh){
    const struct zmk_conditional_state_changed *event = as_zmk_conditional_state_changed(eh);
    
    if (event->slot >= CONDITION_SLOT_COUNT) {
        LOG_ERR("requested slot if out-of-bounds: assert(%d < %d)", event->slot, CONDITION_SLOT_COUNT);
        return ZMK_EV_EVENT_CAPTURED;
    }
    
    conditions[event->slot] = event->state;

    return ZMK_EV_EVENT_CAPTURED;
}

#define _TRANSFORM_ENTRY(idx, node) ZMK_KEYMAP_EXTRACT_BINDING(idx, node)

#define TRANSFORMED_BINDINGS(node)                                                          \
    { LISTIFY(DT_INST_PROP_LEN(node, bindings), _TRANSFORM_ENTRY, (, ), DT_DRV_INST(node)) }

#define CONDITIONAL_INST(n)                                                                 \
    static struct zmk_behavior_binding                                                      \
        conditional_config_##n##_bindings[DT_INST_PROP_LEN(n, bindings)] =                  \
            TRANSFORMED_BINDINGS(n);                                                        \
    static struct conditional_config conditional_config_##n = {                             \
        .slot = DT_INST_PROP_OR(n, slot, 0),                                                \
        .count = DT_INST_PROP_LEN(n, bindings),                                             \
        .bindings = conditional_config_##n##_bindings                                       \
    };                                                                                      \
    BEHAVIOR_DT_INST_DEFINE(n, conditional_init, NULL, NULL, &conditional_config_##n,       \
                            POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,               \
                            &conditional_driver_api);

DT_INST_FOREACH_STATUS_OKAY(CONDITIONAL_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
