#include <libopencm3/stm32/rcc.h>
#include <libopencmsis/core_cm3.h>
#include "st7789_stm32_spi.h"
#include "font_fixedsys_mono_24.h"
#include "usb_descriptor.h"
#include "common.h"
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/usb/usbd.h>
#include <string.h>


static void usb_set_config(usbd_device *usbd_dev, uint16_t wValue);
static void usb_data_received(usbd_device *usbd_dev, uint8_t ep);
static enum usbd_request_return_codes prepare_to_receive_stream(usbd_device *usbd_dev, 
																struct usb_setup_data *req, 
																uint8_t **buf, 
																uint16_t *len, 
																usbd_control_complete_callback *complete) ;

void usb_init();

// USB device instance
static usbd_device *usb_device;

// buffer for control requests
static uint8_t usbd_control_buffer[256];


/* Set STM32 to 72 MHz. */
static void clock_setup(void)
{
	rcc_clock_setup_in_hse_8mhz_out_72mhz();
    rcc_periph_clock_enable(RCC_GPIOC);
    rcc_periph_clock_enable(RCC_USB);
	// GPIOA, AFIO clocks are handled by st_init();
}


int main(void)
{
	/* Hardware config starts */
	clock_setup();
	systick_init();

	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_10_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
	gpio_clear(GPIOC, GPIO13);
	/* Hardware config ends */

	//----------------------------------------------------------
	st_init();
	//rotating display to potrait mode
	//st_rotate_display(1);

	// Filling the display with some color
	st_fill_screen(ST_COLOR_NAVY);

	st_draw_string_withbg(10, 200, "Tiny Monitor v1.0", ST_COLOR_CYAN, ST_COLOR_BLACK, &font_fixedsys_mono_24);
	st_draw_string(10, 10, "Connect USB...", 0xffff, &font_fixedsys_mono_24);

	// Init USB at the end so MCU can start sending data to display as soon as USB is present
	usb_init();

	while (1)
	{
		// Nothing to do here. Everything is handled in callbacks
	}
	
	return 0;
}


void usb_init()
{
    // reset USB peripheral
    rcc_periph_reset_pulse(RST_USB);

    // Pull USB D+ (A12) low for 80ms to trigger device reenumeration
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_10_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO12);
    gpio_clear(GPIOA, GPIO12);
    delay(80);

    usb_init_serial_num();

    // create USB device
    usb_device = usbd_init(&st_usbfs_v1_usb_driver, &usb_device_desc, usb_config_descs,
                           usb_desc_strings, sizeof(usb_desc_strings) / sizeof(usb_desc_strings[0]),
                           usbd_control_buffer, sizeof(usbd_control_buffer));

    // Set callback for config calls
    usbd_register_set_config_callback(usb_device, usb_set_config);
    //register_wcid_desc(usb_device);

	

    // Enable interrupt
    nvic_set_priority(NVIC_USB_LP_CAN_RX0_IRQ, 2 << 6);
    nvic_enable_irq(NVIC_USB_LP_CAN_RX0_IRQ);
}

// Called when the host connects to the device and selects a configuration
void usb_set_config(usbd_device *usbd_dev, __attribute__((unused)) uint16_t wValue)
{
    usbd_ep_setup(usbd_dev, EP_DATA_OUT, USB_ENDPOINT_ATTR_BULK, BULK_MAX_PACKET_SIZE, usb_data_received);
	usbd_ep_setup(usbd_dev, 0x82, USB_ENDPOINT_ATTR_ISOCHRONOUS, BULK_MAX_PACKET_SIZE, nullptr);


	usbd_register_control_callback(usb_device, 
		USB_REQ_TYPE_VENDOR | USB_REQ_TYPE_INTERFACE, 
		USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT, 
		prepare_to_receive_stream);

	st_draw_string(10, 40, "USB connected. Config is set...", 0xffff, &font_fixedsys_mono_24);
}

// Called when host sends control transfer. 
// If host sends wValue=0x88 and bRequest=0x33, we'll prepare to receive streams
static enum usbd_request_return_codes prepare_to_receive_stream(usbd_device *usbd_dev, struct usb_setup_data *req, uint8_t **buf, uint16_t *len, usbd_control_complete_callback *complete) 
{
	if (req->bRequest == 0x33 && req->wIndex == 0 && req->wValue == 0x88) 
	{
		st_draw_string(10, 100, "Connected to host software...", 0xffff, &font_fixedsys_mono_24);

		// after writing string, prepare the display for drawing
		// Once we set the address window, we can fill the screen continuously without calling it again
		st_set_address_window(0, 0, 239, 239);

		uint8_t ack_buff[1] = {0xAA}; //0xAA is the magic number that the python script is expecting
		// Send host 0xAA to let know that we're ready
		usbd_ep_write_packet(usbd_dev, 0x82, ack_buff, 1);

		return USBD_REQ_HANDLED;
	}
	return USBD_REQ_NEXT_CALLBACK;
}

// Called when data has been received
void usb_data_received(__attribute__((unused)) usbd_device *usbd_dev, __attribute__((unused)) uint8_t ep)
{
    // Retrieve USB data (has side effect of setting endpoint to VALID)
    uint8_t packet[BULK_MAX_PACKET_SIZE] __attribute__((aligned(4)));
    int len = usbd_ep_read_packet(usb_device, EP_DATA_OUT, packet, sizeof(packet));

	st_fill_color_array(packet, len);

	// GPIOC_ODR ^= GPIO13;

	//usbd_ep_nak_set(usbd_dev, EP_DATA_OUT, 1);
	//usbd_ep_nak_set(usbd_dev, EP_DATA_OUT, 0);
}



// USB interrupt handler
void usb_lp_can_rx0_isr()
{
    usbd_poll(usb_device);
	/* GPIOC_ODR ^= GPIO13; */
}