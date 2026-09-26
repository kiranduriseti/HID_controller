/* Original PC descriptor, with its field annotations preserved. */
static uint8_t pc_report_descriptor[] = {
0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
		    0x09, 0x04,                    // USAGE (Joystick)
		    0xa1, 0x01,                    // COLLECTION (Application)
		    0x09, 0x01,                    //   USAGE (Pointer)
		    0xa1, 0x00,                    //   COLLECTION (Physical)
		    0x05, 0x09,                    //     USAGE_PAGE (Button)
		    0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
		    0x29, 0x0e,                    //     USAGE_MAXIMUM (Button 14)
		    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
		    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
		    0x95, 0x0e,                    //     REPORT_COUNT (14)
		    0x75, 0x01,                    //     REPORT_SIZE (1)
		    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
		    0x95, 0x01,                    //     REPORT_COUNT (1)
		    0x75, 0x02,                    //     REPORT_SIZE (2)
		    0x81, 0x01,                    //     INPUT (Cnst,Ary,Abs)
		    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
		    0x09, 0x30,                    //     USAGE (X)
		    0x09, 0x31,                    //     USAGE (Y)
		    0x09, 0x32,                    //     USAGE (Z)
		    0x09, 0x33,                    //     USAGE (Rx)
		    0x16, 0x01, 0x80,              //     LOGICAL_MINIMUM (-32767)
		    0x26, 0xff, 0x7f,              //     LOGICAL_MAXIMUM (32767)
		    0x75, 0x10,                    //     REPORT_SIZE (16)
		    0x95, 0x04,                    //     REPORT_COUNT (4)
		    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
		    0xc0,                          //     END_COLLECTION

    0xC0    /*     END_COLLECTION	             */
};
