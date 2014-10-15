/* Copyright (c) 2008 Joerg Wunsch, Axel Wachtler
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   * Neither the name of the authors nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE. */

/* $Id$ */

/*
 * The USB specification can be found at:
 *
 * Base spec: http://www.usb.org/developers/docs/usb_20_040908.zip
 * CDC spec:  http://www.usb.org/developers/devclass_docs/CDC1.2_WMC1.1.zip
 *
 * All field and constant names in the structures below are taken
 * literally from those standard documents for easy cross-reference.
 */

struct device_descriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint8_t bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t iManufacturer;
    uint8_t iProduct;
    uint8_t iSerialNumber;
    uint8_t bNumConfigurations;
};

struct configuration_descriptor_header
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wTotalLength;
    uint8_t bNumInterfaces;
    uint8_t bConfigurationValue;
    uint8_t iConfiguration;
    uint8_t bmAttributes;
    uint8_t bMaxPower;
};

struct header_functional_descriptor
{
    uint8_t bFunctionLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint16_t bcdCDC;
};

struct union_functional_descriptor
{
    uint8_t bFunctionLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bControlInterface;
    uint8_t bSubordinateInterface0;
};

struct call_management_functional_descriptor
{
    uint8_t bFunctionLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bmCapabilities;
    uint8_t bDataInterface;
};

struct abstract_control_management_functional_descriptor
{
    uint8_t bFunctionLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bmCapabilities;
};


struct interface_descriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iInterface;
};

struct endpoint_descriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bEndpointAddress;
    uint8_t bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t bInterval;
};

struct configuration_descriptor
{
    struct configuration_descriptor_header conf_header;
    struct
    {
        struct interface_descriptor iface;
        struct header_functional_descriptor header_functional;
        struct union_functional_descriptor union_functional;
        struct call_management_functional_descriptor call_management;
        struct abstract_control_management_functional_descriptor
            abstract_control_management;
        struct endpoint_descriptor endpoint0;
    } ctrl_iface;
    struct
    {
        struct interface_descriptor iface;
        struct endpoint_descriptor endpoint0;
        struct endpoint_descriptor endpoint1;
    } data_iface;
};

/*
 * The supported language IDs are described in the string descriptor
 * with index 0.
 */
struct string_descriptor_langs
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wLANGID[1];        /* Only one language supported. */
};

struct string_descriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    wchar_t bString[];
};

/*
 * Implied data structure by SET_LINE_CODING/GET_LINE_CODING
 * PSTN-specific requests.
 */
struct line_coding
{
    uint32_t dwDTERate;         /* baud rate */
    uint8_t bCharFormat;        /* #stop bits:
                                 * 0 -> 1
                                 * 1 - 1.5
                                 * 2 - 2 */
    uint8_t bParityType;        /* 0 - none
                                 * 1 - odd
                                 * 2 - even
                                 * 3 - mark
                                 * 4 - space */
    uint8_t bDataBits;          /* 5 through 8, or 16 */
};

/*
 * Generic USB direction encoding, ORed into the respective fields.
 */
enum usb_direction
{
    DIR_OUT,
    DIR_IN = 0x80
};

/*
 * Type subfield of bmRequestType.
 */
enum bm_request_type
{
    RQTYPE_STANDARD = 0,
    RQTYPE_CLASS = 0x20,
    RQTYPE_VENDOR = 0x40,
    RQTYPE_MASK = 0xC0          /* 0xC0 itself is "reserved" */
};

/*
 * Recipient subfield of bmRequestType.
 */
enum bm_request_recipient
{
    RQRCPT_DEVICE,
    RQRCPT_INTERFACE,
    RQRCPT_ENDPOINT,
    RQRCPT_OTHER,
    RQRCPT_MASK = 0x1F
};

/*
 * Standard Feature Selectors.
 */
enum standard_feature_selectors
{
    /* feature */               /* valid for */
    ENDPOINT_HALT,              /* endpoint */
    DEVICE_REMOTE_WAKEUP,       /* device */
    TEST_MODE,                  /* device */
};

/*
 * Device status bits.
 */
enum device_status
{
    DEVSTATUS_NONE,
    DEVSTATUS_SELFPOWERED = 1,
    DEVSTATUS_REMOTEWAKEUP = 2,
};

/*
 * Interface status bits.
 */
enum interface_status
{
    IFSTATUS_NONE,
};

/*
 * Endpoint status bits.
 */
enum endpoint_status
{
    EPSTATUS_NONE,
    EPSTATUS_HALT = 1,
};


/*
 * Values for bRequest
 */

/*
 * Standard USB 2.0 requests
 */
enum request_type
{
    GET_STATUS,
    CLEAR_FEATURE,
    /* ... */
    SET_FEATURE = 3,
    /* ... */
    SET_ADDRESS = 5,
    GET_DESCRIPTOR,
    SET_DESCRIPTOR,
    GET_CONFIGURATION,
    SET_CONFIGURATION,
    GET_INTERFACE,
    SET_INTERFACE,
    SYNCH_FRAME,
};

/*
 * CDC/PSTN class specific requests (partial list).
 */
