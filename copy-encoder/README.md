# XMA COPY ENCODER         


This package contains an encoder plugin example with XMA library for 2018.3 XRT. 

Two applications are provided with this encoder plugin example.

   1. A Standalone application: Provided with this repo
   2. A FFMpeg application: Can be downloded from Open-Sourced FFMpeg-XMA

## Changes from the previous versions: 

In earlier versions of XMA plugin **xma_plg_register_write** and **xlc_plg_register_read** were used for various purposes. However starting from 2018.3, **xma_plg_register_write** and **xlc_plg_register_read** are depricated and new APIs are provided at a higher level of abstraction. The new APIs are purposed-based. So instead of direct register read/write the user will use appropriate higher-level purposed based API to achieve the same result. 
  
Towards that end, XMA now offers a new execution model with three brand new APIs. 

The new APIs are: 
  
  * xma_plg_register_prep_write
  * xma_plg_schedule_work_item
  * xma_plg_is_work_item_done 

Lets consider the various purposes where the above APIs would be useful. 

### Purpose 1: 
The API **xma_plg_register_write** was used to send scaler inputs to the kernel by directly writing to the AXI-LITE registers. Now the higher level API **xma_plg_register_prep_write** should be used for the same purpose. 

### Purpose 2: 
The API **xma_plg_register_write** was also used to start the kernel by writing to the start bit of the AXI-LITE registers. For this purpose the new API **xma_plg_schedule_work_item** should be used instead of **xma_plg_register_write**.

### Purpose 3: 
The API **xma_plg_register_read** was used to check kernel idle status (by reading AXI-LITE register bit) to determine if the kernel finished processing the operation. For this purpose now the new API **xma_plg_is_work_item_done** should be used.

The below table summarizes how to migrate to the new APIs from **xma_plg_register_write**/**xma_plg_register_read**.  



| Purposes   |  Earlier register read/write API |  New API  |
|---|---|---|
| Sending scalar input  | xma_plg_register_write  |  xma_plg_register_prep_write |
| Starting the kernel  |  xma_plg_register_write |  xma_plg_schedule_work_item |
| Checking if kernel finished processing | xma_plg_register_read | xma_plg_is_work_item_done |

## XRT installation: 

The latest 2018.3 XRT (Xilinx Runtime) is required to execute this example. 

Please download and build the latest XRT
      https://github.com/Xilinx/XRT/tree/2018.3  (2018.3 branch). 

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

SDx 2018.3 release is needed to compile the kernel

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

### FFMpeg application Compilation
  
The FFMpeg application is accessbile from Open-Sourced FFMpeg-XMA git repo. 

Please download and build FFMpeg-XMA from the following repo

https://github.com/Xilinx/FFmpeg-xma/tree/2018.3 (2018.3 branch)

FFMpeg-XMA contains an encoder **xlnx_copy_enc** that utilize this copy-kernel

### Runtime configuration file for FFMpeg

By default (unless you want to customize the FFMpeg-XMA code), ffmpeg executable expects the runtime configuration file as /var/tmp/xilinx/xmacfg.yaml

```
      cp ./datamover_cfg.yaml /var/tmp/xilinx/xmacfg.yaml
```
   
However, after the copy operation, you need to modify the *pluginpath* and *xclbinpath* to full-path of plugin shared library and xclbin file respectively

### Running the FFMpeg executable 

```
   <FFMpeg-XMA Path>/ffmpeg -f rawvideo -s:v 1920x1080 -pix_fmt yuv420p -r 60 -i ../test_videos/crowd8_420_1080p50.yuv -frames 500 -global_quality 0 -b:v 0 -g 30 -c:v xlnx_copy_enc -f rawvideo -pix_fmt yuv420p -s:v 1920x1080 -r 60 -y -map 0 crowd_run_1080p60_out_1c.yuv
```
   
The above command line encoding the one of the video through encoder **xlnx_copy_enc**. The registered enoder **xlnx_copy_enc** simply using the plugin library to offload the copy kernel on the FPGA to copy the input video to the output video.

[testseqreadme]: ./test_videos/README.md

