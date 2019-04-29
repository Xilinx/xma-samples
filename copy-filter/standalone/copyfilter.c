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
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <xma.h>
#include <time.h>

#define CH 1
#define XMA_COPYFILTER_APP "xma_copyfilter_app"
#define ENABLE_DEBUG 0

int main(int argc, char *argv[])
{
    int32_t rc;

    if (argc != 4)
    {
        printf("Usage:\n");
        printf("   copyfilter_app: <Configuration file name> <input file 1> <number of frames>\n");
        printf("   ./copyfilter_app copyfilter_cfg.yaml input_1920x1080p.yuv 10\n");
        return -1;
    }

    rc = xma_initialize(argv[1]);
    if (rc != 0)
    {
        xma_logmsg(XMA_ERROR_LOG, XMA_COPYFILTER_APP, "XMA Initialization failed\n");
        return -1;
    }

    const char *in_file[CH] = {argv[2]};
    const char *out_file[CH] = {"output_1920x1080p_1.yuv"};
    unsigned int num_frames = atoi(argv[3]);
    unsigned int outsize[3] = {0};

    // File IO for copy filter
    FILE *fpin[CH],*fpout[CH] = {NULL};
    
    for (int f = 0; f < CH; f++)
    {
        fpin[f] = fopen(in_file[f],"rb");
        fpout[f] = fopen(out_file[f],"wb");
        if (fpin[f] == NULL) 
        {
            xma_logmsg(XMA_ERROR_LOG, XMA_COPYFILTER_APP, "Failed to open input file %d, exiting...\n", f);
            return -1;
        }
    
        if (fpout[f] == NULL) 
        {
            xma_logmsg(XMA_ERROR_LOG, XMA_COPYFILTER_APP, "Failed to open output file %d, exiting...\n", f);
            return -1;
        }
        fseek(fpin[f], 0L, SEEK_SET);
    }

#if (ENABLE_DEBUG==1)    
    FILE *fpdo;       
    const char* debugOutFileName = "Output_yuv_data.txt";
    fpdo = fopen(debugOutFileName, "w");
    if (fpdo == NULL)
    {
        xma_logmsg(XMA_ERROR_LOG, XMA_COPYFILTER_APP, "Failed to open output file %d, exiting...\n", debugOutFileName);
        return -1;
    }   
#endif 

    // Setup copy filter input port properties
    XmaFilterPortProperties in_props;
    memset(&in_props, 0, sizeof(in_props));
    in_props.format = XMA_YUV420_FMT_TYPE; 
    in_props.bits_per_pixel = 8;
    in_props.width = 1920; 
    in_props.height = 1080;
    
    // Setup copy filter output port properties
    XmaFilterPortProperties out_props;
    memset(&out_props, 0, sizeof(out_props));
    out_props.format = XMA_YUV420_FMT_TYPE; 
    out_props.bits_per_pixel = 8;
    out_props.width = 1920; 
    out_props.height = 1080;
    
    outsize[0] = out_props.width * out_props.height;
    outsize[1] = outsize[0]/4;
    outsize[2] = outsize[0]/4;
    
    // Setup copy filter properties
    XmaFilterProperties filter_props;
    memset(&filter_props, 0, sizeof(filter_props));
    filter_props.hwfilter_type = XMA_2D_FILTER_TYPE;
    strcpy(filter_props.hwvendor_string, "Xilinx");
    filter_props.input = in_props;
    filter_props.output = out_props;

    // Create copy filter session based on the requested properties
    XmaFilterSession *filter_session[CH];
    
    // Create an input frame for copy filter
    XmaFrameProperties fprops;
    fprops.format = XMA_YUV420_FMT_TYPE;
    fprops.width = in_props.width;
    fprops.height = in_props.height;
    fprops.bits_per_pixel = 8;
    XmaFrame *input_frame[CH];
    XmaFrame *output_frame[CH];

    // Create data buffer for copy filter
    for (int s = 0; s < CH; s++)
    {
        filter_session[s] = xma_filter_session_create(&filter_props);
        if (!filter_session[s])
        {
            xma_logmsg(XMA_ERROR_LOG, XMA_COPYFILTER_APP, "Failed to create filter session %d\n", s);
            return -1;
        }
        input_frame[s] = xma_frame_alloc(&fprops);
        output_frame[s] = xma_frame_alloc(&fprops);
    }

    // Run copy filter for num_frames
    for (int i = 0; i < num_frames; i++)
    {
        for (int j = 0; j < CH; j++)
        {
            // Read raw video input
            int32_t rc;
            
            rc = fread(input_frame[j]->data[0].buffer, sizeof(uint8_t), (in_props.width*in_props.height), fpin[j]);
            if (rc <= 0)
                return 1;
            rc = fread(input_frame[j]->data[1].buffer, sizeof(uint8_t), ((in_props.width*in_props.height)/4), fpin[j]);
            if (rc <= 0)
                return 1;
            rc = fread(input_frame[j]->data[2].buffer, sizeof(uint8_t), ((in_props.width*in_props.height)/4), fpin[j]);
            if (rc <= 0)
                return 1;

            // Send input frame to copy filter
            rc = xma_filter_session_send_frame(filter_session[j], input_frame[j]);
            if (rc == -2)
            {
                continue;
            }
            else if (rc != 0)
            {
                xma_logmsg(XMA_ERROR_LOG, XMA_COPYFILTER_APP, "Failed to send input frame to filter\n");
                return rc;
            }

            // Receive data from copy filter
            rc = xma_filter_session_recv_frame(filter_session[j], output_frame[j]);
            
			// Write raw video output to file
            fwrite(output_frame[j]->data[0].buffer, sizeof(uint8_t), (out_props.width*out_props.height), fpout[j]);
            fwrite(output_frame[j]->data[1].buffer, sizeof(uint8_t), ((out_props.width*out_props.height)/4), fpout[j]);
            fwrite(output_frame[j]->data[2].buffer, sizeof(uint8_t), ((out_props.width*out_props.height)/4), fpout[j]);

#if (ENABLE_DEBUG==1)       
            unsigned int x,y,check_status;
            int err_flag = 0;
            // Loop for Y,U and V components
            for (x = 0; x < 3; x++)
            {
                check_status = 0;
                fprintf(fpdo,"frame=%d comp=%d size=%d\n", i, x, outsize[x]);
                fprintf(fpdo," InPixel   OutPixel\n");
                unsigned char *in, *out;
                in = input_frame[j]->data[x].buffer;
                out = output_frame[j]->data[x].buffer;
                
                for (y = 0; y < outsize[x]; y++)
                {
                    fprintf(fpdo,"%3d(0x%02x) %3d(0x%02x) \n", *out, *out, *in, *in);
                    if (*in != *out)
                    {
                        check_status = 1;
                        if (err_flag==0)
                        {
                            xma_logmsg(XMA_ERROR_LOG, XMA_COPYFILTER_APP, "Frame=%d; (%d,%d) in=%d out=%d\n", i, x, y, *in, *out);
                            err_flag =1;
                        }                                            
                    }   
                    in = in+1;
                    out = out+1;
                }
                
                if (check_status)
                {
                    xma_logmsg(XMA_ERROR_LOG, XMA_COPYFILTER_APP, "ERROR: Input and output files donot match...\n");
                }
            }   
#endif          
        }
    }

    // Close copy filter session
    for (int d = 0; d < CH; d++)
    {
        rc = xma_filter_session_destroy(filter_session[d]);
        if (rc != 0)
        {
            xma_logmsg(XMA_ERROR_LOG, XMA_COPYFILTER_APP, "Failed to destroy filter session\n");
            return rc;      
        }
		
		printf("INFO: Test completed successfully.\n");

        // Clean up
        fclose(fpin[d]);
        fclose(fpout[d]);
    }
}
