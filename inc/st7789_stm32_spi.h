/*
MIT License

Copyright (c) 2020 Avra Mitra

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#include "fonts/bitmap_typedefs.h"
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/dma.h>
#include <stdlib.h>

#ifndef INC_ST7789_STM32_SPI_H_
#define INC_ST7789_STM32_SPI_H_


#define ST7789_NOP0x00
#define ST7789_SWRESET		0x01
#define ST7789_RDDID		0x04
#define ST7789_RDDST		0x09

#define ST7789_SLPIN		0x10
#define ST7789_SLPOUT  		0x11
#define ST7789_PTLON   		0x12
#define ST7789_NORON   		0x13

#define ST7789_INVOFF  		0x20
#define ST7789_INVON   		0x21
#define ST7789_DISPOFF 		0x28
#define ST7789_DISPON  		0x29
#define ST7789_CASET   		0x2A
#define ST7789_RASET   		0x2B
#define ST7789_RAMWR   		0x2C
#define ST7789_RAMRD   		0x2E

#define ST7789_PTLAR   		0x30
#define ST7789_COLMOD  		0x3A
#define ST7789_MADCTL  		0x36

#define ST7789_MADCTL_MY  0x80  // Page Address Order
#define ST7789_MADCTL_MX  0x40  // Column Address Order
#define ST7789_MADCTL_MV  0x20  // Page/Column Order
#define ST7789_MADCTL_ML  0x10  // Line Address Order
#define ST7789_MADCTL_MH  0x04  // Display Data Latch Order
#define ST7789_MADCTL_RGB 0x00
#define ST7789_MADCTL_BGR 0x08

#define ST7789_RDID1   		0xDA
#define ST7789_RDID2   		0xDB
#define ST7789_RDID3   		0xDC
#define ST7789_RDID4   		0xDD

// color modes
#define ST7789_COLOR_MODE_65K      0x50
#define ST7789_COLOR_MODE_262K     0x60
#define ST7789_COLOR_MODE_12BIT    0x03
#define ST7789_COLOR_MODE_16BIT    0x05
#define ST7789_COLOR_MODE_18BIT    0x06
#define ST7789_COLOR_MODE_16M      0x07

/*
#define ILI_PWCTR6  0xFC
*/


// Color definitions

#define	ST_R_POS_RGB   11	// Red last bit position for RGB display
#define	ST_G_POS_RGB   5 	// Green last bit position for RGB display
#define	ST_B_POS_RGB   0	// Blue last bit position for RGB display

#define	ST_RGB(R,G,B) \
	(((uint16_t)(R >> 3) << ST_R_POS_RGB) | \
	((uint16_t)(G >> 2) << ST_G_POS_RGB) | \
	((uint16_t)(B >> 3) << ST_B_POS_RGB))

#define ST_COLOR_BLACK       ST_RGB(0,     0,   0)
#define ST_COLOR_NAVY        ST_RGB(0,     0, 123)
#define ST_COLOR_DARKGREEN   ST_RGB(0,   125,   0)
#define ST_COLOR_DARKCYAN    ST_RGB(0,   125, 123)
#define ST_COLOR_MAROON      ST_RGB(123,   0,   0)
#define ST_COLOR_PURPLE      ST_RGB(123,   0, 123)
#define ST_COLOR_OLIVE       ST_RGB(123, 125,   0)
#define ST_COLOR_LIGHTGREY   ST_RGB(198, 195, 198)
#define ST_COLOR_DARKGREY    ST_RGB(123, 125, 123)
#define ST_COLOR_BLUE        ST_RGB(0,     0, 255)
#define ST_COLOR_GREEN       ST_RGB(0,   255,   0)
#define ST_COLOR_CYAN        ST_RGB(0,   255, 255)
#define ST_COLOR_RED         ST_RGB(255,   0,   0)
#define ST_COLOR_MAGENTA     ST_RGB(255,   0, 255)
#define ST_COLOR_YELLOW      ST_RGB(255, 255,   0)
#define ST_COLOR_WHITE       ST_RGB(255, 255, 255)
#define ST_COLOR_ORANGE      ST_RGB(255, 165,   0)
#define ST_COLOR_GREENYELLOW ST_RGB(173, 255,  41)
#define ST_COLOR_PINK        ST_RGB(255, 130, 198)


