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

/* === includes ============================================================ */
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "board.h"
#include "ioutil.h"
#include "hif_usb.h"

#define USB_DEBUG 0

#if USB_DEBUG
#  define UART_BAUD 9600ul
#endif

#if HIF_TYPE == HIF_AT90USB

/* === macros ============================================ */

#if !defined(USB_VID) || !defined(USB_PID)
#  error "No USB VID/PID pair defined, check current board_xxx.h"
#endif

#if !defined(USB_VENDOR_NAME)
#  error "No USB vendor name defined, check current board_xxx.h"
#endif

#if !defined(USB_PRODUCT_NAME)
#  error "No USB product name defined, check current board_xxx.h"
#endif

#if !defined(USB_BCD_RELEASE)
#  error "No USB BCD release defined, check current board_xxx.h"
#endif

#if F_CPU == 8000000UL
#  define PLLSETUP _BV(PLLP1) | _BV(PLLP0) | _BV(PLLE)
#elif F_CPU == 4000000UL
#  if defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB647__)
#    define PLLSETUP _BV(PLLP2) | _BV(PLLP1) | _BV(PLLE)
#  else /* AT90USB1286/1287 */
#    define PLLSETUP _BV(PLLP2) | _BV(PLLP0) | _BV(PLLE)
#  endif
#else
#  error "Illegal CPU frequency for USB PLL setup"
#endif

#define RXBUF_SIZE (sizeof(hif_rxbuf))
#define RXBUF_MASK (RXBUF_SIZE-1)

#define CTRL_EP_SIZE 64         /* this is *not* easily customizable */

#if USB_DEBUG
#  define DPRINTF(fmt, ...) printf_P(PSTR(fmt), __VA_ARGS__)
#  define DPRINT(fmt) printf_P(PSTR(fmt))
#else
#  define DPRINTF(fmt, ...)
#  define DPRINT(fmt)
#endif

#if defined(LED_ACTIVITY)
#  define LED_FLASH_TIME 100    /* ms */
#endif

/* === data types ======================================== */

/*
 * Endpoint size in UECFG1X[EPSIZE2:0]
 */
enum epsize
{
    EP8 = 0,
    EP16,
    EP32,
    EP64,
    EP128,                      /* only EP 1 */
    EP256                       /* only EP 1 */
};

/*
 * Endpoint type in UECFG0X[EPTYPE1:0]
 */
enum eptype
{
    EP_CONTROL,
    EP_ISOCHRONOUS,
    EP_BULK,
    EP_INTERRUPT
};

/*
 * Endpoint numbers to be used.  Order is important, and should not be
 * changed.  The control endpoint must be number 0 (in strict USB
 * terminology, it would be considered two endpoints, one IN and one
 * OUT EP, but the AT90USB1287 architecture hides that detail from the
 * user).  Memory allocation in the DPRAM must be performed in the
 * numerical order of EPs.
 */
enum ep_num
{
    CTRL_EP, TX_EP, RX_EP, INT_EP,
    EP_MAX
};

/* === prototypes ======================================== */

#if USB_DEBUG
static int uart_putchar(char, FILE *);
#endif

/* === globals =========================================== */

/* temporary uart buffer */
static uint8_t hif_rxbuf[128];
static volatile uint8_t rxhead;
static volatile uint8_t rxtail;

static volatile bool tx_data_pending;

#if defined(LED_ACTIVITY)
static volatile uint8_t led_active;
#endif

/*
 * Endpoint configuration for the AT90USB1287.  This will be used to
 * configure the respective EP config registers at the end of USB
 * reset (i.e. upon receiving an EORST interrupt).
 */
static struct ep_configuration
{
    uint8_t uecfg0x;
    uint8_t uecfg1x;
    uint8_t ueienx;
} const PROGMEM ep_conf_table[] =
{
    /* type: control, dir: OUT, size 64, single-banked */
    [CTRL_EP] =
    {
        .uecfg0x = (EP_CONTROL << EPTYPE0) /* | !_BV(EPDIR) */,
        .uecfg1x = (EP64 << EPSIZE0) /* | !_BV(EPBK0) */,
        .ueienx = _BV(RXSTPE),
    },

    /* type: bulk, dir: IN, size 64, double-banked */
    [TX_EP] =
    {
        .uecfg0x = (EP_BULK << EPTYPE0) | _BV(EPDIR),
        .uecfg1x = (EP64 << EPSIZE0) | _BV(EPBK0),
        .ueienx = 0,
    },

    /* type: bulk, dir: OUT, size 64, double-banked */
    [RX_EP] =
    {
        .uecfg0x = (EP_BULK << EPTYPE0) /* | !_BV(EPDIR) */,
        .uecfg1x = (EP64 << EPSIZE0) | _BV(EPBK0),
        .ueienx = _BV(RXOUTE),
    },

    /* type: interupt, dir: IN, size 16, single-banked */
    [INT_EP] =
    {
        .uecfg0x = (EP_INTERRUPT << EPTYPE0) | _BV(EPDIR),
        .uecfg1x = (EP16 << EPSIZE0) /* | !_BV(EPBK0) */,
        .ueienx = 0,
    }
};

