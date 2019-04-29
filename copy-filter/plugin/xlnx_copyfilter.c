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
#include <string.h>
#include "xlnx_copyfilter.h"
#include "krnl_copyfilter_hw.h"

static int32_t xma_copyfilter_init(XmaFilterSession *sess)
{
    XmaCopyfilterCtx *ctx = sess->base.plugin_data;
    int32_t i;

    if (sess->props.input.format != XMA_YUV420_FMT_TYPE)
        return -1;

    ctx->width = sess->props.input.width;
    ctx->height = sess->props.input.height;
    
    unsigned int input_size = (ctx->width) * (ctx->height);
    unsigned int output_size = (ctx->width) * (ctx->height);

    /* Allocate input and output device buffers for processing YUV420 frame data */
    for (i = 0; i < NUM_BUFFERS; i++)
    {
        ctx->copyfilter.input_y_buffer[i].b_size = input_size;
        ctx->copyfilter.input_y_buffer[i].b_handle = xma_plg_buffer_alloc(sess->base.hw_session, 
                                                                          ctx->copyfilter.input_y_buffer[i].b_size);
        ctx->copyfilter.input_y_buffer[i].paddr = xma_plg_get_paddr(sess->base.hw_session, 
                                                                    ctx->copyfilter.input_y_buffer[i].b_handle);                                                                   

        ctx->copyfilter.input_u_buffer[i].b_size = input_size >> 2;
        ctx->copyfilter.input_u_buffer[i].b_handle = xma_plg_buffer_alloc(sess->base.hw_session, 
                                                                          ctx->copyfilter.input_u_buffer[i].b_size);
        ctx->copyfilter.input_u_buffer[i].paddr = xma_plg_get_paddr(sess->base.hw_session, 
                                                                    ctx->copyfilter.input_u_buffer[i].b_handle);

        ctx->copyfilter.input_v_buffer[i].b_size = input_size >> 2;
        ctx->copyfilter.input_v_buffer[i].b_handle = xma_plg_buffer_alloc(sess->base.hw_session, 
                                                                          ctx->copyfilter.input_v_buffer[i].b_size);
        ctx->copyfilter.input_v_buffer[i].paddr = xma_plg_get_paddr(sess->base.hw_session, 
                                                                    ctx->copyfilter.input_v_buffer[i].b_handle);

        ctx->copyfilter.output_y_buffer[i].b_size = output_size;
        ctx->copyfilter.output_y_buffer[i].b_handle = xma_plg_buffer_alloc(sess->base.hw_session, 
                                                                         ctx->copyfilter.output_y_buffer[i].b_size);
        ctx->copyfilter.output_y_buffer[i].paddr = xma_plg_get_paddr(sess->base.hw_session, 
                                                                         ctx->copyfilter.output_y_buffer[i].b_handle);  

        ctx->copyfilter.output_u_buffer[i].b_size = output_size >> 2;
        ctx->copyfilter.output_u_buffer[i].b_handle = xma_plg_buffer_alloc(sess->base.hw_session, 
                                                                         ctx->copyfilter.output_u_buffer[i].b_size);
        ctx->copyfilter.output_u_buffer[i].paddr = xma_plg_get_paddr(sess->base.hw_session, 
                                                                         ctx->copyfilter.output_u_buffer[i].b_handle);                                                               

        ctx->copyfilter.output_v_buffer[i].b_size = output_size >> 2;
        ctx->copyfilter.output_v_buffer[i].b_handle = xma_plg_buffer_alloc(sess->base.hw_session, 
                                                                         ctx->copyfilter.output_v_buffer[i].b_size);
        ctx->copyfilter.output_v_buffer[i].paddr = xma_plg_get_paddr(sess->base.hw_session, 
                                                                         ctx->copyfilter.output_v_buffer[i].b_handle);
    }
    xma_logmsg(XMA_DEBUG_LOG, XMA_COPYFILTER_PLUGIN, "Init: Buffer allocation complete...\n");

    return 0;
}

