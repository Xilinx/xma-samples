KERNEL  	    = krnl_datamover
NKERNEL 	    = 3
PLATFORM            = xilinx_u200_qdma_201920_1

KERNEL_SOURCE_FILES = krnl_datamover.cpp


XO_FILE	= xclbin/${KERNEL}.${TARGET}.${PLATFORM}.xo
XCLBIN_FILE = xclbin/fpga.${NKERNEL}k.${TARGET}.${PLATFORM}.xclbin


${XO_FILE}: ${KERNEL_SOURCE_FILES} 
	mkdir -p xclbin
	v++ -c -t ${TARGET} -s --platform ${PLATFORM} -k ${KERNEL} ${KERNEL_SOURCE_FILES} -o $@ 

${XCLBIN_FILE}: ${XO_FILE}
	v++ -l -t ${TARGET} -s --platform ${PLATFORM} $<  -o $@ --config vpp_link.cfg

build: ${XCLBIN_FILE}

hw:
	make build TARGET=hw


clean:
	rm -rf  emconfig.json emulation* xsim* *.wcfg *.wdb .Xil prj  *.log xclbin/* _xocc* _x ${XO_FILE} *.o Kimono_out.yuv
