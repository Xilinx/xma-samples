/**********
Copyright (c) 2018, Xilinx, Inc.
All rights reserved.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**********/
#ifndef __SYNTHESIS__
#include <stdio.h>
#endif
#include <stdio.h>
#include <assert.h>
#include "ap_int.h"
#include "hls_video.h"

void krnl_copyfilter_core(
				ap_uint<512>* srcYImg,
				ap_uint<512>* srcUImg,
				ap_uint<512>* srcVImg,
				unsigned int FRAME_WIDTH,
				unsigned int FRAME_HEIGHT,			
				ap_uint<512>* dstYImg,
				ap_uint<512>* dstUImg,
				ap_uint<512>* dstVImg				
                 );
				 
void copy(ap_uint<512>*srcImg, ap_uint<512>*dstImg, int burst_num, unsigned int width, unsigned int height);	
void copy_core(hls::stream<unsigned char>& pix_fifo_in, hls::stream<unsigned char>& pix_fifo_out, unsigned int width, unsigned int height);			 

extern "C" {
void krnl_copyfilter(
				ap_uint<512>* srcYImg,
				ap_uint<512>* srcUImg,
				ap_uint<512>* srcVImg,
				unsigned int FRAME_WIDTH,
				unsigned int FRAME_HEIGHT,			
				ap_uint<512>* dstYImg,
				ap_uint<512>* dstUImg,
				ap_uint<512>* dstVImg			
                 )
{

#pragma HLS INTERFACE m_axi port=srcYImg offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=srcUImg offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=srcVImg offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=dstYImg offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=dstUImg offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=dstVImg offset=slave bundle=gmem

#pragma HLS INTERFACE s_axilite port=srcYImg bundle=control
#pragma HLS INTERFACE s_axilite port=srcUImg bundle=control
#pragma HLS INTERFACE s_axilite port=srcVImg bundle=control
#pragma HLS INTERFACE s_axilite port=FRAME_WIDTH bundle=control
#pragma HLS INTERFACE s_axilite port=FRAME_HEIGHT bundle=control

#pragma HLS INTERFACE s_axilite port=dstYImg bundle=control
#pragma HLS INTERFACE s_axilite port=dstUImg bundle=control
#pragma HLS INTERFACE s_axilite port=dstVImg bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control 

    krnl_copyfilter_core(srcYImg, srcUImg, srcVImg, FRAME_WIDTH, FRAME_HEIGHT, dstYImg, dstUImg, dstVImg);
}
}//extern C

void krnl_copyfilter_core(
				ap_uint<512>* srcYImg,
				ap_uint<512>* srcUImg,
				ap_uint<512>* srcVImg,
				unsigned int FRAME_WIDTH,
				unsigned int FRAME_HEIGHT,		
				ap_uint<512>* dstYImg,
				ap_uint<512>* dstUImg,
				ap_uint<512>* dstVImg					
                 )
{
	int outFrameSize = (FRAME_WIDTH * FRAME_HEIGHT);
	int y_burst_num = (outFrameSize + 63) / 64;
    int c_burst_num = y_burst_num/4;
	
		
	copy(srcYImg, dstYImg, y_burst_num, FRAME_WIDTH, FRAME_HEIGHT); //Y
	copy(srcUImg, dstUImg, c_burst_num, FRAME_WIDTH>>1, FRAME_HEIGHT>>1); //U
	copy(srcVImg, dstVImg, c_burst_num, FRAME_WIDTH>>1, FRAME_HEIGHT>>1); //V

}

void copy(ap_uint<512>*srcImg, ap_uint<512>*dstImg, int burst_num, unsigned int width, unsigned int height)
{
    hls::stream<ap_uint<512> >    data_fifo_in;
#pragma HLS stream depth=128 variable=data_fifo_in

    hls::stream<ap_uint<512> >    data_fifo_out;
#pragma HLS stream depth=128 variable=data_fifo_out

	hls::stream<unsigned char>    pix_fifo_in;
#pragma HLS stream depth=128 variable=pix_fifo_in

	hls::stream<unsigned char>    pix_fifo_out;
#pragma HLS stream depth=128 variable=pix_fifo_out
	
	ap_uint<512> data_in;
    ap_uint<512> data_out;
	int i = 0;
    int j = 0;
	
#pragma HLS DATAFLOW
    // Read data into FIFO
    for ( i=0; i < burst_num; i++) {
#pragma HLS pipeline II=1
		data_in = srcImg[i];
		data_fifo_in << data_in;
	}	
	
	// Convert 512 bit data into pixels
    for ( j=0; j < burst_num; j++) {
		data_fifo_in >> data_in;
		for (i=0; i<64; i++) {
#pragma HLS pipeline II=1		
            unsigned char pix = (unsigned char)data_in(7+8*i,8*i); // Chop up 8 bits at a time
			pix_fifo_in << pix;
		}
	}
	
	// Call pixel processing function
	copy_core(pix_fifo_in, pix_fifo_out, width, height);
	
	// Convert pixels into 512 bit data
    for ( j=0; j < burst_num; j++) {
		for (i=0; i<64; i++) {
#pragma HLS pipeline II=1
			unsigned char pix;
			pix_fifo_out >> pix;
			data_out(7+8*i,8*i) = pix; // Assemble 8 bits at a time
		}
		data_fifo_out << data_out;
	}
	
    // Write data from FIFO
    for ( j=0; j < burst_num; j++) {
#pragma HLS pipeline II=1
		data_fifo_out >> data_out;
		dstImg[j] = data_out;		
	}	
}	

void copy_core(hls::stream<unsigned char>& pix_fifo_in, hls::stream<unsigned char>& pix_fifo_out, unsigned int width, unsigned int height)
{
	// Pixel processing function
    for (unsigned int j=0; j < height; j++) {
		for (unsigned int i=0; i < width; i++) {
#pragma HLS pipeline II=1
           unsigned char pix_in;
           unsigned char pix_out;
           pix_fifo_in >> pix_in;
		   
		   pix_out = pix_in; // copy
		   
		   pix_fifo_out << pix_out;
		}
	}
}