/*
 * USB device descriptor
 */
static struct device_descriptor const PROGMEM device_desc =
{
    .bLength = sizeof(struct device_descriptor),
    .bDescriptorType = DEVICE_DESC,
    .bcdUSB = 0x0110,                /* USB 1.10 */
    .bDeviceClass = 2,               /* communication device (CDC) */
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = 64,
    .idVendor = USB_VID,
    .idProduct = USB_PID,
    .bcdDevice = USB_BCD_RELEASE,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 0,     /** \todo need a callback protocol for S/N */
    .bNumConfigurations = 1
};

/*
 * USB configuration descriptor
 */
static struct configuration_descriptor const PROGMEM configuration_desc =
{
    .conf_header =
    {
        .bLength = sizeof(struct configuration_descriptor_header),
        .bDescriptorType = CONFIGURATION_DESC,
        .wTotalLength = sizeof(struct configuration_descriptor),
        .bNumInterfaces = 2,
        .bConfigurationValue = 1,
        .iConfiguration = 0,
#if defined(USB_DEVICE_SELF_POWERED)
        .bmAttributes = RESERVED_ALWAYS_SET | SELF_POWERED,
#else  /* bus-powered */
        .bmAttributes = RESERVED_ALWAYS_SET,
#endif
        .bMaxPower = 50           /* 100 mA */
    },
    .ctrl_iface =
    {
        .iface =
        {
            .bLength = sizeof(struct interface_descriptor),
            .bDescriptorType = INTERFACE_DESC,
            .bInterfaceNumber = 0,
            .bAlternateSetting = 0,
            .bNumEndpoints = 1,
            .bInterfaceClass = 2  /* communication interface class */,
            .bInterfaceSubClass = 2 /* abstract control model */,
            .bInterfaceProtocol = 1 /* ITU-T V.250 (AT command set) */,
            .iInterface = 0
        },
        .header_functional =
        {
            .bFunctionLength = sizeof(struct header_functional_descriptor),
            .bDescriptorType = CS_INTERFACE,
            .bDescriptorSubtype = HEADER_FUNCTIONAL,
            .bcdCDC = 0x0110 /* USB 1.10 */
        },
        .union_functional =
        {
            .bFunctionLength = sizeof(struct union_functional_descriptor),
            .bDescriptorType = CS_INTERFACE,
            .bDescriptorSubtype = UNION_FUNCTIONAL,
            .bControlInterface = 0,
            .bSubordinateInterface0 = 1
        },
        .call_management =
        {
            .bFunctionLength =
            sizeof(struct call_management_functional_descriptor),
            .bDescriptorType = CS_INTERFACE,
            .bDescriptorSubtype = CALL_MANAGEMENT,
            .bmCapabilities = CM_DATA_MANAGED | CM_MANAGES_CALLS,
            .bDataInterface = 1
        },
        .abstract_control_management =
        {
            .bFunctionLength =
            sizeof(struct abstract_control_management_functional_descriptor),
            .bDescriptorType = CS_INTERFACE,
            .bDescriptorSubtype = ABSTRACT_CONTROL_MANAGEMENT,
            .bmCapabilities = ACM_LINE_CODING_SET
        },
        .endpoint0 =
        {
            .bLength = sizeof(struct endpoint_descriptor),
            .bDescriptorType = ENDPOINT_DESC,
            .bEndpointAddress = DIR_IN | INT_EP,
            .bmAttributes = INTERRUPT_XFER,
            .wMaxPacketSize = 16,
            /*
             * At least some Linux kernels cannot handle too small
             * bInterval values lest they return EIO upon trying to
             * open() the device, apparently due to an internal USB
             * bandwidth allocation starvation.
             */
            .bInterval = 100 /* ms */,
        }
    },
    .data_iface =
    {
        .iface =
        {
            .bLength = sizeof(struct interface_descriptor),
            .bDescriptorType = INTERFACE_DESC,
            .bInterfaceNumber = 1,
            .bAlternateSetting = 0,
            .bNumEndpoints = 2,
            .bInterfaceClass = 0x0A /* data interface class */,
            .bInterfaceSubClass = 0x00 /* unused */,
            .bInterfaceProtocol = 0x01 /* unused */,
            .iInterface = 0
        },
        .endpoint0 =
        {
            .bLength = sizeof(struct endpoint_descriptor),
            .bDescriptorType = ENDPOINT_DESC,
            .bEndpointAddress = DIR_IN | TX_EP,
            .bmAttributes = BULK_XFER,
            .wMaxPacketSize = 64,
            .bInterval = 0,
        },
        .endpoint1 =
        {
            .bLength = sizeof(struct endpoint_descriptor),
            .bDescriptorType = ENDPOINT_DESC,
            .bEndpointAddress = DIR_OUT | RX_EP,
            .bmAttributes = BULK_XFER,
            .wMaxPacketSize = 64,
            .bInterval = 0,
        }
    }
};