enum cdc_request_type
{
    SEND_ENCAPSULATED_COMMAND = 0x00,
    GET_ENCAPSULATED_RESPONSE,
    SET_COMM_FEATURE,
    GET_COMM_FEATURE,
    CLEAR_COMM_FEATURE,

    SET_AUX_LINE_STATE = 0x10,
    SET_HOOK_STATE,
    PULSE_SETUP,
    SEND_PULSE,
    SET_PULSE_TIME,
    RING_AUX_JACK,

    SET_LINE_CODING = 0x20,
    GET_LINE_CODING,
    SET_CONTROL_LINE_STATE,
    SEND_BREAK,

    SET_RINGER_PARMS = 0x30,
    GET_RINGER_PARMS,
    SET_OPERATION_PARMS,
    GET_OPERATION_PARMS,
    SET_LINE_PARMS,
    GET_LINE_PARMS,
    DIAL_DIGITS,
    SET_UNIT_PARAMETER,
    GET_UNIT_PARAMETER,
    CLEAR_UNIT_PARAMETER,
    GET_PROFILE,
};

/*
 * Values for bDescriptorType in each descriptor.
 */
enum descriptor_type
{
    DEVICE_DESC = 1,
    CONFIGURATION_DESC,
    STRING_DESC,
    INTERFACE_DESC,
    ENDPOINT_DESC,
    DEVICE_QUALIFIER_DESC,
    OTHER_SPEED_CONFIGURATION_DESC,
    INTERFACE_POWER_DESC,
    /* CDC */
    CS_INTERFACE = 0x24,
};

/*
 * Values for bDescriptorSubtype in CDC functional descriptors.
 */
enum functional_descriptor_subtype
{
    HEADER_FUNCTIONAL = 0x00,
    CALL_MANAGEMENT,
    ABSTRACT_CONTROL_MANAGEMENT,
    UNION_FUNCTIONAL = 0x06,
};

/*
 * Bits for bmAttributes in standard configuration descriptor.
 */
enum configuration_descriptor_attributes
{
    /* Device supports remote wakeup. */
    REMOTE_WAKEUP = 0x20,
    /* Device is self-powered */
    SELF_POWERED = 0x40,
    /* Reserved: always set */
    RESERVED_ALWAYS_SET = 0x80,
};

/*
 * Bits for bmAttributes in endpoint descriptor.  Only bits 0 and 1
 * are mentioned, for isochronous endpoints, bits 2 through 5 contain
 * additional information.
 */
enum endpoint_descriptor_attributes
{
    CONTROL_XFER,
    ISOCHRONOUS_XFER,
    BULK_XFER,
    INTERRUPT_XFER,
};

/*
 * Bits for bmCapabilities within the call management functional
 * descriptor.
 */
enum call_management_capabilities
{
    /*
     * Device can send/receive call management information over a Data
     * Class interface.
     */
    CM_DATA_MANAGED = 0x01,
    /* Device handles call management itself. */
    CM_MANAGES_CALLS = 0x02,
};

/*
 * Bits for bmCapabilities within the abstract control management
 * functional descriptor.
 */
enum abstract_control_management_capabilities
{
    /*
     * Device supports the request combination of Set_Comm_Feature,
     * Clear_Comm_Feature, and Get_Comm_Feature.
     */
    ACM_COMM_FEATURE_SET = 0x01,
    /*
     * Device supports the request combination of Set_Line_Coding,
     * Set_Control_Line_State, Get_Line_Coding, and the notification
     * Serial_State.
     */
    ACM_LINE_CODING_SET = 0x02,
    /*
     * Device supports the request Send_Break
     */
    ACM_SEND_BREAK = 0x04,
    /*
     * Device supports the notification Network_Connection.
     */
    ACM_NETWORK_CONNECTION = 0x08,
};

/*
 * Bits for control line state in SET_CONTROL_LINE_STATE PSTN request.
 */
enum control_line_state
{
    CTRL_STATE_NONE = 0,
    CTRL_STATE_DTE_PRESENT = 1, /* corresponds to DTR */
    CTRL_STATE_CARRIE = 2,      /* corresponds to RTS for half-duplex
                                 * modems */
};

/*
 * PSTN-specific notification values.
 */
enum pstn_notification
{
    NETWORK_CONNECTION_NOTIFICATION = 0,
    RESPONSE_AVAILABLE_NOTIFICATION = 1,
    AUX_JACK_HOOK_STATE_NOTIFICATION = 8,
    RING_DETECT_NOTIFICATION = 9,
    SERIAL_STATE_NOTIFICATION = 0x20,
    CALL_STATE_CHANGE_NOTIFICATION = 0x28,
    LINE_STATE_CHANGE_NOTIFICATION = 0x23,
};

/*
 * PSTN SERIAL_STATE notification bit values.
 */
enum serial_state_notification_bits
{
    bOverRun = 0x40,
    bParity = 0x20,
    bFraming = 0x10,
    bRingSignal = 0x08,
    bBreak = 0x04,
    bTxCarrier = 0x02,
    bRxCarrier = 0x01,
};

