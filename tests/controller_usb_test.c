#define __USBD_CUSTOM_HID_IF_H__
#define __USB_CUSTOMHID_H
#define __USBD_DESC__C__
#define __USBD_CORE_H
#define __USB_REQUEST_H
#define __USB_DEVICE__H__
#ifdef _MSC_VER
#define __attribute__(x)
#define _Static_assert static_assert
#endif
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
enum { USBD_OK, USBD_BUSY, USBD_EMEM, USBD_FAIL, USBD_STATE_CONFIGURED };
enum { CUSTOM_HID_IDLE, CUSTOM_HID_BUSY };
#define USBD_CUSTOMHID_OUTREPORT_BUF_SIZE 64
#define USBD_CUSTOM_HID_REPORT_DESC_SIZE 56
#define CUSTOM_HID_EPOUT_SIZE 64
#define CUSTOM_HID_EPIN_ADDR 0x81
#define USBD_LPM_ENABLED 0
#define USBD_SELF_POWERED 0
#define USBD_MAX_STR_DESC_SIZ 512
#define DEVICE_FS 0
#define USB_REQ_TYPE_MASK 0x60
#define USB_REQ_TYPE_STANDARD 0
#define USB_REQ_TYPE_CLASS 0x20
#define USB_REQ_GET_DESCRIPTOR 6
#define CUSTOM_HID_REPORT_DESC 0x22
#define CUSTOM_HID_DESCRIPTOR_TYPE 0x21
#define CUSTOM_HID_REQ_GET_REPORT 1
#define USB_CUSTOM_HID_DESC_SIZ 9
#define MIN(a,b) ((a)<(b)?(a):(b))
typedef int USBD_SpeedTypeDef;
typedef struct { uint8_t bmRequest, bRequest; uint16_t wValue, wIndex, wLength; } USBD_SetupReqTypedef;
typedef struct { int state; } USBD_CUSTOM_HID_HandleTypeDef;
typedef struct { unsigned classId; int dev_state; void *pClassDataCmsit[1]; } USBD_HandleTypeDef;
typedef struct {
    uint8_t *pReport;
    int8_t (*Init)(void);
    int8_t (*DeInit)(void);
    int8_t (*OutEvent)(uint8_t, uint8_t);
} USBD_CUSTOM_HID_ItfTypeDef;
typedef struct {
    uint8_t *(*GetDeviceDescriptor)(USBD_SpeedTypeDef, uint16_t *);
    uint8_t *(*GetProductStrDescriptor)(USBD_SpeedTypeDef, uint16_t *);
    uint8_t *(*GetManufacturerStrDescriptor)(USBD_SpeedTypeDef, uint16_t *);
} USBD_DescriptorsTypeDef;
typedef struct {
    uint8_t (*Setup)(USBD_HandleTypeDef *, USBD_SetupReqTypedef *);
    uint8_t *(*GetFSConfigDescriptor)(uint16_t *);
    uint8_t *(*GetHSConfigDescriptor)(uint16_t *);
    uint8_t *(*GetOtherSpeedConfigDescriptor)(uint16_t *);
} USBD_ClassTypeDef;
static unsigned forwarded, stalls;
static uint8_t fallback_setup(USBD_HandleTypeDef *d, USBD_SetupReqTypedef *r) { (void)d; (void)r; ++forwarded; return USBD_OK; }
static uint8_t pc_identity[] = {18,1,0,2,0,0,0,64,0x83,4,0x50,0x57,0,2,1,2,3,1};
static uint8_t *pc_device(USBD_SpeedTypeDef s, uint16_t *n) { (void)s; *n=sizeof(pc_identity); return pc_identity; }
static USBD_ClassTypeDef USBD_CUSTOM_HID = { .Setup = fallback_setup };
static USBD_DescriptorsTypeDef FS_Desc = { pc_device, pc_device, pc_device };
static USBD_ClassTypeDef *registered_class;
static uint8_t *control_data;
static uint16_t control_length;
static uint8_t USBD_CtlSendData(USBD_HandleTypeDef *d, uint8_t *data, uint16_t len) { (void)d; control_data=data; control_length=len; return USBD_OK; }
static void USBD_CtlError(USBD_HandleTypeDef *d, USBD_SetupReqTypedef *r) { (void)d; (void)r; ++stalls; }
static void USBD_GetString(uint8_t *s, uint8_t *out, uint16_t *n) { *n=(uint16_t)strlen((char *)s); memcpy(out,s,*n); }
static int USBD_Init(USBD_HandleTypeDef *d, USBD_DescriptorsTypeDef *desc, unsigned id) { (void)d; (void)desc; (void)id; return USBD_OK; }
static int USBD_RegisterClass(USBD_HandleTypeDef *d, USBD_ClassTypeDef *c) { (void)d; registered_class=c; return USBD_OK; }
static int USBD_CUSTOM_HID_RegisterInterface(USBD_HandleTypeDef *d, USBD_CUSTOM_HID_ItfTypeDef *f) { (void)d; (void)f; return USBD_OK; }
static int USBD_Start(USBD_HandleTypeDef *d) { (void)d; return USBD_OK; }
static uint8_t mock_pc_descriptor[USBD_CUSTOM_HID_REPORT_DESC_SIZE] = {0};
static USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops_FS = { .pReport = mock_pc_descriptor };
static uint32_t __get_PRIMASK(void) { return 0; }
static void __disable_irq(void) {}
static void __set_PRIMASK(uint32_t p) { (void)p; }
static void Error_Handler(void) { abort(); }
static int USBD_DeInit(USBD_HandleTypeDef *);
static int USBD_CUSTOM_HID_ReceivePacket(USBD_HandleTypeDef *);
static uint8_t USBD_LL_Transmit(USBD_HandleTypeDef *, unsigned, uint8_t *, uint16_t);
static void MX_USB_DEVICE_Init(void);
#include "../Core/Src/controller_usb.c"
USBD_HandleTypeDef hUsbDeviceFS;
static USBD_CUSTOM_HID_HandleTypeDef transport;
static uint8_t *in_flight;
static uint16_t in_flight_length;
static unsigned sends, starts, stops;
static bool fail_send;
static int USBD_DeInit(USBD_HandleTypeDef *dev)
{
    ++stops; dev->dev_state = 0; dev->pClassDataCmsit[0] = NULL; in_flight = NULL;
    return USBD_OK;
}
static int USBD_CUSTOM_HID_ReceivePacket(USBD_HandleTypeDef *dev) { (void)dev; return USBD_OK; }
static uint8_t USBD_LL_Transmit(USBD_HandleTypeDef *dev, unsigned ep, uint8_t *buf, uint16_t len)
{
    (void)dev; assert(ep == 0x81);
    if (fail_send) return USBD_FAIL;
    assert(transport.state == CUSTOM_HID_BUSY);
    transport.state = CUSTOM_HID_BUSY; in_flight = buf; in_flight_length = len; ++sends;
    return USBD_OK;
}
static void MX_USB_DEVICE_Init(void) { ++starts; controller_usb_init(); }
static void configure(void)
{
    transport.state = CUSTOM_HID_IDLE;
    hUsbDeviceFS.dev_state = USBD_STATE_CONFIGURED;
    hUsbDeviceFS.pClassDataCmsit[0] = &transport;
    controller_fops.Init();
}
int main(void)
{
    joystick_report input = {1, 32767, -32767, 0, 0};
    controller_usb_init();
    controller_usb_send(&input, 10); assert(sends == 0);
    configure(); controller_usb_send(&input, 10);
    assert(sends == 1 && in_flight_length == 10);
    for (unsigned i = 0; i < 10; ++i) assert(in_flight[i] == 0);
    uint8_t saved[10]; memcpy(saved, in_flight, 10);
    controller_usb_send(&input, 20);
    assert(sends == 1 && memcmp(saved, in_flight, 10) == 0);
    transport.state = CUSTOM_HID_IDLE;
    controller_usb_send(&input, 20);
    assert(sends == 2 && in_flight[0] == 1 && in_flight[2] == 0xff);
    transport.state = CUSTOM_HID_IDLE;
    assert(controller_usb_send(&input, 21) == USBD_BUSY && sends == 2);
    uint16_t length = 64;
    uint8_t *control = hid_get_report(&length);
    memcpy(saved, control, 10);
    input.buttons = 2; transport.state = CUSTOM_HID_IDLE;
    controller_usb_send(&input, 25);
    assert(memcmp(saved, control, 10) == 0);
    controller_usb_toggle(100);
    assert(stops == 1 && controller_usb_mode() == CONTROLLER_SWITCH);
    controller_usb_poll(349); assert(starts == 0);
    controller_usb_send(&input, 349); assert(sends == 3);
    controller_usb_poll(350); assert(starts == 1);
    assert(controller_usb_report_descriptor_size() == 86);
    configure(); controller_usb_send(&input, 351);
    assert(in_flight_length == 8 && in_flight[0] == 0 && in_flight[2] == 8 && in_flight[3] == 128);
    transport.state = CUSTOM_HID_IDLE;
    fail_send = true; assert(controller_usb_send(&input, 352) == USBD_FAIL);
    assert(sends == 4 && transport.state == CUSTOM_HID_IDLE);
    fail_send = false; controller_usb_send(&input, 352); assert(sends == 5);
    assert(in_flight[0] == 2 && in_flight[3] == 255 && in_flight[4] == 0);
    controller_usb_toggle(UINT32_MAX - 99U);
    controller_usb_poll(149); assert(starts == 1);
    controller_usb_poll(150); assert(starts == 2);
    assert(controller_usb_mode() == CONTROLLER_PC && controller_usb_report_descriptor_size() == 56);
    USBD_SetupReqTypedef request = {0x81, USB_REQ_GET_DESCRIPTOR, 0x2200, 0, 255};
    assert(registered_class->Setup(&hUsbDeviceFS, &request) == USBD_OK && control_length == 56);
    assert(control_data == mock_pc_descriptor);
    mode = CONTROLLER_SWITCH; controller_usb_prepare();
    assert(registered_class->Setup(&hUsbDeviceFS, &request) == USBD_OK && control_length == 86);
    request.wLength = 7;
    registered_class->Setup(&hUsbDeviceFS, &request); assert(control_length == 7);
    request.wValue = 0x2100; request.wLength = 255;
    registered_class->Setup(&hUsbDeviceFS, &request); assert(control_length == 9 && control_data[7] == 86);
    request.bmRequest = 0xa1; request.bRequest = CUSTOM_HID_REQ_GET_REPORT; request.wValue = 0x0100;
    registered_class->Setup(&hUsbDeviceFS, &request); assert(control_length == 8);
    request.wValue = 0x0200;
    assert(registered_class->Setup(&hUsbDeviceFS, &request) == USBD_FAIL && stalls == 1);
    request.bRequest = 10; registered_class->Setup(&hUsbDeviceFS, &request); assert(forwarded == 1);
    assert(USBD_CUSTOM_HID.Setup == fallback_setup);
    puts("PASS: unconfigured/busy/failing USB, buffer lifetime, EP0 isolation, neutral startup, reconnect and rollover");
    return 0;
}