/*
 * USB String descriptors
 */

static struct string_descriptor_langs const PROGMEM string_descriptor_langs =
{
    .bLength = sizeof(struct string_descriptor_langs),
    .bDescriptorType = STRING_DESC,
    .wLANGID =
    {
        [0] = 0x0409            /* US English */
    }
};

static struct string_descriptor const PROGMEM vendor_name =
{
    .bLength = sizeof(struct string_descriptor) +
        sizeof(USB_VENDOR_NAME) - sizeof(wchar_t),
    .bDescriptorType = STRING_DESC,
    .bString = USB_VENDOR_NAME
};

static struct string_descriptor const PROGMEM product_name =
{
    .bLength = sizeof(struct string_descriptor) +
        sizeof(USB_PRODUCT_NAME) - sizeof(wchar_t),
    .bDescriptorType = STRING_DESC,
    .bString = USB_PRODUCT_NAME
};

static volatile uint8_t configuration_no;
static volatile bool attached_to_usb; /* whether we are attached to a USB or not */

/*
 * PSTN features to emulate standard RS-232 compatible "line" coding.
 * This can be set and queried by the host, but is otherwise ignored
 * by the emulation.
 */
static struct line_coding lcoding =
{
    .dwDTERate = 9600,          /* 9600 Bd */
    .bCharFormat = 0,           /* 1 stop bit */
    .bParityType = 0,           /* no parity */
    .bDataBits = 8              /* 8 data bits */
};

static uint8_t line_state;      /* DTR state */

#if USB_DEBUG
static FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, 0, _FDEV_SETUP_WRITE);
#endif

/* === functions ========================================= */

#if USB_DEBUG
static void uart_init(void)
{
  UBRR1L = (F_CPU / (16UL * UART_BAUD)) - 1;
  UCSR1B = _BV(TXEN1) | _BV(RXEN1);
}

static int uart_putchar(char c, FILE *f)
{
  if (c == '\n')
    uart_putchar('\r', f);
  loop_until_bit_is_set(UCSR1A, UDRE1);
  UDR1 = c;

  return 0;
}
#endif  /* USB_DEBUG */

void hif_init(const uint32_t baudrate)
{
#if USB_DEBUG
    stdout = &uart_str;
    uart_init();
    DELAY_US(1000);
#endif  /* USB_DEBUG */

    DPRINT("Hello, USB world!\n");

    /*
     * Enable the USB macro, the VCC pad (which is misnamed "OTG
     * pad"), and the Vbus transition interrupt.
     */
    USBCON = _BV(USBE) | _BV(OTGPADE) | _BV(VBUSTE);

    if (USBSTA & _BV(VBUS))
    {
        /* We are already connected to the USB. */

        /*
         * Start USB clock PLL.
         */
        PLLCSR = PLLSETUP;
        while ((PLLCSR & _BV(PLOCK)) == 0)
        {
            /* wait for PLL lock */
        }
        /*
         * Enable the end-of-reset and suspend function interrupts.
         * Activate the voltage regulator.  Unfreeze the clock, and
         * attach.
         */
        UDIEN = _BV(EORSTE) | _BV(SUSPE);
        UHWCON |= _BV(UVREGE);
        USBCON &= ~_BV(FRZCLK);
        UDCON &= ~_BV(DETACH);

        attached_to_usb = true;
    }
    else
    {
        /*
         * We are not connected to the USB: wait for the bus power
         * being detected (which triggers a VBUSTI interrupt).
         */
        attached_to_usb = false;
    }

    rxhead = rxtail = 0;
}

void hif_puts_p(const char *progmem_s)
{
    register char c;
    while ( (c = pgm_read_byte(progmem_s++)) )
    {
        hif_putc(c);
    }
}


void hif_puts(const char *s )
{
    uint8_t len, tmp, *p;

    len = strlen(s);
    p = (uint8_t *)s;
    tmp = 0;
    do
    {

        tmp = hif_put_blk(p, len);
        p += tmp;
        len -=tmp;
    }
    while(len > 0);
}

/*
 * Transmit buffer
 */
uint8_t hif_put_blk(unsigned char *data, uint8_t size)
{
    bool data_pending;
    uint8_t s = size;

    if (s == 0)
        return 0;

    while (configuration_no == 0)
    {
        return 0;
    }

    UENUM = TX_EP;
    while (s > 0)
    {
        data_pending = true;

        /* If the current bank is still busy, wait. */
        while ((UEINTX & _BV(RWAL)) == 0)
            { /* wait */ }
        while (s > 0 && (UEINTX & _BV(RWAL)) != 0)
        {
            UEDATX = (uint8_t)*data++;
            s--;
        }
        /* If the current bank is full, start transmission. */
        if ((UEINTX & _BV(RWAL)) == 0)
        {
            UEINTX &= ~_BV(TXINI);
            UEINTX &= ~_BV(FIFOCON);
            data_pending = false;
        }
    }

    tx_data_pending = data_pending;

    return size;
}


