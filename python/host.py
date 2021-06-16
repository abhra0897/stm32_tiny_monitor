#import usb1
#with usb1.USBContext() as context:
    #handle = context.openByVendorIDAndProductID(0xcafe, 0xceaf, skip_on_error=True,)
    #if handle is None:
        #Device not present, or user is not allowed to access device.
        #print("nada")
    #with handle.claimInterface(0):
        #Do stuff with endpoints on claimed interface. 
        #print("hudda")

import usb.core
import time
import PIL
#import pyautogui
import mss
from PIL import Image


basewidth = 240
baseheight = 240

start_x = 1475
start_y = 635
w = 445
h = 445



monitor = {'top': start_y, 'left': start_x, 'width': w, 'height': h}

# find device
dev = usb.core.find(idVendor=0xcafe, idProduct=0xceaf)
if dev is None:
    raise ValueError('Device not found')
else :
	print("Found!!")

# set configuration
dev.set_configuration()

#usb.util.claim_interface(dev, 0)

led_on = False

byte_len = 240 * 240 * 2

def create_data(val):
	data = bytearray(byte_len)
	i = 0
	for i in range(byte_len):
		data[i] = val
		if val == 255:
			val = 0
		val += 0
	return data


def rgb888_to_rgb565(r, g, b):
	"""Converts a 8-bit RGB tuple to 16-bit RGB565 value"""
	w = (r & 0xf8) << 8
	w |= (g & 0xfc) << 3
	w |= b >> 3
	return w

def convert_rgb565(image):
	"""Converts the RGB image into a byte array in RGB565 format"""
	data = bytearray(240 * 240 * 2)
	i = 0
	for (r, g, b, a) in image.getdata():
		w = rgb888_to_rgb565(r, g, b)
		data[i] = w >> 8
		i += 1
		data[i] = w & 0xff
		i += 1
	return data

#val = 0
#while True:
	
	#led_on = not led_on
	#bmRequestType = 0x41 means type=vendor
	#dev.ctrl_transfer(bmRequestType=0x41, bRequest=0x33, wValue=int(led_on), wIndex=0)
	#val = val + 1
	#if val >= 255:
		#val = 0
	#pixels = create_data(val)
    
	#dev.write(1, pixels, 2000)


# load image
#img = Image.open("win10hd.png")
#img = img.resize((basewidth, baseheight), PIL.Image.ANTIALIAS)
#pixels = convert_rgb565(img)
 
#while True:
	#dev.write(1, pixels, 2000)

sct = mss.mss()
pixels_cnt = basewidth * baseheight
pixels_rgb565_8bit = bytearray(pixels_cnt * 2)
while True:
	#img = Image.open('win10hd.png','r')
	## take a screenshot
	#img = pyautogui.screenshot(region=(start_x,start_y, w, h))			# <---this guy is slow
	img = Image.frombytes('RGB', (w, h), sct.grab(monitor).rgb)			# <-- mss is way fater than pyautogui
	
	#wpercent = (basewidth / float(img.size[0]))
	#hsize = int((float(img.size[1]) * float(wpercent)))

	
	img = img.resize((basewidth, baseheight), PIL.Image.ANTIALIAS)

	## get raw pixel values in (r8, g8, b8, a8) format
	pixels_rgb888 = list(img.getdata())


	## make rgb565 16 bit value from (r8, g8, b8, a8)
	## store the 16 bit value in the bytearray as two 8 bits (firmware expects each color to be seperated into two bytes)
	for pix in range (pixels_cnt):
		rgb565_16bit = (int(pixels_rgb888[pix][0] / 255 * 31) << 11) | (int(pixels_rgb888[pix][1] / 255 * 63) << 5) | (int(pixels_rgb888[pix][2] / 255 * 31))
		pixels_rgb565_8bit[pix*2] = (rgb565_16bit >> 8)         #storing MSB
		pixels_rgb565_8bit[pix*2 + 1] = (rgb565_16bit & 255)    #storing LSB

	## send all the 8 bit values from bytearray over USB
	dev.write(1, pixels_rgb565_8bit, 2000)


