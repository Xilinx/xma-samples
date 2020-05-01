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
#include <pthread.h>
#include <xma.h>
#include <time.h>

typedef struct thread_data
{
    XmaEncoderSession *enc_session;
    XmaEncoderProperties enc_props;
    const char *input_filename;
    const char *output_filename;
    int32_t num_frames;
} thread_data;

void *thread(void *ptr)
{
    struct thread_data *data = (thread_data*)ptr;
    XmaEncoderSession *enc_session;
    int32_t num_frames;
    int64_t rc;
    
    // File IO for copy encoder
    FILE *fpin,*fpout = NULL;

    fpin = fopen(data->input_filename,"rb");
    fpout = fopen(data->output_filename,"wb");
    
    if (fpin == NULL) 
    {
        printf("Failed to open input file, exiting...\n");
        return (void *)-1;
    }

    if (fpout == NULL) 
    {
        printf("Failed to open output file, exiting...\n");
        return (void *)-1;
    }

    fseek(fpin, 0L, SEEK_SET);
    num_frames = data->num_frames;
    enc_session = data->enc_session;

    // Create an input frame for copy encoder
    XmaFrameProperties fprops;
    fprops.format = XMA_YUV420_FMT_TYPE;
    fprops.width = 1920;
    fprops.height = 1080;
    fprops.bits_per_pixel = 8;
    XmaFrame *input_frame = xma_frame_alloc(&fprops, false);

    // Create data buffer for copy encoder
    XmaDataBuffer *buffer = xma_data_buffer_alloc((fprops.width)*(fprops.height)*1.5, false);
    rc = 0;

    // Run copy encoder for num_frames
    int i=0;
    for (i = 0; i < num_frames; i++)
    {
        // Read raw video input
        int32_t data_size = 0;

        rc = fread(input_frame->data[0].buffer, sizeof(uint8_t), (fprops.width*fprops.height), fpin);
        if (rc <= 0)
            return (void *)1;
        rc = fread(input_frame->data[1].buffer, sizeof(uint8_t), ((fprops.width*fprops.height)/4), fpin);
        if (rc <= 0)
            return (void *)1;
        rc = fread(input_frame->data[2].buffer, sizeof(uint8_t), ((fprops.width*fprops.height)/4), fpin);
        if (rc <= 0)
            return (void *)1;
        
        // Send input frame to copy encoder
        rc = xma_enc_session_send_frame(enc_session, input_frame);
        if (rc == XMA_SEND_MORE_DATA) {
            continue;
        }
        else if (rc != 0) {
            printf("Failed to send input frame to encoder\n");
            return (void *)rc;
        }

        // Receive data from copy encoder
        rc = xma_enc_session_recv_data(enc_session, buffer, &data_size);
        if (data_size)
        {
            // printf("Frame %d: Writing %d bytes to output file\n", i, data_size);
            fwrite(buffer->data.buffer, sizeof(uint8_t), data_size, fpout);
        }

        //NULL frame sent to read last frame output from device
        if(i==num_frames-1) {
            XmaFrame *input_null = xma_frame_alloc(&fprops, true);
            rc = xma_enc_session_send_frame(enc_session, input_null);
            rc = xma_enc_session_recv_data(enc_session, buffer, &data_size);
            xma_frame_free(input_null);
            if (data_size) {
                fwrite(buffer->data.buffer, sizeof(uint8_t), data_size, fpout);
            }
        }
    }



    // Clean up
    fclose(fpin);
    fclose(fpout);
    xma_frame_free(input_frame);
    xma_data_buffer_free(buffer);

    return NULL;
}

int main(int argc, char *argv[])
{
    int32_t rc, i, thread_cnt, num_frames;
    pthread_t thread_inst[8];
    struct thread_data thread_data[8];

    if (argc < 5 || argc > 6)
    {
        printf("Usage:\n");
        printf("   datamover_app: <input 1> <input 2> <input 3> <number of frames> [thread_count]\n");
        printf("   thread count = 1 to 3 (max)  [default: 1] \n");
        printf("   ./datamover_app input1.yuv input2.yuv input3.yuv 500 6\n");
        return -1;
    }

    XmaXclbinParameter tmp_xclbin_param;
    tmp_xclbin_param.device_id = 0;
    tmp_xclbin_param.xclbin_name = "../kernel/xclbin/fpga.3k.hw.xilinx_u200_qdma_201920_1.xclbin";

    rc = xma_initialize(&tmp_xclbin_param, 1);
    if (rc != 0) {
        return -1;
    }


    char *in_file[] = {argv[1], argv[2], argv[3]};
    char *out_file[] = {"output_1920x1080p60_1.yuv", "output_1920x1080p60_2.yuv", "output_1920x1080p60_3.yuv"};

    num_frames = atoi(argv[4]);
    thread_cnt = 1;
    if (argv[5])
        thread_cnt = atoi(argv[5]);
    if (thread_cnt > 3) {
        printf("WARNING:  Using max thread count of 3\n");
        printf("WARNING:  xclbin only has 3 CUs\n");
        thread_cnt = 3;
    }
    if (thread_cnt < 1) {
        thread_cnt = 1;
    }

    // Setup copy encoder properties
    XmaEncoderProperties enc_props;
    memset(&enc_props, 0, sizeof(enc_props));
    enc_props.hwencoder_type = XMA_COPY_ENCODER_TYPE;
    strcpy(enc_props.hwvendor_string, "Xilinx");
    enc_props.format = XMA_YUV420_FMT_TYPE;
    enc_props.bits_per_pixel = 8;
    enc_props.width = 1920;
    enc_props.height = 1080;
    enc_props.framerate.numerator = 60;
    enc_props.framerate.denominator = 1;
    enc_props.bitrate = 0;
    enc_props.qp = 0;
    enc_props.gop_size = 0;
    enc_props.idr_interval = 0;

    enc_props.plugin_lib = "../plugin/libxlnxdatamover.so";
    enc_props.dev_index = 0;
    enc_props.ddr_bank_index = -1;//XMA to select the ddr bank based on xclbin meta data

    // Create copy encoder sessions based on requested properties
    for (i = 0; i < thread_cnt; i++) {
        thread_data[i].input_filename = in_file[i];
        thread_data[i].num_frames = num_frames;
        thread_data[i].output_filename = out_file[i];

        enc_props.cu_index = i;//xclbin has only 3 CUs
        enc_props.channel_id = i;

        thread_data[i].enc_session = xma_enc_session_create(&enc_props);
        if (!thread_data[i].enc_session)
            printf("Failed to create copy encoder session #%d\n", i);
        thread_data[i].enc_props = enc_props;
    }

    for (i = 0; i < thread_cnt; i++) {
        if (thread_data[i].enc_session)
            pthread_create(&thread_inst[i], NULL, thread, &thread_data[i]);
    }

    for (i = 0; i < thread_cnt; i++) {
        if (thread_data[i].enc_session) {
            pthread_join(thread_inst[i], NULL);
            rc = xma_enc_session_destroy(thread_data[i].enc_session);
            if (rc != 0) {
                printf("Failed to destroy encoder session\n");
                return rc;              
            }    
        }
    }

    return 0;
    
}