int hif_putc(int c)
{
    while (configuration_no == 0)
        { /* wait until configured by host */ }

    UENUM = TX_EP;
    UEDATX = (uint8_t)c;
    if ((UEINTX & _BV(RWAL)) == 0)
    {
        /* transmit this bank, switch to next one */
        UEINTX &= ~_BV(TXINI);
        UEINTX &= ~_BV(FIFOCON);
        tx_data_pending = false;
    }
    else
    {
        tx_data_pending = true;
    }
	return c;
}


int hif_getc(void)
{
int ret = EOF;
    if (rxtail != rxhead)
    {
        ret = hif_rxbuf[rxtail];
        rxtail = ((rxtail + 1) & RXBUF_MASK);
    }else{
        ret = EOF;
    }
    return ret;
}


uint8_t hif_get_blk(unsigned char *data, uint8_t max_size)
{

uint8_t used_size, retsize;
uint8_t newtail, currtail, currhead, b1=0, b2=0;

    uint8_t __sreg = SREG; cli();
    /* compute space in uart_tx buffer */
    currtail = rxtail;
    currhead = rxhead;
    newtail = ((currhead - 1) & RXBUF_MASK);

    if (currtail == currhead)
    {
        /* no bytes in circular buffer */
        b1 = 0;
        b2 = 0;
    }
    else if (currtail > currhead)
    {
        b1 = ((RXBUF_SIZE-currtail) & RXBUF_MASK);
        b2 = currhead;
    }
    else if (currtail < currhead)
    {
        b1 = ((currhead - currtail) & RXBUF_MASK);
        b2 = 0;
    }
    used_size = b1 + b2;
    retsize =  (max_size > used_size) ? used_size : max_size;

    /* handle block 1*/
    b1 = (b1 > max_size) ? max_size : b1;
    if (b1 > 0)
    {
        memcpy(data, (void*)&hif_rxbuf[currtail],  b1);
        data += b1;
        rxtail = ((currtail + b1) & RXBUF_MASK);
        max_size = ((max_size - b1)  & RXBUF_MASK);
    }

    /* handle block 2*/
    b2 = (max_size > b2) ?  b2 : max_size;
    if(b2 > 0)
    {
        memcpy(data, (void*)&hif_rxbuf[0], b2);
        rxtail = (b2);
    }

    SREG = __sreg;

    return retsize;
}

#if defined(LED_ACTIVITY)
static void led_activate(void)
{
    LED_SET(0);
    led_active = LED_FLASH_TIME;
}
#endif  /* LED_ACTIVITY */


static bool configure_endpoint(uint8_t epnum)
{
    struct ep_configuration p;

    memcpy_P(&p, &ep_conf_table[epnum], sizeof(struct ep_configuration));

    UENUM = epnum;
    UECONX = _BV(EPEN);
    UECFG0X = p.uecfg0x;
    UECFG1X = p.uecfg1x | _BV(ALLOC);
    UEIENX = p.ueienx;

    return (UESTA0X & _BV(CFGOK)) != 0;
}

static bool configure_endpoints(void)
{
    uint8_t ep;
    bool success;

    for (ep = CTRL_EP, success = true; ep < EP_MAX; ep++)
    {
        success = success && configure_endpoint(ep);
    }

    return success;
}

static void set_endpoint_halt_feature(uint8_t ep)
{
    UENUM = ep;

    UECONX &= ~_BV(EPEN);

    /* Send a zero-length packet in response. */
    UEINTX &= ~_BV(TXINI);
}

static void clear_endpoint_halt_feature(uint8_t ep)
{
    UENUM = ep;

    UECONX |= _BV(EPEN);

    /* Send a zero-length packet in response. */
    UEINTX &= ~_BV(TXINI);
}

static void get_configuration(void)
{
    /* data IN phase: send configuration value */
    UEDATX = configuration_no;
    UEINTX &= ~_BV(TXINI);

    /* status OUT phase: receive (and ACK) ZLP */
    while ((UEINTX & _BV(RXOUTI)) == 0)
        { /* wait */ }
    UEINTX &= ~_BV(RXOUTI);
}

static void set_configuration(uint8_t conf)
{
    configuration_no = conf;
#if defined(LED_USB_CONFIGURED)
    if (conf)
    {
    LED_SET(3);
    }
    else
    {
    LED_CLR(3);
    }
#endif  /* LED_USB_CONFIGURED */

    /* Send a zero-length packet in response. */
    UEINTX &= ~_BV(TXINI);
}

