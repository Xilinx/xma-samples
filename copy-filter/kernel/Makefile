KERNEL  	    = krnl_copyfilter
NKERNEL 	    = 1
PLATFORM  	    = xilinx_u200_xdma_201830_1

KERNEL_SOURCE_FILES = kernel.cpp


XO_FILE	= xclbin/${KERNEL}.${TARGET}.${PLATFORM}.xo
XCLBIN_FILE = xclbin/fpga.${NKERNEL}k.${TARGET}.${PLATFORM}.xclbin


${XO_FILE}: ${KERNEL_SOURCE_FILES} 
	mkdir -p xclbin
	xocc -c -t ${TARGET} -s --platform ${PLATFORM} -k ${KERNEL} ${KERNEL_SOURCE_FILES} -o $@ 

${XCLBIN_FILE}: ${XO_FILE}
	xocc -l -t ${TARGET} --nk ${KERNEL}:${NKERNEL}:krnl_0 -s --platform ${PLATFORM} $<  -o $@ 

build: ${XCLBIN_FILE}

hw:
	make build TARGET=hw


clean:
	rm -rf sdaccel_* emconfig.json emulation* xsim* *.wcfg *.wdb .Xil prj sdx_* *.log xclbin/* _xocc* _x ${XO_FILE} *.o Kimono_out.yuv