static int32_t xma_copyfilter_send_frame(XmaFilterSession *sess, XmaFrame *frame)
{
    XmaCopyfilterCtx *ctx = sess->base.plugin_data;
    XmaHwSession hw_sess = sess->base.hw_session;
    uint32_t nb = 0, axi_ctrl =0;
    int32_t ret;
    
    xma_plg_register_prep_write(hw_sess, &(ctx->width), sizeof(uint32_t), ADDR_FRAME_WIDTH_DATA);
    xma_plg_register_prep_write(hw_sess, &(ctx->height), sizeof(uint32_t), ADDR_FRAME_HEIGHT_DATA);
    xma_plg_register_prep_write(hw_sess, &(ctx->copyfilter.input_y_buffer[nb].paddr), sizeof(uint64_t), ADDR_SRCYIMG_V_DATA);
    xma_plg_register_prep_write(hw_sess, &(ctx->copyfilter.input_u_buffer[nb].paddr), sizeof(uint64_t), ADDR_SRCUIMG_V_DATA);
    xma_plg_register_prep_write(hw_sess, &(ctx->copyfilter.input_v_buffer[nb].paddr), sizeof(uint64_t), ADDR_SRCVIMG_V_DATA);
    xma_plg_register_prep_write(hw_sess, &(ctx->copyfilter.output_y_buffer[nb].paddr), sizeof(uint64_t), ADDR_DSTYIMG_V_DATA);
    xma_plg_register_prep_write(hw_sess, &(ctx->copyfilter.output_u_buffer[nb].paddr), sizeof(uint64_t), ADDR_DSTUIMG_V_DATA);
    xma_plg_register_prep_write(hw_sess, &(ctx->copyfilter.output_v_buffer[nb].paddr), sizeof(uint64_t), ADDR_DSTVIMG_V_DATA);

    xma_plg_buffer_write(hw_sess,
            ctx->copyfilter.input_y_buffer[nb].b_handle,
            frame->data[0].buffer,
            ctx->copyfilter.input_y_buffer[nb].b_size, 0);
            
    xma_plg_buffer_write(hw_sess,
            ctx->copyfilter.input_u_buffer[nb].b_handle,
            frame->data[1].buffer,
            ctx->copyfilter.input_u_buffer[nb].b_size, 0);
    
    xma_plg_buffer_write(hw_sess,
            ctx->copyfilter.input_v_buffer[nb].b_handle,
            frame->data[2].buffer,
            ctx->copyfilter.input_v_buffer[nb].b_size, 0);

    /* Start copy filter kernel */
    xma_plg_schedule_work_item(hw_sess);

    ctx->n_frame++;
    ctx->in_frame++;
    
    return 0;
}

static int32_t xma_copyfilter_recv_frame(XmaFilterSession *sess, XmaFrame *frame)
{
    XmaCopyfilterCtx *ctx = sess->base.plugin_data;
    XmaHwSession hw_sess = sess->base.hw_session;
    
    uint32_t nb = 0, axi_ctrl =0;
    
    
    xma_plg_is_work_item_done(hw_sess, 1000);

    xma_plg_buffer_read(hw_sess,
            ctx->copyfilter.output_y_buffer[nb].b_handle,
            frame->data[0].buffer,
            ctx->copyfilter.output_y_buffer[nb].b_size, 0);
    
    xma_plg_buffer_read(hw_sess,
            ctx->copyfilter.output_u_buffer[nb].b_handle,
            frame->data[1].buffer,
            ctx->copyfilter.output_u_buffer[nb].b_size, 0);
            
    xma_plg_buffer_read(hw_sess,
            ctx->copyfilter.output_v_buffer[nb].b_handle,
            frame->data[2].buffer,
            ctx->copyfilter.output_v_buffer[nb].b_size, 0);
    
    ctx->out_frame++;
    
    return 0;
}

static int32_t xma_copyfilter_close(XmaFilterSession *sess)
{
    XmaCopyfilterCtx *ctx = sess->base.plugin_data;
    XmaHwSession hw_sess = sess->base.hw_session;
    
    for (int i = 0; i < NUM_BUFFERS; i++)
    {
        xma_plg_buffer_free(hw_sess, ctx->copyfilter.input_y_buffer[i].b_handle);
        xma_plg_buffer_free(hw_sess, ctx->copyfilter.input_u_buffer[i].b_handle);
        xma_plg_buffer_free(hw_sess, ctx->copyfilter.input_v_buffer[i].b_handle);
        xma_plg_buffer_free(hw_sess, ctx->copyfilter.output_y_buffer[i].b_handle);  
        xma_plg_buffer_free(hw_sess, ctx->copyfilter.output_u_buffer[i].b_handle);  
        xma_plg_buffer_free(hw_sess, ctx->copyfilter.output_v_buffer[i].b_handle);          
    }
    
    xma_logmsg(XMA_DEBUG_LOG, XMA_COPYFILTER_PLUGIN, "Released copy filter resources!\n");
    return 0;
}

XmaFilterPlugin filter_plugin = {
    .hwfilter_type = XMA_2D_FILTER_TYPE,
    .hwvendor_string = "Xilinx",
    .plugin_data_size = sizeof(XmaCopyfilterCtx),
    .init = xma_copyfilter_init,
    .send_frame = xma_copyfilter_send_frame,
    .recv_frame = xma_copyfilter_recv_frame,
    .close = xma_copyfilter_close,
};
