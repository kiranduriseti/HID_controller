"""Validate the actual descriptor bytes without USB hardware."""
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]

def array(path, name):
    text = (root / path).read_text()
    text = re.sub(r'/\*.*?\*/|//[^\n]*', '', text, flags=re.S)
    body = re.search(r'\b' + name + r'\[[^\]]*\]\s*(?:__ALIGN_END\s*)?=\s*\{([^}]+)\}', text).group(1)
    body = body.replace('USBD_CUSTOM_HID_REPORT_DESC_SIZE', str(len(array('USB_DEVICE/App/usbd_custom_hid_if.c', 'CUSTOM_HID_ReportDesc_FS')))) if name.endswith('_config') else body
    body = re.sub(r'sizeof\((pc|switch)_report_descriptor\)', lambda m: str(len(array(f'Core/Inc/{m[1]}_report_descriptor.h', f'{m[1]}_report_descriptor'))), body)
    return bytes(int(v.strip(), 0) for v in body.split(',') if v.strip())

def report_bits(data):
    offset = size = count = depth = 0
    bits = {'input': 0, 'output': 0}
    while offset < len(data):
        prefix = data[offset]
        length = (0, 1, 2, 4)[prefix & 3]
        value = int.from_bytes(data[offset + 1:offset + 1 + length], 'little')
        offset += 1 + length
        assert offset <= len(data)
        tag = prefix & 0xfc
        if tag == 0x74: size = value
        elif tag == 0x94: count = value
        elif tag == 0x80: bits['input'] += size * count
        elif tag == 0x90: bits['output'] += size * count
        elif tag == 0xa0: depth += 1
        elif tag == 0xc0: depth -= 1
        assert depth >= 0
    assert depth == 0
    return bits

for mode, desc_size, input_bits, output_bits, interval in (
    ('pc', 56, 80, 0, 5), ('switch', 86, 64, 64, 1)
):
    report = array('USB_DEVICE/App/usbd_custom_hid_if.c', 'CUSTOM_HID_ReportDesc_FS') if mode == 'pc' else array('Core/Inc/switch_report_descriptor.h', 'switch_report_descriptor')
    assert len(report) == desc_size
    assert report_bits(report) == {'input': input_bits, 'output': output_bits}
    config = array('Core/Src/controller_usb.c', f'{mode}_config')
    assert len(config) == int.from_bytes(config[2:4], 'little') == 41
    assert int.from_bytes(config[25:27], 'little') == desc_size
    assert config[7] == 0x80
    endpoints = {}
    pos = 0
    while pos < len(config):
        length, kind = config[pos:pos + 2]
        assert length > 0 and pos + length <= len(config)
        if kind == 5:
            endpoints[config[pos+2]] = config[pos:pos+length]
        pos += length
    assert set(endpoints) == {0x81, 0x02}
    for endpoint in endpoints.values():
        assert endpoint[3] == 3
        assert int.from_bytes(endpoint[4:6], 'little') == 64
        assert endpoint[6] == interval
device = array('Core/Src/controller_usb.c', 'switch_device')
assert len(device) == 18 and device[8:12] == bytes.fromhex('0d0f9200')
assert device[16] == 0
print('PASS: descriptor lengths, collections, report bit counts, endpoints, intervals, Switch identity')

assert array('USB_DEVICE/App/usbd_custom_hid_if.c', 'CUSTOM_HID_ReportDesc_FS') == array('tests/reference_pc_descriptor.h', 'pc_report_descriptor')
print('PASS: original annotated PC descriptor bytes preserved in the original file')
