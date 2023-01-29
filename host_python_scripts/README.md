## Setup

### 1. Linux:

1. Install required python packages using pip:
    - pyusb
    - libusb
    - mss
2. Add the udev rule `50-usb-tinymonitor.rules` file inside `/etc/udev/rules.d/` folder.
3. Add current user to `plugdev` using: `adduser <username> plugdev`
4. Reload and restart udev using:
    - `sudo udevadm control --reload`
    - `sudo udevadm trigger`
    - `sudo service udev reload`
    - `sudo service udev restart`
5. Connect the device using USB. The display should show "USB connected. Config is set..."
6. Run the host software. You can modify the display capture area inside the python code.

### 2. Windows:

1. Install required python packages using pip:
    - pyusb
    - libusb
    - mss
2. Download `zadig` from https://zadig.akeo.ie/
3. Plugin the device and run `zadig`.
4. Select the device named "display" from the first dropdown. Make sure the USB ID shows: `CAFE` `CEAF`
5. Select "libusb-win32" driver from the right drop down. Then click "Install Driver" button.
6. It should take some time. After installation done, the display should show "USB connected. Config is set..."
7. Run the host software.

## Host Softwares

These are the python scripts to run on host (PC) for sending the screen data to device (STM32 Tiny Monitor). There are two folders
1. Slow Host
2. Fast Host

### Slow Host

Slow host contains the original python script that I've written to establsih the data stream between host and device. This one works fine, but it's slow. Python's slowness in capturing screen and processing the captured data is causing the bottleneck. 

To start host,

`python host.py`

### Fast Host

To solve the aboce issue, the best way is to write the host software in C, which I'll do eventually. But for now, I gave "cython" a try, and it surely increased the performance noticeably. This is completely based on the Slow Host's python script but with some minor change to cythonize it. Then compiled to a .c file using setup.py. The main script is renamed from `host.py` to `fast_host.pyx`.

To compile, 

`python setup.py build_ext --inplace`

To run, either

`python start_host.py`

or

```
python
>>> import fast_host
```

## Note

No need to run the Slow Host. This is kept here just for reference. Use Fast Host for best performance (higher fps).

I'll soon write a C/C++ host software to get even better speed (hopefully).
