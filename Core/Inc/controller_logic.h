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


#define PACK_LSB_16(out, i, value) \
    do { \
        (out)[(i)++] = (uint8_t)(value); \
        (out)[(i)++] = (uint8_t)((uint16_t)(value) >> 8); \
    } while (0)

static inline uint16_t encode_pc_report(uint8_t *out, const joystick_report *s)
{
    uint16_t i = 0;

    PACK_LSB_16(out, i, s->buttons);
    PACK_LSB_16(out, i, s->lx);
    PACK_LSB_16(out, i, s->ly);
    PACK_LSB_16(out, i, s->rx);
    PACK_LSB_16(out, i, s->ry);

    return (i == sizeof(joystick_report)) ? i : 0;
}

static inline uint8_t switch_axis(int16_t value)
{
    return (uint8_t)(((int32_t)value + 32768) >> 8);
}

static inline uint16_t encode_switch_report(uint8_t *out, const joystick_report *s)
{
    uint16_t b = (s->buttons & 0x3ff0U) | ((s->buttons & 1U) << 2)
               | (s->buttons & 2U) | ((s->buttons & 4U) << 1)
               | ((s->buttons & 8U) >> 3);

    uint16_t i = 0;

    PACK_LSB_16(out, i, b);
    out[i++] = 8;
    out[i++] = switch_axis(s->lx);
    out[i++] = switch_axis(s->ly);
    out[i++] = switch_axis(s->rx);
    out[i++] = switch_axis(s->ry);
    out[i++] = 0;

    return i; //should be 8
}
#endif
