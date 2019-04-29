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
// ==============================================================
// File generated on Thu Feb 14 11:34:14 PST 2019
// Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC v2018.3 (64-bit)
// SW Build 2449176 on Tue Feb 12 10:30:59 MST 2019
// IP Build 2407534 on Tue Feb 12 14:00:30 MST 2019
// Copyright 1986-2018 Xilinx, Inc. All Rights Reserved.
// ==============================================================
// control
// 0x00 : Control signals
//        bit 0  - ap_start (Read/Write/COH)
//        bit 1  - ap_done (Read/COR)
//        bit 2  - ap_idle (Read)
//        bit 3  - ap_ready (Read)
//        bit 7  - auto_restart (Read/Write)
//        others - reserved
// 0x04 : Global Interrupt Enable Register
//        bit 0  - Global Interrupt Enable (Read/Write)
//        others - reserved
// 0x08 : IP Interrupt Enable Register (Read/Write)
//        bit 0  - Channel 0 (ap_done)
//        bit 1  - Channel 1 (ap_ready)
//        others - reserved
// 0x0c : IP Interrupt Status Register (Read/TOW)
//        bit 0  - Channel 0 (ap_done)
//        bit 1  - Channel 1 (ap_ready)
//        others - reserved
// 0x10 : Data signal of srcYImg_V
//        bit 31~0 - srcYImg_V[31:0] (Read/Write)
// 0x14 : Data signal of srcYImg_V
//        bit 31~0 - srcYImg_V[63:32] (Read/Write)
// 0x18 : reserved
// 0x1c : Data signal of srcUImg_V
//        bit 31~0 - srcUImg_V[31:0] (Read/Write)
// 0x20 : Data signal of srcUImg_V
//        bit 31~0 - srcUImg_V[63:32] (Read/Write)
// 0x24 : reserved
// 0x28 : Data signal of srcVImg_V
//        bit 31~0 - srcVImg_V[31:0] (Read/Write)
// 0x2c : Data signal of srcVImg_V
//        bit 31~0 - srcVImg_V[63:32] (Read/Write)
// 0x30 : reserved
// 0x34 : Data signal of FRAME_WIDTH
//        bit 31~0 - FRAME_WIDTH[31:0] (Read/Write)
// 0x38 : reserved
// 0x3c : Data signal of FRAME_HEIGHT
//        bit 31~0 - FRAME_HEIGHT[31:0] (Read/Write)
// 0x40 : reserved
// 0x44 : Data signal of dstYImg_V
//        bit 31~0 - dstYImg_V[31:0] (Read/Write)
// 0x48 : Data signal of dstYImg_V
//        bit 31~0 - dstYImg_V[63:32] (Read/Write)
// 0x4c : reserved
// 0x50 : Data signal of dstUImg_V
//        bit 31~0 - dstUImg_V[31:0] (Read/Write)
// 0x54 : Data signal of dstUImg_V
//        bit 31~0 - dstUImg_V[63:32] (Read/Write)
// 0x58 : reserved
// 0x5c : Data signal of dstVImg_V
//        bit 31~0 - dstVImg_V[31:0] (Read/Write)
// 0x60 : Data signal of dstVImg_V
//        bit 31~0 - dstVImg_V[63:32] (Read/Write)
// 0x64 : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define ADDR_AP_CTRL           0x00
#define ADDR_GIE               0x04
#define ADDR_IER               0x08
#define ADDR_ISR               0x0c
#define ADDR_SRCYIMG_V_DATA    0x10
#define BITS_SRCYIMG_V_DATA    64
#define ADDR_SRCUIMG_V_DATA    0x1c
#define BITS_SRCUIMG_V_DATA    64
#define ADDR_SRCVIMG_V_DATA    0x28
#define BITS_SRCVIMG_V_DATA    64
#define ADDR_FRAME_WIDTH_DATA  0x34
#define BITS_FRAME_WIDTH_DATA  32
#define ADDR_FRAME_HEIGHT_DATA 0x3c
#define BITS_FRAME_HEIGHT_DATA 32
#define ADDR_DSTYIMG_V_DATA    0x44
#define BITS_DSTYIMG_V_DATA    64
#define ADDR_DSTUIMG_V_DATA    0x50
#define BITS_DSTUIMG_V_DATA    64
#define ADDR_DSTVIMG_V_DATA    0x5c
#define BITS_DSTVIMG_V_DATA    64