static void get_flash_descriptor(const char *desc, uint8_t len,
                                 uint16_t maxlen)
{
    enum
    {
        ZLP_UNDECIDED,
        ZLP_YES,
        ZLP_NO
    } __attribute__((packed)) need_zlp;
    uint8_t count;

    if (len == maxlen)
    {
        /*
         * If the requested size exactly matches the amount of data to
         * transfer, no zero-length packet (ZLP) is required.
         */
        need_zlp = ZLP_NO;
    }
    else
    {
        /*
         * Otherwise, a ZLP might be required in case the last transferred
         * packet is a full-sized packet.
         */
        need_zlp = ZLP_UNDECIDED;
    }
    if (len > maxlen)
    {
        /*
         * Only a partial descriptor has been requested.
         */
        len = maxlen;
    }
    DPRINTF("get_flash_descriptor() -> %d bytes\n", len);

    /*
     * As descriptors returned (unlike all other SETUP responses)
     * could possibly span multiple USB packets, we have to keep
     * track of packet boundaries.  The FIFOCON and RWAL bits are
     * not usable on control endpoints, so we have to manually
     * track the amount of data transmitted.
     */
    while (len != 0)
    {
        count = 0;

        while ((UEINTX & _BV(TXINI)) == 0)
            { /* wait */ }

        while (len != 0 && count < CTRL_EP_SIZE)
        {
            char c = pgm_read_byte(desc);
            desc++;
            UEDATX = (uint8_t)c;

            len--;
            count++;
        }
        UEINTX &= ~_BV(TXINI);
        /*
         * If this packet was exactly the endpoint size, a zero-length
         * packet might be required.
         */
        if (need_zlp != ZLP_NO)
        {
            if (count == CTRL_EP_SIZE)
            {
                need_zlp = ZLP_YES;
            }
            else
            {
                need_zlp = ZLP_UNDECIDED;
            }
        }
    }
    if ((UEINTX & _BV(RXOUTI)) != 0)
    {
        /* transfer aborted */
        UEINTX &= ~_BV(RXOUTI);
        return;
    }
    if (need_zlp == ZLP_YES)
    {
        DPRINT("sending ZLP\n");
        while ((UEINTX & _BV(TXINI)) == 0)
            { /* wait */ }
        UEINTX &= ~_BV(TXINI);
    }
    while ((UEINTX & _BV(RXOUTI)) == 0)
        { /* wait */ }
    UEINTX &= ~_BV(RXOUTI);
}

static void set_address(uint8_t addr)
{
    /* Set device address but do not activate yet (ADDEN = 0). */
    UDADDR = addr;

    /* send zero-length packet in response */
    UEINTX &= ~_BV(TXINI);

    while ((UEINTX & _BV(TXINI)) == 0)
        { /* wait */ }

    /* Setup handled, activate new address. */
    UDADDR |= _BV(ADDEN);
}

static void get_status(uint16_t status)
{
    /* data IN phase: return status */
    UEDATX = status & 0xFF;
    UEDATX = status >> 8;
    UEINTX &= ~_BV(TXINI);

    /* status phase: wait for ZLP and ACK it */
    while ((UEINTX & _BV(RXOUTI)) == 0)
        { /* wait */ }
    UEINTX &= ~_BV(RXOUTI);
}

static void set_interface(uint8_t iface)
{
    /* No data phase */

    /* status phase: zero-length IN packet  */
    UEINTX &= ~_BV(TXINI);
}

static void set_line_coding(void)
{
    uint8_t *bp;
    uint8_t i;

    /* data phase: data OUT phase, wait for completetion */
    while ((UEINTX & _BV(RXOUTI)) == 0)
        { /* wait */ }

    for (bp = (uint8_t *)&lcoding, i = 0;
         i < sizeof(struct line_coding);
         i++)
    {
        *bp++ = UEDATX;
    }
    UEINTX &= ~_BV(RXOUTI);

    /* status phase: send ZLP */
    UEINTX &= ~_BV(TXINI);
    while ((UEINTX & _BV(TXINI)) == 0)
        { /* wait */ }

    /*
     * We insist on even parity, one stop bit, and 8 data bits, so
     * effectively, only the line bit rate can be changed to an
     * arbitrary value.
     */
    lcoding.bParityType = 0;
    lcoding.bCharFormat = 0;
    lcoding.bDataBits = 8;
}

static void get_line_coding(void)
{
    uint8_t *bp;
    uint8_t i;

    /* data phase: data IN phase */
    for (bp = (uint8_t *)&lcoding, i = 0;
         i < sizeof(struct line_coding);
         i++)
    {
        UEDATX = *bp++;
    }
    UEINTX &= ~_BV(TXINI);

    /* status phase: receive (and ACK) ZLP OUT packet */
    while ((UEINTX & _BV(RXOUTI)) == 0)
        { /* wait */ }
    UEINTX &= ~_BV(RXOUTI);
}