/**
 * Pin mapping:
 * ST7789				STM32
 * ---------------------------
 * SDA					PA7
 * SCL					PA5				..			
 * RESETn				PA4
 * D/Cn					PA2
 * BLK					PA3
 */

#define ST_USE_SPI_DMA
#define ST_HAS_RST
//#define ST_HAS_CS
#ifdef ST_HAS_CS
//	#define ST_RELEASE_WHEN_IDLE
#endif

#define ST_SPI			SPI1
#ifdef ST_USE_SPI_DMA
	#define ST_DMA			DMA1
	#define ST_DMA_CHANNEL	3
#endif

#define ST_PORT			GPIOA

#define ST_RST			GPIO4
#define ST_DC			GPIO2
#define ST_SDA			GPIO7
#define ST_SCL			GPIO5
#define ST_BLK			GPIO3
// To use Chip Select (CS), uncomment `HAS_CS` above
#define ST_CS			GPIO6


#define ST_DC_CMD			GPIO_BRR(ST_PORT) = ST_DC
#define ST_DC_DAT			GPIO_BSRR(ST_PORT) = ST_DC
#define ST_RST_ACTIVE		GPIO_BRR(ST_PORT) = ST_RST
#define ST_RST_IDLE			GPIO_BSRR(ST_PORT) = ST_RST
#ifdef ST_HAS_CS
	#define ST_CS_ACTIVE		GPIO_BRR(ST_PORT) = ST_CS
	#define ST_CS_IDLE			GPIO_BSRR(ST_PORT) = ST_CS
#endif

#define ST_CONFIG_GPIO_CLOCK()	{ \
									rcc_periph_clock_enable(RCC_GPIOA); \
									rcc_periph_clock_enable(RCC_AFIO); \
									rcc_periph_clock_enable(RCC_SPI1); \
									rcc_periph_clock_enable(RCC_DMA1); \
								}
#ifdef ST_HAS_CS
	#define ST_CONFIG_GPIO()		{ \
										/*Configure GPIO pins : PA2 PA3 PA4 PA5 PA7 */ \
										gpio_set_mode(ST_PORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, ST_SCL|ST_SDA); \
										gpio_set_mode(ST_PORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, ST_DC|ST_BLK|ST_RST|ST_CS); \
										/*Configure GPIO pin Output Level */ \
										gpio_set(ST_PORT, ST_BLK|ST_RST|ST_DC|ST_CS); \
										/* Configures PB4 as GPIO */ \
										AFIO_MAPR |= AFIO_MAPR_SWJ_CFG_FULL_SWJ_NO_JNTRST; \
									}
#else
	#define ST_CONFIG_GPIO()		{ \
										/*Configure GPIO pins : PA2 PA3 PA4 PA5 PA7 */ \
										gpio_set_mode(ST_PORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, ST_SCL|ST_SDA); \
										gpio_set_mode(ST_PORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, ST_DC|ST_BLK|ST_RST); \
										/*Configure GPIO pin Output Level */ \
										gpio_set(ST_PORT, ST_BLK|ST_RST|ST_DC); \
										/* Configures PB4 as GPIO */ \
										AFIO_MAPR |= AFIO_MAPR_SWJ_CFG_FULL_SWJ_NO_JNTRST; \
									}
#endif

#define ST_CONFIG_SPI()			{ \
									/* Reset SPI, SPI_CR1 register cleared, SPI is disabled */ \
									spi_reset(ST_SPI); \
									SPI_I2SCFGR(ST_SPI) = 0; \
									/* Must use SPI_MODE = 2. (CPOL 1, CPHA 0) */\
									/* Read about SPI MODEs: https://en.wikipedia.org/wiki/Serial_Peripheral_Interface*/ \
									spi_init_master(ST_SPI, SPI_CR1_BAUDRATE_FPCLK_DIV_2, SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE, SPI_CR1_CPHA_CLK_TRANSITION_1, SPI_CR1_DFF_8BIT, SPI_CR1_MSBFIRST); \
									spi_enable_software_slave_management(ST_SPI); \
									spi_set_full_duplex_mode(ST_SPI); \
									spi_set_nss_high(ST_SPI); \
									/* Enable SPI1 periph. */ \
									spi_enable(ST_SPI); \
								}


