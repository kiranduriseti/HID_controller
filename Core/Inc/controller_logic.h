#ifndef CONTROLLER_LOGIC_H
#define CONTROLLER_LOGIC_H
#include <stdbool.h>
#include <stdint.h>
#include "joystick.h"

typedef enum { CONTROLLER_PC, CONTROLLER_SWITCH } controller_mode;
#define MODE_BUTTONS ((1U << 8) | (1U << 9))
typedef struct { uint32_t since; bool tracking; bool latched; } mode_gesture;

static inline bool mode_gesture_update(mode_gesture *g, uint16_t buttons, uint32_t now)
{
    uint16_t held = buttons & MODE_BUTTONS;
    if (held == 0) g->latched = false;
    if (held != MODE_BUTTONS || g->latched) { g->tracking = false; return false; }
    if (!g->tracking) { g->tracking = true; g->since = now; }
    if ((uint32_t)(now - g->since) < 2000U) return false;
    g->latched = true;
    g->tracking = false;
    return true;
}

static inline uint8_t switch_axis(int16_t value)
{
    return (uint8_t)(((int32_t)value + 32768) >> 8);
}

static inline uint16_t encode_pc_report(uint8_t *out, const joystick_report *s)
{
    uint16_t values[5] = {s->buttons, (uint16_t)s->lx, (uint16_t)s->ly,
                          (uint16_t)s->rx, (uint16_t)s->ry};
    for (unsigned i = 0; i < 5; ++i) {
        out[2*i] = (uint8_t)values[i]; out[2*i+1] = (uint8_t)(values[i] >> 8);
    }
    return 10;
}

static inline uint16_t encode_switch_report(uint8_t *out, const joystick_report *s)
{
    uint16_t b = (s->buttons & 0x3ff0U) | ((s->buttons & 1U) << 2)
               | (s->buttons & 2U) | ((s->buttons & 4U) << 1)
               | ((s->buttons & 8U) >> 3);
    out[0] = (uint8_t)b; out[1] = (uint8_t)(b >> 8);
    out[2] = 8;
    out[3] = switch_axis(s->lx); out[4] = switch_axis(s->ly);
    out[5] = switch_axis(s->rx); out[6] = switch_axis(s->ry);
    out[7] = 0;
    return 8;
}
#endif