static void send_serial_state_notification(void)
{
    UENUM = INT_EP;
    if (((UEINTX & _BV(RWAL)) == 0) ||
        ((UEINTX & _BV(TXINI)) == 0))
    {
        /*
         * Interrupt endpoint not ready, punt.
         */
        return;
    }

    UEDATX = DIR_IN | RQTYPE_CLASS | RQRCPT_INTERFACE;
    UEDATX = SERIAL_STATE_NOTIFICATION;
    UEDATX = 0x00;          /* wValue = 0 */
    UEDATX = 0x00;
    UEDATX = 0x00;          /* wIndex = Interface */
    UEDATX = 0x00;
    UEDATX = 0x02;          /* wLength = 2 */
    UEDATX = 0x00;
    if (line_state)
    {
        UEDATX = bTxCarrier | bRxCarrier;
    }
    else
    {
        UEDATX = 0;
    }
    UEDATX = 0x00;              /* all bits in high byte are reserved */

    UEINTX &= ~_BV(TXINI);
    UEINTX &= ~_BV(FIFOCON);
}

static void set_control_line_state(uint16_t state)
{
    /* No data phase. */

    /* status phase: IN, send zero-length packet in response */
    UEINTX &= ~_BV(TXINI);
    while ((UEINTX & _BV(TXINI)) == 0)
        { /* wait */ }

    /*
     * We are only interested in the DTR part of the state just
     * received.
     */
    state &= CTRL_STATE_DTE_PRESENT;

    if (line_state != state)
        {
            line_state = state;
            /*
             * Change in DTR state, transmit a SERIAL_STATE
             * notification on the interrupt endpoint.  This emulates
             * a "null modem" (DSR and DCD on DCE follow DTE's DTR).
             */
            send_serial_state_notification();
        }
}

static inline uint16_t read16(void)
{
    uint16_t word;

    word = UEDATX;
    word |= (uint16_t)UEDATX << 8;

    return word;
}

/*
 * Process a control message..
 */