#define ST_SWAP(a, b)		{uint16_t temp; temp = a; a = b; b = temp;}


// Either use TXE and BSY flag check to ensure transaction completes, 
// or use nops to get higher fps. Add more nops if display is not working properly
// Remove nops if fps is too low. 
// Rule of thumb: If optimization flag value is set to higher, and/or MCU is overclocked, then increase nops

// Note: If display shows wrong output, either change number of nops as said above,
//		 or uncomment TXE and BSY flag checks and comment out all nops.
#define ST_WRITE_8BIT(d)	do{ \
							SPI_DR(ST_SPI) = (uint8_t)(d); \
							/*while (!(SPI_SR(ST_SPI) & SPI_SR_TXE));*/ \
							/*while (SPI_SR(ST_SPI) & SPI_SR_BSY);*/ \
							__asm__("nop"); __asm__("nop"); __asm__("nop"); __asm__("nop"); \
							__asm__("nop"); __asm__("nop"); __asm__("nop"); __asm__("nop"); \
							__asm__("nop"); __asm__("nop"); __asm__("nop"); __asm__("nop"); \
							__asm__("nop"); __asm__("nop"); __asm__("nop"); __asm__("nop"); \
							__asm__("nop"); __asm__("nop"); __asm__("nop"); __asm__("nop"); \
						} while(0)

#ifdef ST_USE_SPI_DMA

	#define ST_CONFIG_SPI_DMA()		{ \
									/* DMA Peripheral address set to SPI*/ \
									DMA_CPAR(ST_DMA, ST_DMA_CHANNEL) = (uint32_t)&SPI_DR(ST_SPI); \
									/* Dma memory address is reset */ \
									DMA_CMAR(ST_DMA, ST_DMA_CHANNEL) = 0; \
									/* Number of data transfer is reset */ \
									DMA_CNDTR(ST_DMA, ST_DMA_CHANNEL) = 0; \
									/* DMA priority is high */ \
									DMA_CCR(ST_DMA, ST_DMA_CHANNEL) = DMA_CCR_PL_HIGH; \
									/* Data transfer direction: Read from memory */ \
									DMA_CCR(ST_DMA, ST_DMA_CHANNEL) |= DMA_CCR_DIR; \
									/* Disable circular DMA */ \
									DMA_CCR(ST_DMA, ST_DMA_CHANNEL) &= ~DMA_CCR_CIRC; \
									/* peripheral increment disabled */\
									DMA_CCR(ST_DMA, ST_DMA_CHANNEL) &= ~DMA_CCR_PINC; \
									/* memory increment enabled */ \
									DMA_CCR(ST_DMA, ST_DMA_CHANNEL) |= DMA_CCR_MINC; \
									/* peripheral and memory data size set to 8 bit */ \
									DMA_CCR(ST_DMA, ST_DMA_CHANNEL) |= DMA_CCR_PSIZE_8BIT | DMA_CCR_MSIZE_8BIT; \
								}

	
	__attribute__((always_inline)) static inline void _st_write_spi_dma(void *data_addr, uint16_t length)
	{
		// Set memory source address
		DMA_CMAR(ST_DMA, ST_DMA_CHANNEL) = (uint32_t)data_addr;
		// set data count
		DMA_CNDTR(ST_DMA, ST_DMA_CHANNEL) = length;

		// Enable DMA channel 
		DMA_CCR(ST_DMA, ST_DMA_CHANNEL) |= DMA_CCR_EN;
		// Enable SPI DMA. This will start the DMA transaction
		SPI_CR2(ST_SPI) |= SPI_CR2_TXDMAEN;

		// wait until all data is sent (count becomes 0)
		while (DMA_CNDTR(ST_DMA, ST_DMA_CHANNEL));
		// Wait until tx buffer is empty (not set)
		while (!(SPI_SR(ST_SPI) & SPI_SR_TXE));
		// Wait until bus is not busy
		while (SPI_SR(ST_SPI) & SPI_SR_BSY);
	
		// Disable SPI DMA tx
		SPI_CR2(ST_SPI) &= ~SPI_CR2_TXDMAEN;
		// Disable DMA channel
		DMA_CCR(ST_DMA, ST_DMA_CHANNEL) &= ~DMA_CCR_EN;
	}

