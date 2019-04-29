KERNEL  	    = krnl_datamover
NKERNEL 	    = 3
PLATFORM  	    = xilinx_u200_xdma_201830_1

KERNEL_SOURCE_FILES = krnl_datamover.cpp


XO_FILE	= xclbin/${KERNEL}.${TARGET}.${PLATFORM}.xo
XCLBIN_FILE = xclbin/fpga.${NKERNEL}k.${TARGET}.${PLATFORM}.xclbin


${XO_FILE}: ${KERNEL_SOURCE_FILES} 
	mkdir -p xclbin
	xocc -c -t ${TARGET} -s --platform ${PLATFORM} -k ${KERNEL} ${KERNEL_SOURCE_FILES} -o $@ 

${XCLBIN_FILE}: ${XO_FILE}
	xocc -l -t ${TARGET} --nk ${KERNEL}:${NKERNEL}:krnl_0.krnl_1.krnl_2 --sp krnl_0.m_axi_gmem:bank0 --sp krnl_1.m_axi_gmem:bank1 --sp krnl_2.m_axi_gmem:bank2 -s --platform ${PLATFORM} $<  -o $@ 

build: ${XCLBIN_FILE}

hw:
	make build TARGET=hw


clean:
	rm -rf sdaccel_* emconfig.json emulation* xsim* *.wcfg *.wdb .Xil prj sdx_* *.log xclbin/* _xocc* _x ${XO_FILE} *.o Kimono_out.yuv
