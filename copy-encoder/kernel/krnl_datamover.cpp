#ifndef __SYNTHESIS__
#include <stdio.h>
#endif
#include <stdio.h>
#include <assert.h>
#include "ap_int.h"
#include "hls_stream.h"

void krnl_datamover_core(
				ap_uint<512>* srcYImg,
				ap_uint<512>* srcUImg,
				ap_uint<512>* srcVImg,
				unsigned int FRAME_WIDTH,
				unsigned int FRAME_HEIGHT,
				unsigned int QP,
				unsigned int BITRATE,
				unsigned int INTRA_PERIOD,
				unsigned int dummy_outRatio,
				unsigned int dummy_delay,				
				ap_uint<512>* dstImg,
				ap_uint<512>* dstRef,				
				ap_uint<64>* dstNAL_size,
				ap_uint<64>* dstDummy_Cnt
                 );
				 
void copy(ap_uint<512>*srcImg, ap_uint<512>*dstImg, int burst_num);				 

extern "C" {
void krnl_datamover(
				ap_uint<512>* srcYImg,
				ap_uint<512>* srcUImg,
				ap_uint<512>* srcVImg,
				unsigned int FRAME_WIDTH,
				unsigned int FRAME_HEIGHT,
				unsigned int QP,
				unsigned int BITRATE,
				unsigned int INTRA_PERIOD,
				unsigned int dummy_outRatio,
				unsigned int dummy_delay,				
				ap_uint<512>* dstImg,
				ap_uint<512>* dstRef,				
				ap_uint<64>* dstNAL_size,
				ap_uint<64>* dstDummy_Cnt							
                 )
{

#pragma HLS INTERFACE m_axi port=srcYImg offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=srcUImg offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=srcVImg offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=dstImg offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=dstRef offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=dstNAL_size offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=dstDummy_Cnt offset=slave bundle=gmem


#pragma HLS INTERFACE s_axilite port=srcYImg bundle=control
#pragma HLS INTERFACE s_axilite port=srcUImg bundle=control
#pragma HLS INTERFACE s_axilite port=srcVImg bundle=control
#pragma HLS INTERFACE s_axilite port=FRAME_WIDTH bundle=control
#pragma HLS INTERFACE s_axilite port=FRAME_HEIGHT bundle=control
#pragma HLS INTERFACE s_axilite port=QP bundle=control
#pragma HLS INTERFACE s_axilite port=BITRATE bundle=control
#pragma HLS INTERFACE s_axilite port=INTRA_PERIOD bundle=control
#pragma HLS INTERFACE s_axilite port=dummy_outRatio bundle=control
#pragma HLS INTERFACE s_axilite port=dummy_delay bundle=control

#pragma HLS INTERFACE s_axilite port=dstImg bundle=control
#pragma HLS INTERFACE s_axilite port=dstRef bundle=control
#pragma HLS INTERFACE s_axilite port=dstNAL_size bundle=control
#pragma HLS INTERFACE s_axilite port=dstDummy_Cnt bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control 


    krnl_datamover_core(srcYImg, srcUImg, srcVImg, FRAME_WIDTH, FRAME_HEIGHT, QP, BITRATE, INTRA_PERIOD, dummy_outRatio, dummy_delay, dstImg, dstRef, dstNAL_size, dstDummy_Cnt);
}
}//extern C

void krnl_datamover_core(
				ap_uint<512>* srcYImg,
				ap_uint<512>* srcUImg,
				ap_uint<512>* srcVImg,
				unsigned int FRAME_WIDTH,
				unsigned int FRAME_HEIGHT,
				unsigned int QP,
				unsigned int BITRATE,
				unsigned int INTRA_PERIOD,
				unsigned int dummy_outRatio,
				unsigned int dummy_delay,				
				ap_uint<512>* dstImg,
				ap_uint<512>* dstRef,				
				ap_uint<64>* dstNAL_size,
				ap_uint<64>* dstDummy_Cnt				
                 )
{
	int outFrameSize = (FRAME_WIDTH * FRAME_HEIGHT)/dummy_outRatio;
	int y_burst_num = (outFrameSize + 63) / 64;
    int c_burst_num = y_burst_num/4;		
    unsigned int delay_10cnt = 0 ;

    ap_uint<512>* dstYImg = dstImg;
    ap_uint<512>* dstUImg = dstYImg + y_burst_num;
    ap_uint<512>* dstVImg = dstUImg + c_burst_num;
	
	*dstNAL_size = (outFrameSize*3)>>1; //420pl 1.5
		
	copy(srcYImg, dstYImg, y_burst_num); //Y
	copy(srcUImg, dstUImg, c_burst_num); //U
	copy(srcVImg, dstVImg, c_burst_num); //V
	
	//Add delay to increase kernel time to mimic reasonable encoder time
	for ( unsigned int i=0;i<dummy_delay; i++)
	{
		if (delay_10cnt < (1<<30))	delay_10cnt = delay_10cnt + 10;					
		else delay_10cnt = 0;
			
	}
	*dstDummy_Cnt = delay_10cnt;
}

void funcRead(ap_uint<512>* srcImg, hls::stream<ap_uint<512> > &data_fifo, int burst_num) {
	int i = 0;
	ap_uint<512> data_in;
    //Read data into FIFO
    for ( i = 0; i < burst_num; i++) {
#pragma HLS pipeline II=1
		data_in = srcImg[i];
		data_fifo << data_in;
	}	
}

void funcWrite(hls::stream<ap_uint<512> > &data_fifo, ap_uint<512>* dstImg, int burst_num) {
	int j = 0;
	ap_uint<512> data_out;
	//Write data from FIFO
	for ( j=0; j < burst_num; j++) {
#pragma HLS pipeline II=1
		data_fifo >> data_out;
		dstImg[j] =  data_out;		
	}
}

void copy(ap_uint<512>*srcImg, ap_uint<512>*dstImg,int burst_num)
{
    hls::stream<ap_uint<512> >  data_fifo;
#pragma HLS stream depth=128 variable=data_fifo
#pragma HLS DATAFLOW
    //Read data into FIFO
    funcRead(srcImg, data_fifo, burst_num);

    //Write data from FIFO
    funcWrite(data_fifo, dstImg, burst_num);

}	

