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
#include <pthread.h>
#include <xma.h>
#include <xmaplugin.h>
#include "krnl_datamover_hw.h"

#define XLNX_DATAMOVER_MAX_CHAN_CAP 1920 * 1080 *60 *8UL
#define NUM_BUFFERS              2
#define XMA_ERROR_EOF            0

#ifdef __cplusplus
extern "C"{
#endif


typedef struct XmaEncoderBuffers
{
    XmaBufferObj     input_y_buffer[NUM_BUFFERS];
    XmaBufferObj     input_u_buffer[NUM_BUFFERS];
    XmaBufferObj     input_v_buffer[NUM_BUFFERS];
    XmaBufferObj     output_buffer[NUM_BUFFERS];
    XmaBufferObj     ref_buffer[NUM_BUFFERS];
    XmaBufferObj     output_len[NUM_BUFFERS];
    XmaBufferObj     dummy_count[NUM_BUFFERS];
} XmaEncoderBuffers;

typedef struct DatamoverContext {
    uint32_t            width;
    uint32_t            height;
    uint32_t            fixed_qp;
    uint32_t            bitrate;
    uint32_t            intra_period;
    uint32_t            dummy_outratio;
    uint32_t            dummy_delay;
    uint32_t            in_frame;
    uint32_t            out_frame;
    uint32_t            n_frame;
    uint8_t             regmap[REGMAP_SIZE];
    XmaEncoderBuffers   encoder;	
} DatamoverContext;

#ifdef __cplusplus
}
#endif // __cplusplus
