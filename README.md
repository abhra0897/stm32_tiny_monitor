## STM32 Tiny Monitor

### A super tiny monitor for your PC, suitable for your pet ant. 

A python script sends the captured screen over USB to the STM32 microcontroller, then that data is displayed on ST7789 display. FPS is not that high, but I made it good enough after several optimizations.

![TinyMonitor demo](images/tinymonitor.gif)

I made this project because I'm learning USB now and this is a fun project to practice USB stuffs. I still know almost nothing about USB. To get started, follow this tutorial: https://medium.com/@manuel.bl/usb-for-microcontrollers-part-1-basic-usb-concepts-519a33372cc9


### Parts:
- STM32F103 MCU (Bluepill)
- ST7789 SPI Display (240x240 resolution)


### How to use:
1. Compile the firmware and flash the microcontroller
2. Follow the instruction in [**host_python_scripts**](host_python_scripts/README.md) folder to run the host software.
3. Connect microcontroller to PC using USB cable. Host software will start the stream.
4. Enjoy your tiny monitor :)


### Demo:
1. https://youtu.be/5AGjsRObFtk
2. https://youtu.be/oFxuoMdBxJk

### TODO:
- Optimize the device (mcu) firmware. Buffer the incoming packets befoe sending to SPI bus.
- Write host software in C/C++ to get higher performance


Feel free to modify the code and do whatever you want with this useless project.