#endif

/*
 * inline function to send 8 bit command to the display
 * User need not call it
 */
__attribute__((always_inline)) static inline void _st_write_command_8bit(uint8_t cmd)
{
	#ifdef ST_RELEASE_WHEN_IDLE
		CS_ACTIVE;
	#endif
	ST_DC_CMD;
	ST_WRITE_8BIT(cmd);
	#ifdef ST_RELEASE_WHEN_IDLE
		CS_IDLE;
	#endif
}

/*
 * inline function to send 8 bit data to the display
 * User need not call it
 */
__attribute__((always_inline)) static inline void _st_write_data_8bit(uint8_t dat)
{
	#ifdef ST_RELEASE_WHEN_IDLE
		CS_ACTIVE;
	#endif
	ST_DC_DAT;
	ST_WRITE_8BIT(dat);
	#ifdef ST_RELEASE_WHEN_IDLE
		CS_IDLE;
	#endif
}

/*
 * inline function to send 16 bit data to the display
 * User need not call it
 */
__attribute__((always_inline)) static inline void _st_write_data_16bit(uint16_t dat)
{
	#ifdef ST_RELEASE_WHEN_IDLE
		CS_ACTIVE;
	#endif
	ST_DC_DAT;
	ST_WRITE_8BIT((uint8_t)(dat >> 8));
	ST_WRITE_8BIT((uint8_t)dat);
	#ifdef ST_RELEASE_WHEN_IDLE
		CS_IDLE;
	#endif
}

/*
* function prototypes
*/

/**
 * fixed delay of 5000 NOPs + time due to looping.
 * Used for sendinf start sequence
 */
void _st_fixed_delay();

/**
 * Set an area for drawing on the display with start row,col and end row,col.
 * User don't need to call it usually, call it only before some functions who don't call it by default.
 * @param x1 start column address.
 * @param y1 start row address.
 * @param x2 end column address.
 * @param y2 end row address.
 */
void st_set_address_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

/**
 * Fills `len` number of pixels with `color`.
 * Call st_set_address_window() before calling this function.
 * @param color 16-bit RGB565 color value
 * @param len 32-bit number of pixels
 */
void st_fill_color(uint16_t color, uint32_t len);

/**
 * Fills `len` number of pixels with `color`.
 * Call st_set_address_window() before calling this function.
 * @param color_arr pointer to uint8_t array. Each 16-bit color is seperated into two 8-bit `high` and `low` components
 * @param bytes 32-bit number of bytes in the array (= no. of pixels x2)
 */

void st_fill_color_array(uint8_t *color_arr, uint32_t bytes);

/**
 * Draw a line from (x0,y0) to (x1,y1) with `width` and `color`.
 * @param x0 start column address.
 * @param y0 start row address.
 * @param x1 end column address.
 * @param y1 end row address.
 * @param width width or thickness of the line
 * @param color 16-bit RGB565 color of the line
 */
void st_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t width, uint16_t color);

/**
 * Experimental
 * Draw a rectangle without filling it
 * @param x start column address.
 * @param y start row address
 * @param w Width of rectangle
 * @param h height of rectangle
 */
void st_draw_rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

/*
 * Called by st_draw_line().
 * User need not call it
 */
void _st_plot_line_low(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t width, uint16_t color);

/*
 * Called by st_draw_line().
 * User need not call it
 */
void _st_plot_line_high(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t width, uint16_t color);

