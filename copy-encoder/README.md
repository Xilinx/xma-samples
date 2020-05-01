# XMA COPY ENCODER         


This package contains an encoder plugin example with XMA library (XMA2) for 2020.1 XRT. 

This is a standalone application. If needed the plugin can be integrated into higher level video framework such as FFMpeg.


## XRT installation: 

The latest 2020.1 XRT (Xilinx Runtime) is required to execute this example. 

## DSA: 

This example is developed on xilinx_u200_xdma_201830_1 DSA

## Preparing the Test input YUV files

At first, we need to generate test data to run these examples. 

Follow the [instructions][testseqreadme] to create the Test input YUV files. 

## Example structure: 

The example has three parts: 

1. Kernel
2. Plugin 
3. Standalone application or FFMpeg application


### Kernel Compilation 

#### Functionality: 

This example is solely focussing on the host code development using XMA. So the kernel used here is merely a dummy kernel that copies the inputs to the outputs.

#### Source code: 
      ./kernel/krnl_datamover.cpp

#### Software to compile the kernel: 

Vitis 2020.1 release is needed to compile the kernel

#### How to compile the kernel: 

   ``````````````````````````````````
      cd kernel
      make build TARGET=hw 
   ``````````````````````````````````
After a sucessful kernel build process, kernel compiled file .xclbin will be generated inside xclbin directory

### Plugin Compilation

The plugin code is the lower level interface that interacts with FPGA. To compile the plugin code chamge directory to plugin directory and execute the Makefile.
   
   ``````````````````````````````````
      cd plugin
      make
   ``````````````````````````````````

A dynamic library (.so) file will be generated after a successful plugin compilation process. 

** Note**: Once a kernel+plugin pair is compiled that can be used with different higher level applications. This package provides a standalone application as an example. This package also provides instructions to download a FFMpeg encoder example integrating this copy kernel+plugin pair. 

### Standalone application Compilation
The application code is the higher level host application. The application code interacts with the FPGA through the plugin API.


```
      #Make sure you are at the top level directory of this package
      make
```

You will get application executable **datamover_app**

### Runtime configuration file

During the runtime, the application code uses a configuration file that provides the system configuration description. You can find the configuration file used in this example is **./datamover_cfg.yaml**

### Running the executable 

The full command line is contained inside **./runme_standalone.sh**

During the runtime the host application picks three input video files and runs the copy kernel binary on the FPGA. Upon successful run you will get three output video files in the current directory, corresponding to three input video files. 


[testseqreadme]: ./test_videos/README.md

