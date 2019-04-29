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
#include <stdio.h>
#include "xlnx_datamover.h"
#include "krnl_datamover_hw.h"

static int32_t xlnx_datamover_alloc_chan(XmaSession *pending, XmaSession **sessions, uint32_t sess_cnt)
{
   XmaEncoderSession *request;
   XmaEncoderProperties *enc_props;
   unsigned long curr_chan_load = 0;
   unsigned i;
   request = to_xma_encoder(pending);
   for (i = 0; i < sess_cnt; i++) {
      XmaEncoderSession  *curr_sess = to_xma_encoder(sessions[i]);
      XmaEncoderProperties *props = &curr_sess->encoder_props;
      curr_chan_load += (props->framerate.numerator * props->width *
                         props->height * props->bits_per_pixel) /
                         props->framerate.denominator;
   }
   enc_props = &request->encoder_props;
   curr_chan_load += (enc_props->framerate.numerator *
                      enc_props->width *
                      enc_props->height *
                      enc_props->bits_per_pixel) /
                      enc_props->framerate.denominator;
   if (curr_chan_load > XLNX_DATAMOVER_MAX_CHAN_CAP)
       return -1;
   printf("[INFO]: Encoder channel request %u approved for format = %d, bpp = %d, WxH = %d x %d\n",
           sess_cnt, enc_props->format, enc_props->bits_per_pixel, enc_props->width, enc_props->height);
   pending->chan_id = sess_cnt;
   return 0;
}

static int32_t xlnx_datamover_init(XmaEncoderSession *enc_session)
{
    DatamoverContext *ctx = enc_session->base.plugin_data;
    XmaHwSession hw_handle = enc_session->base.hw_session;
    XmaEncoderProperties *enc_props = &enc_session->encoder_props;
	
    ctx->in_frame = 0;
    ctx->out_frame = 0;
    ctx->n_frame = 0;
    ctx->width = enc_props->width;
    ctx->height = enc_props->height;
 
    unsigned int input_size = (ctx->width) * (ctx->height);
    unsigned int output_size = (input_size * 3) >> 1;
    /* Reference frame requires atleast 16MB space in DDR */
    unsigned int ref_size = 16*1024*1024;  

    /* Allocate memory for encoder input and output buffers */
    for (int i = 0; i < NUM_BUFFERS; i++) {
       ctx->encoder.input_y_buffer[i].b_size = input_size;
       ctx->encoder.input_y_buffer[i].b_handle = xma_plg_buffer_alloc(hw_handle, ctx->encoder.input_y_buffer[i].b_size);
       ctx->encoder.input_y_buffer[i].paddr = xma_plg_get_paddr(hw_handle, ctx->encoder.input_y_buffer[i].b_handle);
	
       ctx->encoder.input_u_buffer[i].b_size = input_size >> 2;
       ctx->encoder.input_u_buffer[i].b_handle = xma_plg_buffer_alloc(hw_handle, ctx->encoder.input_u_buffer[i].b_size);
       ctx->encoder.input_u_buffer[i].paddr = xma_plg_get_paddr(hw_handle, ctx->encoder.input_u_buffer[i].b_handle);
	
       ctx->encoder.input_v_buffer[i].b_size = input_size >> 2;
       ctx->encoder.input_v_buffer[i].b_handle = xma_plg_buffer_alloc(hw_handle, ctx->encoder.input_v_buffer[i].b_size);
       ctx->encoder.input_v_buffer[i].paddr = xma_plg_get_paddr(hw_handle, ctx->encoder.input_v_buffer[i].b_handle);
	
       ctx->encoder.output_buffer[i].b_size = output_size;
       ctx->encoder.output_buffer[i].b_handle = xma_plg_buffer_alloc(hw_handle, ctx->encoder.output_buffer[i].b_size);
       ctx->encoder.output_buffer[i].paddr = xma_plg_get_paddr(hw_handle, ctx->encoder.output_buffer[i].b_handle);
	
       ctx->encoder.ref_buffer[i].b_size = ref_size;
       ctx->encoder.ref_buffer[i].b_handle = xma_plg_buffer_alloc(hw_handle, ctx->encoder.ref_buffer[i].b_size);
       ctx->encoder.ref_buffer[i].paddr = xma_plg_get_paddr(hw_handle, ctx->encoder.ref_buffer[i].b_handle);

       ctx->encoder.output_len[i].b_size = sizeof(uint64_t);
       ctx->encoder.output_len[i].b_handle = xma_plg_buffer_alloc(hw_handle, ctx->encoder.output_len[i].b_size);
       ctx->encoder.output_len[i].paddr = xma_plg_get_paddr(hw_handle, ctx->encoder.output_len[i].b_handle);
	
       ctx->encoder.dummy_count[i].b_size = sizeof(uint64_t);
       ctx->encoder.dummy_count[i].b_handle = xma_plg_buffer_alloc(hw_handle, ctx->encoder.dummy_count[i].b_size);
       ctx->encoder.dummy_count[i].paddr = xma_plg_get_paddr(hw_handle, ctx->encoder.dummy_count[i].b_handle);
    }
    //printf("Init: Buffer allocation complete...\n");
	
    /* Set-up encoder parameters */
    ctx->bitrate = 0;
    ctx->fixed_qp = 0;
    int32_t bitrate = enc_props->bitrate;
    int32_t global_quality = enc_props->qp;

    if (bitrate > 0)
       ctx->bitrate = bitrate;    
    else if (global_quality > 0) 
       ctx->fixed_qp = global_quality;

    int32_t gop_size = enc_props->gop_size;
    if (gop_size > 0)
       ctx->intra_period = gop_size;

    ctx->dummy_outratio = 1;
    ctx->dummy_delay = 2000000;
	
    return 0;
}

