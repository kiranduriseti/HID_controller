"""Check the regeneration contract; optionally inspect a CubeMX-generated copy.

Usage: python tests/check_regeneration.py [path/to/regenerated/project]
"""
from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parents[1]
target = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else root

def region(text, name):
    pattern = r'/\* USER CODE BEGIN ' + re.escape(name) + r' \*/(.*?)/\* USER CODE END ' + re.escape(name) + r' \*/'
    return re.search(pattern, text, re.S).group(1)

ioc = (target / 'HID_controller.ioc').read_text()
for setting in [
    'ProjectManager.KeepUserCode=true',
    'USB_DEVICE.USBD_CUSTOMHID_OUTREPORT_BUF_SIZE-CUSTOM_HID_FS=64',
    'USB_DEVICE.USBD_LPM_ENABLED=0',
    'USB_DEVICE.USBD_SELF_POWERED=0',
    'PC0.Signal=GPIO_Input',
]:
    assert setting in ioc, setting

assert 'NVIC.EXTI0_IRQn=true' not in ioc
assert 'GPXTI0' not in ioc
gpio = (target / 'Core/Src/gpio.c').read_text()
assert 'GPIO_MODE_IT_FALLING' not in gpio
assert 'HAL_NVIC_EnableIRQ(EXTI0_IRQn)' not in gpio

usb = (target / 'USB_DEVICE/App/usb_device.c').read_text()
assert '#include "controller_usb.h"' in region(usb, 'Includes')
hook = region(usb, 'USB_DEVICE_Init_PreTreatment')
assert 'controller_usb_init();' in hook and 'return;' in hook
conf = (target / 'USB_DEVICE/Target/usbd_conf.h').read_text()
overrides = region(conf, 'INCLUDE')
for name, value in [('CUSTOM_HID_EPIN_SIZE', '64U'), ('CUSTOM_HID_EPOUT_SIZE', '64U'), ('CUSTOM_HID_EPOUT_ADDR', '0x02U')]:
    assert f'#define {name} {value}' in overrides
for name, value in [('USBD_LPM_ENABLED', '0U'), ('USBD_SELF_POWERED', '0U'), ('USBD_CUSTOMHID_OUTREPORT_BUF_SIZE', '64U')]:
    assert re.search(r'#define\s+' + name + r'\s+' + value + r'\b', conf), name
main = (target / 'Core/Src/main.c').read_text()
for header in ['joystick.h', 'main_loop.h', 'mpu.h']:
    assert header in region(main, 'Includes')
for call in ['joystick_start_scan();', 'mpu_init_gyro();']:
    assert call in region(main, '2')
assert 'main_loop();' in region(main, 'WHILE')

for path in ['Core/Src/main.c', 'USB_DEVICE/App/usb_device.c']:
    assert 'REGENERATION_CANARY_OUTSIDE_USER_CODE' not in (target / path).read_text(), path

descriptor = (target / 'USB_DEVICE/App/usbd_custom_hid_if.c').read_text()
assert 'USAGE_PAGE (Generic Desktop)' in region(descriptor, '0')
if target != root:
    assert region(descriptor, '0') == region((root / 'USB_DEVICE/App/usbd_custom_hid_if.c').read_text(), '0')
    # Custom modules/descriptors must survive byte-for-byte. Generated files may change.
    for name in ['buttons.c', 'joystick.c', 'mpu.c', 'main_loop.c', 'controller_usb.c']:
        path = Path('Core/Src') / name
        assert (target / path).read_bytes() == (root / path).read_bytes(), path
    for name in ['controller_usb.h', 'controller_logic.h', 'switch_report_descriptor.h']:
        path = Path('Core/Inc') / name
        assert (target / path).read_bytes() == (root / path).read_bytes(), path
print('PASS: .ioc settings, protected hooks, generated buffer settings, polled MPU toggle, application ownership')