/*
 * Called by st_draw_line().
 * User need not call it
 */
void _st_draw_fast_h_line(uint16_t x0, uint16_t y0, uint16_t x1, uint8_t width, uint16_t color);

/*
 * Called by st_draw_line().
 * User need not call it
 */
void _st_draw_fast_v_line(uint16_t x0, uint16_t y0, uint16_t y1, uint8_t width, uint16_t color);

/**
 * Rotate the display clockwise or anti-clockwie set by `rotation`
 * @param rotation Type of rotation. Supported values 0, 1, 2, 3
 */
void st_rotate_display(uint8_t rotation);

/**
 * Initialize the display driver
 */
void st_init();

/**
 * Fills a rectangular area with `color`.
 * Before filling, performs area bound checking
 * @param x Start col address
 * @param y Start row address
 * @param w Width of rectangle
 * @param h Height of rectangle
 * @param color 16-bit RGB565 color
 */
void st_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

/*
 * Same as `st_fill_rect()` but does not do bound checking, so it's slightly faster
 */
void st_fill_rect_fast(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

/**
 * Fill the entire display (screen) with `color`
 * @param color 16-bit RGB565 color
 */
void st_fill_screen(uint16_t color);

/*
 * Render a character glyph on the display. Called by `st_draw_string_main()`
 * User need NOT call it
 */
void _st_render_glyph(uint16_t x, uint16_t y, uint16_t fore_color, uint16_t back_color, const tImage *glyph, uint8_t is_bg);

/**
 * Renders a string by drawing each character glyph from the passed string.
 * Called by `st_draw_string()` and `st_draw_string_withbg()`.
 * Text is wrapped automatically if it hits the screen boundary.
 * x_padding and y_padding defines horizontal and vertical distance (in px) between two characters
 * is_bg=1 : Text will habe background color,   is_bg=0 : Text will have transparent background
 * User need NOT call it.
 */
void _st_draw_string_main(uint16_t x, uint16_t y, char *str, uint16_t fore_color, uint16_t back_color, const tFont *font, uint8_t is_bg);

/**
 * Draws a character at a given position, fore color, back color.
 * @param x Start col address
 * @param y Start row address
 * @param character the ASCII character to be drawn
 * @param fore_color foreground color
 * @param back_color background color
 * @param font Pointer to the font of the character
 * @param is_bg Defines if character has background or not (transparent)
 */
void st_draw_char(uint16_t x, uint16_t y, char character, uint16_t fore_color, uint16_t back_color, const tFont *font, uint8_t is_bg);

/**
 * Draws a string on the display with `font` and `color` at given position.
 * Background of this string is transparent
 * @param x Start col address
 * @param y Start y address
 * @param str pointer to the string to be drawn
 * @param color 16-bit RGB565 color of the string
 * @param font Pointer to the font of the string
 */
void st_draw_string(uint16_t x, uint16_t y, char *str, uint16_t color, const tFont *font);

/**
 * Draws a string on the display with `font`, `fore_color`, and `back_color` at given position.
 * The string has background color
 * @param x Start col address
 * @param y Start y address
 * @param str pointer to the string to be drawn
 * @param foe_color 16-bit RGB565 color of the string
 * @param back_color 16-bit RGB565 color of the string's background
 * @param font Pointer to the font of the string
 */
void st_draw_string_withbg(uint16_t x, uint16_t y, char *str, uint16_t fore_color, uint16_t back_color, const tFont *font);

/**
 * Draw a bitmap image on the display
 * @param x Start col address
 * @param y Start row address
 * @param bitmap Pointer to the image data to be drawn
 */
void st_draw_bitmap(uint16_t x, uint16_t y, const tImage *bitmap);
//void st_draw_bitmap_old(uint16_t x, uint16_t y, const tImage16bit *bitmap);

/**
 * Draw a pixel at a given position with `color`
 * @param x Start col address
 * @param y Start row address
 */
void st_draw_pixel(uint16_t x, uint16_t y, uint16_t color);

//------------------------------------------------------------------------
#endif /* INC_ST7789_STM32_SPI_H_ */