static int32_t xlnx_datamover_send_frame(XmaEncoderSession *enc_session, XmaFrame *frame)
{
    DatamoverContext *ctx = enc_session->base.plugin_data;
    XmaHwSession hw_handle = enc_session->base.hw_session;
    uint32_t nb = 0;
    nb = ctx->n_frame % NUM_BUFFERS;


    if(frame->data[0].buffer !=NULL) {
       xma_plg_register_prep_write(hw_handle, &(ctx->width), sizeof(uint32_t), ADDR_FRAME_WIDTH_DATA);
       xma_plg_register_prep_write(hw_handle, &(ctx->height), sizeof(uint32_t), ADDR_FRAME_HEIGHT_DATA);
       xma_plg_register_prep_write(hw_handle, &(ctx->fixed_qp), sizeof(uint32_t), ADDR_QP_DATA);
       xma_plg_register_prep_write(hw_handle, &(ctx->bitrate), sizeof(uint32_t), ADDR_BITRATE_DATA);
       xma_plg_register_prep_write(hw_handle, &(ctx->intra_period), sizeof(uint32_t), ADDR_INTRA_PERIOD_DATA);
       xma_plg_register_prep_write(hw_handle, &(ctx->dummy_outratio), sizeof(uint32_t), ADDR_DUMMY_OUTRATIO_DATA);
       xma_plg_register_prep_write(hw_handle, &(ctx->dummy_delay), sizeof(uint32_t), ADDR_DUMMY_DELAY_DATA);
    
       xma_plg_register_prep_write(hw_handle, &(ctx->encoder.input_y_buffer[nb].paddr), sizeof(uint64_t), ADDR_SRCYIMG_V_DATA);
       xma_plg_register_prep_write(hw_handle, &(ctx->encoder.input_u_buffer[nb].paddr), sizeof(uint64_t), ADDR_SRCUIMG_V_DATA);
       xma_plg_register_prep_write(hw_handle, &(ctx->encoder.input_v_buffer[nb].paddr), sizeof(uint64_t), ADDR_SRCVIMG_V_DATA);

       xma_plg_register_prep_write(hw_handle, &(ctx->encoder.output_buffer[nb].paddr), sizeof(uint64_t), ADDR_DSTIMG_V_DATA);
       xma_plg_register_prep_write(hw_handle, &(ctx->encoder.ref_buffer[nb].paddr), sizeof(uint64_t), ADDR_DSTREF_V_DATA);
       xma_plg_register_prep_write(hw_handle, &(ctx->encoder.output_len[nb].paddr), sizeof(uint64_t), ADDR_DSTNAL_SIZE_V_DATA);
       xma_plg_register_prep_write(hw_handle, &(ctx->encoder.dummy_count[nb].paddr), sizeof(uint64_t), ADDR_DSTDUMMY_CNT_V_DATA);   


       xma_plg_buffer_write(hw_handle,
            ctx->encoder.input_y_buffer[nb].b_handle,
            frame->data[0].buffer,
            ctx->encoder.input_y_buffer[nb].b_size, 0);
			 
       xma_plg_buffer_write(hw_handle,
            ctx->encoder.input_u_buffer[nb].b_handle,
            frame->data[1].buffer,
            ctx->encoder.input_u_buffer[nb].b_size, 0);
	
       xma_plg_buffer_write(hw_handle,
            ctx->encoder.input_v_buffer[nb].b_handle,
            frame->data[2].buffer,
            ctx->encoder.input_v_buffer[nb].b_size, 0);		 
    }
	
    xma_plg_schedule_work_item(hw_handle);

    if (ctx->n_frame == 0) {
       ctx->n_frame++;
       ctx->in_frame++;
       return XMA_SEND_MORE_DATA;
    }

    ctx->n_frame++;
    if(frame->data[0].buffer !=NULL)    
	ctx->in_frame++;
    
    return 0;
}

