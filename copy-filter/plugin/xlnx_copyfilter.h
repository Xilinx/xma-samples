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
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <xma.h>
#include <xmaplugin.h>

#define XLNX_COPYFILTER_MAX_CHAN_CAP 1920 * 1080 *60 *8UL
#define XMA_AXI_IDLE                 0x4
#define XMA_AXI_START                0x1
#define NUM_BUFFERS                  1
#define XMA_COPYFILTER_PLUGIN        "xma_copyfilter_plugin"

#ifdef __cplusplus
extern "C"{
#endif

typedef struct XmaBufferStruct
{
    XmaBufferHandle b_handle;
    uint64_t        paddr;
    uint64_t        b_size;
} XmaBufferStruct;

typedef struct XmaCopyFilterBuffers
{
    XmaBufferStruct     input_y_buffer[NUM_BUFFERS];
    XmaBufferStruct     input_u_buffer[NUM_BUFFERS];
    XmaBufferStruct     input_v_buffer[NUM_BUFFERS];
    XmaBufferStruct     output_y_buffer[NUM_BUFFERS];
    XmaBufferStruct     output_u_buffer[NUM_BUFFERS];
    XmaBufferStruct     output_v_buffer[NUM_BUFFERS];
} XmaCopyFilterBuffers;

typedef struct XmaCopyfilterCtx 
{
    XmaCopyFilterBuffers copyfilter;
    int32_t width;
    int32_t height;
    uint32_t in_frame;
    uint32_t out_frame;
    uint32_t n_frame;
} XmaCopyfilterCtx;

#ifdef __cplusplus
}
#endif // __cplusplus
