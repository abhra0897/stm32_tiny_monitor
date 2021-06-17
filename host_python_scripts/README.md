## Host Software

These are the python scripts to run on host (PC) for sending the screen data to device (STM32 Tiny Monitor). There are two folders
1. Slow Host
2. Fast Host

### SLow Host

Slow host contains the original python script that I've written to establsih the dta a stream between host and device. This one works fine, but it's slow. Python's slowness in capturing screen and processing the captured data is causing the bottleneck. 

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