static bool process_setup_message(void)
{
    /*
     * Standard fields present in all control packets.
     */
    uint8_t bmRequestType = UEDATX;
    uint8_t bRequest = UEDATX;
    uint16_t wValue = read16();
    uint16_t wIndex = read16();
    uint16_t wLength = read16();

    DPRINTF("Setup: bmRequestType = 0x%02x, bRequest = 0x%02x,"
            " wValue = 0x%04x, wIndex = 0x%04x, wLength = 0x%04x\n",
            bmRequestType, bRequest, wValue, wIndex, wLength);

    switch (bRequest)
    {
        /*
         * Standard USB 2.0 requests.
         */
    case CLEAR_FEATURE:
        if (bmRequestType == (DIR_OUT | RQTYPE_STANDARD | RQRCPT_ENDPOINT) &&
            wValue == ENDPOINT_HALT &&
            wIndex <= INT_EP)
        {
            UEINTX &= ~_BV(RXSTPI);
            clear_endpoint_halt_feature((uint8_t)wIndex);
            break;
        }
        return false;

    case GET_CONFIGURATION:
        if (bmRequestType == (DIR_IN | RQTYPE_STANDARD | RQRCPT_DEVICE))
        {
            UEINTX &= ~_BV(RXSTPI);
            get_configuration();
            break;
        }
        return false;

    case GET_DESCRIPTOR:
        if ((DIR_IN | RQTYPE_STANDARD | RQRCPT_DEVICE))
        {
            /* descriptor type */
            switch (wValue >> 8)
            {
            case DEVICE_DESC:
                if (wIndex == 0)
                {
                    UEINTX &= ~_BV(RXSTPI);
                    get_flash_descriptor((const char *)&device_desc,
                                         sizeof(device_desc), wLength);
                    break;
                }
                return false;

            case CONFIGURATION_DESC:
                if (wIndex == 0)
                {
                    UEINTX &= ~_BV(RXSTPI);
                    get_flash_descriptor((const char *)&configuration_desc,
                                         sizeof(configuration_desc), wLength);
                    break;
                }
                return false;

            case STRING_DESC:
                /* descriptor index */
                switch (wValue & 0xFF)
                {
                    uint8_t len;

                case 0:
                    UEINTX &= ~_BV(RXSTPI);

                    /* .bLength of actual descriptor */
                    len = pgm_read_byte(&string_descriptor_langs);
                    get_flash_descriptor((const char *)&string_descriptor_langs,
                                         len, wLength);
                    break;

                case 1:
                    UEINTX &= ~_BV(RXSTPI);

                    /* .bLength of actual descriptor */
                    len = pgm_read_byte(&vendor_name);
                    get_flash_descriptor((const char *)&vendor_name,
                                         len, wLength);
                    break;

                case 2:
                    UEINTX &= ~_BV(RXSTPI);

                    /* .bLength of actual descriptor */
                    len = pgm_read_byte(&product_name);
                    get_flash_descriptor((const char *)&product_name,
                                         len, wLength);
                    break;

                default:
                    /* unknown string descriptor index */
                    return false;
                }
                break;

            default:
                /* unknown descriptor type */
                return false;
            }
        }
        return false;

    case GET_STATUS:
        switch (bmRequestType)
        {
        case DIR_IN | RQTYPE_STANDARD | RQRCPT_DEVICE:
            if (wIndex == 0)
            {
                UEINTX &= ~_BV(RXSTPI);
#if defined(USB_DEVICE_SELF_POWERED)
                get_status(DEVSTATUS_SELFPOWERED);
#else
                get_status(DEVSTATUS_NONE);
#endif
                break;
            }
            return false;

        case DIR_IN | RQTYPE_STANDARD | RQRCPT_INTERFACE:
            if (wIndex == 0)
            {
                UEINTX &= ~_BV(RXSTPI);
                get_status(IFSTATUS_NONE);
                break;
            }
            return false;

        case DIR_IN | RQTYPE_STANDARD | RQRCPT_ENDPOINT:
            if (wIndex < EP_MAX)
            {
                UEINTX &= ~_BV(RXSTPI);
                UENUM = (uint8_t)wIndex;
                if ((UECONX & _BV(EPEN)) == 0)
                {
                    /* endpoint halted */
                    get_status(EPSTATUS_HALT);
                }
                else
                {
                    get_status(EPSTATUS_NONE);
                }
                break;
            }
            return false;

        default:
            /* unknown GET_STATUS request type */
            return false;
        }
        return false;

    case SET_ADDRESS:
        if (bmRequestType == (DIR_OUT | RQTYPE_STANDARD | RQRCPT_DEVICE) &&
            wValue <= 127)
        {
            UEINTX &= ~_BV(RXSTPI);
            set_address(wValue);
            break;
        }
        return false;

    case SET_CONFIGURATION:
        if (bmRequestType == (DIR_OUT | RQTYPE_STANDARD | RQRCPT_DEVICE) &&
            (wValue == 0 /* return to addressed state */ ||
             wValue == 1 /* activate valid configuration */))
        {
            UEINTX &= ~_BV(RXSTPI);
            set_configuration((uint8_t)wValue);
            break;
        }
        return false;

    case SET_FEATURE:
        if (bmRequestType == (DIR_OUT | RQTYPE_STANDARD | RQRCPT_ENDPOINT) &&
            wValue == ENDPOINT_HALT &&
            wIndex <= INT_EP)
        {
            UEINTX &= ~_BV(RXSTPI);
            set_endpoint_halt_feature((uint8_t)wIndex);
            break;
        }
        return false;

    case SET_INTERFACE:
        if (bmRequestType == (DIR_OUT | RQTYPE_STANDARD | RQRCPT_INTERFACE) &&
            (wIndex == 0 || wIndex == 1) &&
            wValue == 0)
        {
            UEINTX &= ~_BV(RXSTPI);
            set_interface(wValue);
            break;
        }
        return false;

        /*
         * CDC/PSTN class-specific requests.
         */
    case SET_LINE_CODING:
        if (bmRequestType == (DIR_OUT | RQTYPE_CLASS | RQRCPT_INTERFACE) &&
            wLength == sizeof(struct line_coding))
        {
            UEINTX &= ~_BV(RXSTPI);
            set_line_coding();
            break;
        }
        return false;

    case GET_LINE_CODING:
        if (bmRequestType == (DIR_IN | RQTYPE_CLASS | RQRCPT_INTERFACE) &&
            wLength == sizeof(struct line_coding))
        {
            UEINTX &= ~_BV(RXSTPI);
            get_line_coding();
            break;
        }
        return false;

    case SET_CONTROL_LINE_STATE:
        if (bmRequestType == (DIR_OUT | RQTYPE_CLASS | RQRCPT_INTERFACE))
        {
            UEINTX &= ~_BV(RXSTPI);
            set_control_line_state(wValue);
            break;
        }
        return false;

    default:
        /* unhandled setup request */
        return false;
    }
    return true;
}


