import usb.core
import time
import PIL
import mss
from PIL import Image

# Area of target display (here 240x240 ST7789)
cpdef unsigned int basewidth = 240
cpdef unsigned int baseheight = 240

#screenshot area
cpdef unsigned int start_x = 1475
cpdef unsigned int start_y = 635
cpdef unsigned int w = 445
cpdef unsigned int h = 445


cpdef dict monitor = {'top': start_y, 'left': start_x, 'width': w, 'height': h}

cpdef handle_dev_connection():
	dev = None
	try:
		# find device
		dev = usb.core.find(idVendor=0xcafe, idProduct=0xceaf)
		if dev is None:
			print("Device not found... Connect device")
		while dev is None:
			dev = usb.core.find(idVendor=0xcafe, idProduct=0xceaf)
			if dev is not None:
				print("Device Found!!")
				break
		else:
			print("TinyMonitor Found... :)")

		# set configuration (this is optional since device has only one configuration and OS automatically sets it on connection)
		print("Setting device configuration...")
		dev.set_configuration()
	except usb.core.USBError:
		print("ERROR: USBError occured. Re-connect the device...")
	
	return dev


cpdef rgb888_to_rgb565(unsigned int r, unsigned int g, unsigned int b):
	"""Converts a 8-bit RGB tuple to 16-bit RGB565 value"""
	cpdef unsigned int w = 0
	w = (r & 0xf8) << 8
	w |= (g & 0xfc) << 3
	w |= b >> 3
	return w


cpdef convert_rgb565(image, unsigned int byte_cnt):
	"""Converts the RGB image into a byte array in RGB565 format"""
	data = bytearray(byte_cnt)
	cdef unsigned int i = 0
	cpdef unsigned int r, g, b, w
	for (r, g, b) in image.getdata():
		w = rgb888_to_rgb565(r, g, b)
		data[i] = w >> 8
		i += 1
		data[i] = w & 0xff
		i += 1
	return data


def do_handshake(dev):
	if dev is None:
		return
	try:
		""" Send control transfer and read ISOCHRONUS transfer.
			Tried to use CONTROL and DATA for reading but failed. ISOCHRONUS works"""
			
		#bmRequestType = 0x41 means type=vendor
		#bRequest=0x33, wValue=0x88. Device will wait for this value. Once received, device will prepare to receive screen data
		while True:
			dev.ctrl_transfer(bmRequestType=0x41, bRequest=0x33, wValue=0x88, wIndex=0)
			# print("Read:", dev.read(0x82, 1, 200))
			rx_buf = dev.read(0x82, 1, 200) #reading from IN ep 0x82 (ISOCHRONUS)
			# if we receive the magic number from dev, we know that dev is ready to receive stream
			if rx_buf[0] == 0xAA:
				print("Handshake complete...")
				break
	except usb.core.USBError:
		print("ERROR: USBError occured. Re-connect the device...")
		return



cpdef stream_out(dev):
	if dev is None:
		return
	print("Streaming started...")

	sct = mss.mss()
	cpdef unsigned int pixels_cnt = basewidth * baseheight
	cpdef unsigned int byte_cnt = pixels_cnt * 2
	pixels_rgb565_8bit = bytearray(byte_cnt)
	while True:
		## take a screenshot
		img = Image.frombytes('RGB', (w, h), sct.grab(monitor).rgb)			# <-- mss is way fater than pyautogui
		
		img = img.resize((basewidth, baseheight), PIL.Image.ANTIALIAS)

		## get raw pixel values in (r8, g8, b8, a8) format
		#pixels_rgb888 = list(img.getdata())


		## make rgb565 16 bit value from (r8, g8, b8, a8)
		## store the 16 bit value in the bytearray as two 8 bits (firmware expects each color to be seperated into two bytes)
		#for pix in range (pixels_cnt):
			#rgb565_16bit = (int(pixels_rgb888[pix][0] / 255 * 31) << 11) | (int(pixels_rgb888[pix][1] / 255 * 63) << 5) | (int(pixels_rgb888[pix][2] / 255 * 31))
			#pixels_rgb565_8bit[pix*2] = (rgb565_16bit >> 8)         #storing MSB
			#pixels_rgb565_8bit[pix*2 + 1] = (rgb565_16bit & 255)    #storing LSB
			
		pixels_rgb565_8bit = convert_rgb565(img, byte_cnt)

		## send all the 8 bit values from bytearray over USB
		try:
			dev.write(0x01, pixels_rgb565_8bit, 2000)
		except usb.core.USBError:
			print("ERROR: USBError occured. Re-connect the device...")
			return
		

cpdef initialize():
	while True:
		print("\n------------- [ initialize ] -------------\n")
		usb_device = handle_dev_connection()
		# Delay is not really needed
		time.sleep(1)
		print("\n---------------- [ end ] ----------------\n\n")
		
		print("------------- [ handshake ] -------------\n")
		do_handshake(usb_device)
		# Delay is not really needed
		time.sleep(2)
		print("\n---------------- [ end ] ----------------\n\n")
		
		print("------------- [ stream start ] -------------\n")
		stream_out(usb_device)
		print("\n---------------- [ end ] ----------------\n\n\n")
	
	
# Call the starting point
initialize()