static int32_t xlnx_datamover_recv_data(XmaEncoderSession *enc_session, XmaDataBuffer *data, int32_t *data_size)
{
    DatamoverContext *ctx = enc_session->base.plugin_data;
    XmaHwSession hw_handle = enc_session->base.hw_session;
    int64_t out_size = 0;
    uint64_t d_cnt = 0;
    uint32_t nb = (ctx->n_frame) % NUM_BUFFERS; 

    xma_plg_is_work_item_done(hw_handle, 1000);
	
    /* Read the length of output data */
    xma_plg_buffer_read(hw_handle, ctx->encoder.output_len[nb].b_handle, &out_size, sizeof(out_size), 0);
	
    /* Ensure output frame length is valid */
    if((out_size > 0) && (out_size <= data->alloc_size)) {
       xma_plg_buffer_read(hw_handle, ctx->encoder.output_buffer[nb].b_handle, data->data.buffer, out_size, 0);
       xma_plg_buffer_read(hw_handle, ctx->encoder.dummy_count[nb].b_handle, &d_cnt, sizeof(d_cnt), 0);
       ctx->out_frame++;
       *data_size = out_size;
    }
    else {
       *data_size = 0;
       return -1;
    }
    return 0;
}

static int32_t xlnx_datamover_close(XmaEncoderSession *enc_session)
{
    DatamoverContext *ctx = enc_session->base.plugin_data;
    XmaHwSession hw_handle = enc_session->base.hw_session;
	
    for (int i = 0; i < NUM_BUFFERS; i++) {
       xma_plg_buffer_free(hw_handle, ctx->encoder.input_y_buffer[i].b_handle);
       xma_plg_buffer_free(hw_handle, ctx->encoder.input_u_buffer[i].b_handle);
       xma_plg_buffer_free(hw_handle, ctx->encoder.input_v_buffer[i].b_handle);	
       xma_plg_buffer_free(hw_handle, ctx->encoder.output_buffer[i].b_handle);
       xma_plg_buffer_free(hw_handle, ctx->encoder.ref_buffer[i].b_handle);
       xma_plg_buffer_free(hw_handle, ctx->encoder.output_len[i].b_handle);
       xma_plg_buffer_free(hw_handle, ctx->encoder.dummy_count[i].b_handle);
    }
    printf("Released datamover resources!\n");
    return 0;
}

XmaEncoderPlugin encoder_plugin = {
    .hwencoder_type    = XMA_COPY_ENCODER_TYPE,
    .hwvendor_string   = "Xilinx",
    .format            = XMA_YUV420_FMT_TYPE,
    .bits_per_pixel    = 8,
    .plugin_data_size  = sizeof(DatamoverContext),
    .kernel_data_size  = sizeof(HostKernelCtx),
    .init              = xlnx_datamover_init,
    .send_frame        = xlnx_datamover_send_frame,
    .recv_data         = xlnx_datamover_recv_data,
    .close             = xlnx_datamover_close,
    .alloc_chan        = xlnx_datamover_alloc_chan
};