#if defined(DOXYGEN)
void USB_GEN_vect();
#else
static volatile uint8_t rxovf;
ISR(USB_GEN_vect)
#endif
{
    if ((UDINT & _BV(SOFI)) && (UDIEN & _BV(SOFE)))
    {
#if defined(LED_ACTIVITY)
    uint8_t led_timer_cached = led_active;
    if (led_timer_cached)
    {
        if (--led_timer_cached == 0)
        {
        LED_CLR(0);
        }
        led_active = led_timer_cached;
    }
#endif  /* LED_ACTIVITY */
        /* start-of-frame interrupt, triggers each 1 ms while attached */
        UDINT &= ~_BV(SOFI);
        if (tx_data_pending)
        {
            UENUM = TX_EP;
            /* transmit this bank, switch to next one */
            UEINTX &= ~_BV(TXINI);
            UEINTX &= ~_BV(FIFOCON);
            tx_data_pending = false;

            led_activate();
        }
    }
    if (USBINT & _BV(VBUSTI))
    {
        USBINT &= ~_BV(VBUSTI);
        if (USBSTA & _BV(VBUS))
        {
            /* handle powerup */
            attached_to_usb = true;

            /*
             * Start PLL, and wait until it locks.
             */
            PLLCSR = PLLSETUP;
            while ((PLLCSR & _BV(PLOCK)) == 0)
            {
                /* wait for PLL lock */
            }

            UDIEN = _BV(EORSTE) | _BV(SUSPE) | _BV(SOFE);
            USBCON &= ~_BV(FRZCLK);
            UHWCON |= _BV(UVREGE);
            UDCON &= ~_BV(DETACH);
        }
        else
        {
            /* handle powerdown */
            attached_to_usb = false;
        configuration_no = 0;

#if defined(LED_USB_CONFIGURED)
        LED_CLR(3);
#endif  /* LED_USB_CONFIGURED */

            UDCON |= _BV(DETACH);
            UHWCON &= ~_BV(UVREGE);
            USBCON |= _BV(FRZCLK);
            UDIEN = 0;
            PLLCSR = 0;
            line_state = 0;
        }
    }
    if ((UDINT & _BV(EORSTI)) && (UDIEN & _BV(EORSTE)))
    {
        UDINT &= ~_BV(EORSTI);
        /* handle end-of-reset */

        (void)configure_endpoints();
    }
    if ((UDINT & _BV(SUSPI)) && (UDIEN & _BV(SUSPE)))
    {
        UDINT &= ~_BV(SUSPI);
        /* handle suspend */
        USBCON |= _BV(FRZCLK);
        UDIEN = _BV(WAKEUPE);
        PLLCSR = 0;
        line_state = 0;
    }
    if ((UDINT & _BV(WAKEUPI)) && (UDIEN & _BV(WAKEUPE)))
    {
        UDINT &= ~_BV(WAKEUPI);
        /* handle wakeup */
        PLLCSR = PLLSETUP;
        while ((PLLCSR & _BV(PLOCK)) == 0)
        {
            /* wait for PLL lock */
        }

        UDIEN = _BV(EORSTE) | _BV(SUSPE) | _BV(SOFE);
        USBCON &= ~_BV(FRZCLK);
    }
}

#if defined(DOXYGEN)
void USB_COM_vect();
#else
ISR(USB_COM_vect)
#endif
{
    bool ctrl_has_data = false, out_has_data = false;
    uint8_t ueienx_ctrl, ueienx_data;

    if (UEINT & _BV(CTRL_EP))
    {
        /* Control endpoint has data. */
        ctrl_has_data = true;
    }
    if (UEINT & _BV(RX_EP))
    {
        /* Out (RX) endpoint has data. */
        out_has_data = true;
    }

    if (!ctrl_has_data && !out_has_data)
    {
        return;
    }

    /*
     * Save current interrupt enable status of the USB endpoints,
     * disable all of them, and allow CPU interrupts while processing
     * all the EP interrupts (which can take quite some time).
     */
    UENUM = CTRL_EP;
    ueienx_ctrl = UEIENX;
    UEIENX = 0;
    UENUM = RX_EP;
    ueienx_data = UEIENX;
    UEIENX = 0;
    sei();

    if (ctrl_has_data)
    {
        UENUM = CTRL_EP;

        if (UEINTX & _BV(RXSTPI))
        {
            if (!process_setup_message())
            {
                /*
                 * SETUP request not understood: STALL the current
                 * setup message, either during the data or status
                 * phase.  Automatically gets cleared when receiving
                 * the next SETUP packet.
                 */
                UECONX |= _BV(STALLRQ);
                UEINTX &= ~_BV(RXSTPI);
            }
        }
    }

    /*
     * Only handle OUT data if we have been enumerated already.
     */
    if (out_has_data && configuration_no != 0)
    {
        UENUM = RX_EP;

        while (UEINTX & _BV(RXOUTI))
        {
            UEINTX &= ~_BV(RXOUTI);

            while (UEINTX & _BV(RWAL))
            {
                /** todo handle other uart errors (usr register)*/
                if (rxhead == rxtail)
                {
                    rxovf++;
                }

                hif_rxbuf[rxhead] = UEDATX;
                rxhead = ((rxhead + 1) & RXBUF_MASK);
            }
            UEINTX &= ~_BV(FIFOCON);
        }

        led_activate();
    }

    /*
     * Restore EP interrupt status before finishing up.
     */
    cli();
    UENUM = CTRL_EP;
    UEIENX = ueienx_ctrl;
    UENUM = RX_EP;
    UEIENX = ueienx_data;
}

#endif /* HIF_TYPE == HIF_AT90USB */
