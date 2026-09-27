/* Application-owned PC/Switch USB profiles. CubeMX hooks are in USER CODE sections. */
#include "usbd_customhid.h"
#include "usbd_desc.h"
#include "usbd_core.h"
#include "usbd_ctlreq.h"
#include "controller_usb.h"
#include "board_config.h"
#include "usb_device.h"
#include "usbd_custom_hid_if.h"
#include "switch_report_descriptor.h"
#include <string.h>
extern USBD_HandleTypeDef hUsbDeviceFS;
static controller_mode mode = CONTROLLER_PC;
static bool reconnecting, neutral_pending = true;
static uint32_t disconnected_at, last_report_ms;

static uint8_t tx_buffer[64] __attribute__((aligned(4)));
static uint8_t control_buffer[64] __attribute__((aligned(4)));
static uint8_t latest_report[64] __attribute__((aligned(4)));
static uint16_t latest_length = sizeof(joystick_report);
static USBD_ClassTypeDef controller_class;
static USBD_DescriptorsTypeDef controller_descriptors;
static uint8_t switch_device[] = {
    18,1,0,2,0,0,0,64,0x0d,0x0f,0x92,0,0,1,1,2,0,1
};
static uint8_t switch_config[] = {
    9,2,41,0,1,1,0,CONTROLLER_USB_ATTRIBUTES,CONTROLLER_USB_SWITCH_MAX_POWER,
    9,4,0,0,2,3,0,0,0,
    9,0x21,0x11,1,0,1,0x22,sizeof(switch_report_descriptor),0,
    7,5,0x02,3,64,0,1,
    7,5,0x81,3,64,0,1
};
static uint8_t pc_config[] = {
    9,2,41,0,1,1,0,CONTROLLER_USB_ATTRIBUTES,CONTROLLER_USB_PC_MAX_POWER,
    9,4,0,0,2,3,0,0,0,
    9,0x21,0x11,1,0,1,0x22,USBD_CUSTOM_HID_REPORT_DESC_SIZE,0,
    7,5,0x81,3,64,0,5,
    7,5,0x02,3,64,0,5
};
_Static_assert(sizeof(switch_report_descriptor) == 86, "Switch descriptor length");
_Static_assert(sizeof(joystick_report) == 10, "PC report layout");
_Static_assert(USBD_CUSTOMHID_OUTREPORT_BUF_SIZE >= CUSTOM_HID_EPOUT_SIZE,
               "Set Custom HID OUT report buffer to 64 in CubeMX");
_Static_assert(USBD_LPM_ENABLED == 0 && USBD_SELF_POWERED == CONTROLLER_USB_SELF_POWERED,
               "Keep USB LPM disabled and USBD_SELF_POWERED tied to board_config.h");
static uint16_t encode(uint8_t *out, const joystick_report *s)
{
    return mode == CONTROLLER_SWITCH ? encode_switch_report(out, s) : encode_pc_report(out, s);
}
controller_mode controller_usb_mode(void) { return mode; }
uint16_t controller_usb_report_descriptor_size(void)
{
    return mode == CONTROLLER_SWITCH ? sizeof(switch_report_descriptor) : USBD_CUSTOM_HID_REPORT_DESC_SIZE;
}
uint8_t *controller_usb_configuration(uint16_t *length)
{
    *length = sizeof(pc_config);
    return mode == CONTROLLER_SWITCH ? switch_config : pc_config;
}
uint8_t *controller_usb_hid_descriptor(void)
{
    uint16_t length;
    return controller_usb_configuration(&length) + 18;
}
static int8_t hid_init(void)
{

    USBD_CUSTOM_HID_HandleTypeDef *hid = hUsbDeviceFS.pClassDataCmsit[hUsbDeviceFS.classId];
    if (hid != NULL) memset(hid, 0, sizeof(*hid));
    neutral_pending = true;
    joystick_report neutral = {0};
    latest_length = encode(latest_report, &neutral);
    return USBD_OK;
}
static int8_t hid_deinit(void) { return USBD_OK; }
static int8_t hid_out(uint8_t event, uint8_t state)
{
    (void)event; (void)state;
    return USBD_CUSTOM_HID_ReceivePacket(&hUsbDeviceFS) == USBD_OK ? USBD_OK : -1;
}
static uint8_t *hid_get_report(uint16_t *length)
{

    memcpy(control_buffer, latest_report, latest_length);
    *length = latest_length;
    return control_buffer;
}
static USBD_CUSTOM_HID_ItfTypeDef controller_fops = {
    .pReport = NULL, .Init = hid_init, .DeInit = hid_deinit,
    .OutEvent = hid_out
};
static void controller_usb_prepare(void)
{
    controller_fops.pReport = mode == CONTROLLER_SWITCH ? switch_report_descriptor : USBD_CustomHID_fops_FS.pReport;
    (void)hid_init();
}

