#ifdef _MSC_VER
#define __attribute__(x)
#endif
#include "controller_logic.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    mode_gesture g = {0};
    assert(!mode_gesture_update(&g, MODE_BUTTONS, 100));
    assert(!mode_gesture_update(&g, MODE_BUTTONS, 2099));
    assert(mode_gesture_update(&g, MODE_BUTTONS, 2100));
    assert(!mode_gesture_update(&g, MODE_BUTTONS, 9000));
    assert(!mode_gesture_update(&g, 1U << 8, 9100));
    assert(!mode_gesture_update(&g, MODE_BUTTONS, 12000));
    assert(!mode_gesture_update(&g, 0, 12001));
    assert(!mode_gesture_update(&g, MODE_BUTTONS, 12002));
    assert(!mode_gesture_update(&g, 1U << 9, 13000));
    assert(!mode_gesture_update(&g, MODE_BUTTONS, 13001));
    assert(!mode_gesture_update(&g, MODE_BUTTONS, 15000));
    assert(mode_gesture_update(&g, MODE_BUTTONS, 15001));
    g = (mode_gesture){0};
    uint32_t start = UINT32_MAX - 999U;
    assert(!mode_gesture_update(&g, MODE_BUTTONS, start));
    assert(!mode_gesture_update(&g, MODE_BUTTONS, start + 1999U));
    assert(mode_gesture_update(&g, MODE_BUTTONS, start + 2000U));

    assert(switch_axis(-32767) == 0);
    assert(switch_axis(0) == 128);
    assert(switch_axis(32767) == 255);
    unsigned previous = 0;
    for (int32_t x = -32768; x <= 32767; ++x) {
        unsigned value = switch_axis((int16_t)x);
        assert(value >= previous && value <= 255);
        previous = value;
    }
    uint8_t out[12];
    joystick_report state = {0};
    const unsigned switch_bits[] = {2,1,3,0,4,5,6,7,8,9,10,11,12,13};
    for (unsigned i = 0; i < 14; ++i) {
        memset(out, 0xa5, sizeof(out));
        state.buttons = (uint16_t)(1U << i);
        assert(encode_switch_report(out, &state) == 8);
        assert((out[0] | ((unsigned)out[1] << 8)) == (1U << switch_bits[i]));
        assert(out[2] == 8 && out[3] == 128 && out[4] == 128);
        assert(out[5] == 128 && out[6] == 128 && out[7] == 0 && out[8] == 0xa5);
    }
    state = (joystick_report){0x1234, -32767, 0, 32767, -1};
    const uint8_t expected[] = {0x34,0x12,0x01,0x80,0,0,0xff,0x7f,0xff,0xff};
    memset(out, 0xa5, sizeof(out));
    assert(encode_pc_report(out, &state) == 10);
    assert(memcmp(out, expected, 10) == 0 && out[10] == 0xa5);
    puts("PASS: hold/release, interrupted hold, rollover, all axes, all buttons, wire format");
    return 0;
}
