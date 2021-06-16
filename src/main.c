#include <libopencm3/stm32/rcc.h>
#include <libopencmsis/core_cm3.h>
#include "st7789_stm32_spi.h"
#include "font_microsoft_16.h"
#include "usb_descriptor.h"
#include "common.h"
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/usb/usbd.h>
#include <string.h>

// Minimum free space in circular buffer for requesting more packets
#define MIN_FREE_SPACE		(2 * BULK_MAX_PACKET_SIZE)
#define	ROW_LEN				480 /* 240 pixels x 2 byte = 480 bytes */

static void usb_set_config(usbd_device *usbd_dev, uint16_t wValue);
static void usb_data_received(usbd_device *usbd_dev, uint8_t ep);
static enum usbd_request_return_codes do_stuff(usbd_device *usbd_dev, struct usb_setup_data *req, uint8_t **buf, uint16_t *len, usbd_control_complete_callback *complete) ;
static void usb_update_nak();
void usb_init();

// USB device instance
static usbd_device *usb_device;

// buffer for control requests
static uint8_t usbd_control_buffer[256];

// main buffer
static uint8_t buff[480];


// indicates if the endpoint is forced to NAK to prevent receiving further data
static volatile uint8_t is_forced_nak = 0;


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
	/* Hardware config ends */

	//----------------------------------------------------------
	st_init();
	//rotating display to potrait mode
	//st_rotate_display(1);

	st_fill_screen(0x2341);
	st_set_address_window(0, 0, 239, 239);
	//st_draw_string(0, 0, "wallaaa", 0xffff, &font_microsoft_16);

	usb_init();

	while (1)
	{
		
	}
	//----------------------------------------------------------
	//rest of the user code below
	//while (1)
	//{
		//		if (data_received)
		//		{
		//			uint16_t pixel = 0;
		//			while(pixel < RX_BUFFER_SIZE)
		//			{
		//				uint16_t pixel_16bit = (uint16_t)buff_single_row[pixel] << 8 | (uint16_t)buff_single_row[pixel+1];
		//				//write_data_16bit(pixel_16bit);
		//				pixel += 2;
		//			}
		//			data_received = 0;
		//		}//if




		/*--- Blocking method (experimental) --*/
		//		uint8_t pixel_msb, pixel_lsb;
		//
		//		while ((USART1->SR & UART_FLAG_RXNE) == RESET);
		//		pixel_msb = USART1->DR;
		//		WRITE_8BIT(pixel_msb);
		//
		//		while ((USART1->SR & UART_FLAG_RXNE) == RESET);
		//		pixel_lsb = USART1->DR;
		//		WRITE_8BIT(pixel_lsb);


	//}//while

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
	// gpio_set(GPIOA, GPIO12);
	// delay(80);
	// gpio_clear(GPIOA, GPIO12);

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
    //register_wcid_desc(usbd_dev);
    usbd_ep_setup(usbd_dev, EP_DATA_OUT, USB_ENDPOINT_ATTR_BULK, BULK_MAX_PACKET_SIZE, usb_data_received);

    //buffer.reset();
    is_forced_nak = 0;

	usbd_register_control_callback(usb_device, USB_REQ_TYPE_VENDOR | USB_REQ_TYPE_INTERFACE, USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT, do_stuff);

	static uint16_t c = 0xffff;
	st_fill_rect_fast(60, 60, 30, 30, c);
	c = c == 0xffff ? 0 : 0xffff;
	st_set_address_window(0, 0, 239, 239);
}

static enum usbd_request_return_codes do_stuff(usbd_device *usbd_dev, struct usb_setup_data *req, uint8_t **buf, uint16_t *len, usbd_control_complete_callback *complete) 
{
	static uint16_t c = 0xffff;
	if (1 == 1/* req->bRequest == 0x33 && req->wIndex == 0 */) 
	{
		st_fill_rect_fast(60, 60, 30, 30, c);
		c = c == 0xffff ? 0 : 0xffff;
		st_set_address_window(0, 0, 239, 239);

		//*buf = NULL;
		//*len = 0;

		return USBD_REQ_HANDLED;
	}
	return USBD_REQ_NEXT_CALLBACK;

}

#define BIG_BUFF_SIZE 	BULK_MAX_PACKET_SIZE * 4
uint8_t big_buff[BIG_BUFF_SIZE];
uint16_t big_buff_index = 0;
// Called when data has been received
void usb_data_received(__attribute__((unused)) usbd_device *usbd_dev, __attribute__((unused)) uint8_t ep)
{
    // Retrieve USB data (has side effect of setting endpoint to VALID)
    uint8_t packet[BULK_MAX_PACKET_SIZE] __attribute__((aligned(4)));
    int len = usbd_ep_read_packet(usb_device, EP_DATA_OUT, packet, sizeof(packet));

	st_fill_color_array(packet, len);

	//usbd_ep_nak_set(usbd_dev, EP_DATA_OUT, 1);
	// copy data into circular buffer
    // for (uint8_t i = 0; i < len; i++)
	// {
	// 	big_buff[big_buff_index++] = packet[i];
	// }
	// if (big_buff_index >= BIG_BUFF_SIZE)
	// {
	// 	st_fill_color_array(big_buff, BIG_BUFF_SIZE);
	// 	big_buff_index = 0;
	// }
	//usbd_ep_nak_set(usbd_dev, EP_DATA_OUT, 0);

	//usbd_ep_nak_set(usbd_dev, EP_DATA_OUT, 1);
	
    

    // check if there is space for less than 2 packets
    // if (!is_forced_nak && buffer.avail_size() < MIN_FREE_SPACE)
    // {
    //     // set endpoint from VALID to NAK
    //     usbd_ep_nak_set(usbd_dev, EP_DATA_OUT, 1);
    //     is_forced_nak = 1;
    // }
}

// Check if endpoints can be reset from NAK to VALID
void usb_update_nak()
{
    // can be set from NAK to VALID if there is space for 2 more packets
    // if (is_forced_nak && buffer.avail_size() >= MIN_FREE_SPACE)
    // {
    //     usbd_ep_nak_set(usb_device, EP_DATA_OUT, 0);
    //     is_forced_nak = 0;
    // }
}


// USB interrupt handler
void usb_lp_can_rx0_isr()
{
    usbd_poll(usb_device);
}