static uint8_t controller_setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t *data = NULL;
    uint16_t length = 0;
    if ((req->bmRequest & USB_REQ_TYPE_MASK) == USB_REQ_TYPE_STANDARD &&
        req->bRequest == USB_REQ_GET_DESCRIPTOR) {
        if ((req->wValue >> 8) == CUSTOM_HID_REPORT_DESC) {
            data = controller_fops.pReport;
            length = controller_usb_report_descriptor_size();
        } else if ((req->wValue >> 8) == CUSTOM_HID_DESCRIPTOR_TYPE) {
            data = controller_usb_hid_descriptor();
            length = USB_CUSTOM_HID_DESC_SIZ;
        }
    } else if ((req->bmRequest & USB_REQ_TYPE_MASK) == USB_REQ_TYPE_CLASS &&
               req->bRequest == CUSTOM_HID_REQ_GET_REPORT) {
        if (req->wValue != 0x0100U) {
            USBD_CtlError(pdev, req);
            return USBD_FAIL;
        }
        data = hid_get_report(&length);
    }
    if (data != NULL) return USBD_CtlSendData(pdev, data, MIN(length, req->wLength));
    return USBD_CUSTOM_HID.Setup(pdev, req);
}

static uint8_t *controller_device_descriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    if (mode == CONTROLLER_PC) return FS_Desc.GetDeviceDescriptor(speed, length);
    *length = sizeof(switch_device);
    return switch_device;
}

static uint8_t controller_string[USBD_MAX_STR_DESC_SIZ] __attribute__((aligned(4)));
static uint8_t *controller_product(USBD_SpeedTypeDef speed, uint16_t *length)
{
    if (mode == CONTROLLER_PC) return FS_Desc.GetProductStrDescriptor(speed, length);
    USBD_GetString((uint8_t *)"POKKEN CONTROLLER", controller_string, length);
    return controller_string;
}

static uint8_t *controller_manufacturer(USBD_SpeedTypeDef speed, uint16_t *length)
{
    if (mode == CONTROLLER_PC) return FS_Desc.GetManufacturerStrDescriptor(speed, length);
    USBD_GetString((uint8_t *)"HORI CO.,LTD.", controller_string, length);
    return controller_string;
}

void controller_usb_init(void)
{
    controller_usb_prepare();
    controller_descriptors = FS_Desc;
    controller_descriptors.GetDeviceDescriptor = controller_device_descriptor;
    controller_descriptors.GetProductStrDescriptor = controller_product;
    controller_descriptors.GetManufacturerStrDescriptor = controller_manufacturer;
    controller_class = USBD_CUSTOM_HID;
    controller_class.Setup = controller_setup;
    controller_class.GetFSConfigDescriptor = controller_usb_configuration;
    controller_class.GetHSConfigDescriptor = controller_usb_configuration;
    controller_class.GetOtherSpeedConfigDescriptor = controller_usb_configuration;
    if (USBD_Init(&hUsbDeviceFS, &controller_descriptors, DEVICE_FS) != USBD_OK ||
        USBD_RegisterClass(&hUsbDeviceFS, &controller_class) != USBD_OK ||
        USBD_CUSTOM_HID_RegisterInterface(&hUsbDeviceFS, &controller_fops) != USBD_OK ||
        USBD_Start(&hUsbDeviceFS) != USBD_OK) Error_Handler();
}
void controller_usb_toggle(uint32_t now)
{
    if (reconnecting) return;

    if (USBD_DeInit(&hUsbDeviceFS) != USBD_OK) Error_Handler();
    mode = mode == CONTROLLER_PC ? CONTROLLER_SWITCH : CONTROLLER_PC;
    disconnected_at = now;
    reconnecting = true;
}
void controller_usb_poll(uint32_t now)
{
    if (reconnecting && (uint32_t)(now - disconnected_at) >= 250U) {
        MX_USB_DEVICE_Init();
        reconnecting = false;
    }
}
uint8_t controller_usb_send(const joystick_report *state, uint32_t now)
{
    if (reconnecting) return USBD_BUSY;
    uint8_t result = USBD_BUSY;
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    uint32_t interval = mode == CONTROLLER_SWITCH ? 1U : 5U;
    USBD_CUSTOM_HID_HandleTypeDef *hid = hUsbDeviceFS.pClassDataCmsit[hUsbDeviceFS.classId];
    if (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED && hid != NULL &&
        hid->state == CUSTOM_HID_IDLE && (uint32_t)(now - last_report_ms) >= interval) {
        joystick_report neutral = {0};
        uint16_t length = encode(tx_buffer, neutral_pending ? &neutral : state);

        hid->state = CUSTOM_HID_BUSY;
        result = USBD_LL_Transmit(&hUsbDeviceFS, CUSTOM_HID_EPIN_ADDR, tx_buffer, length);
        if (result == USBD_OK) {
            memcpy(latest_report, tx_buffer, length);
            latest_length = length;
            neutral_pending = false;
            last_report_ms = now;
        } else {
            hid->state = CUSTOM_HID_IDLE;
        }
    }
    __set_PRIMASK(primask);
    return result;
}
