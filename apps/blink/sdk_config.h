/**
 * Copyright (c) 2017 - 2021, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#pragma once

#include "id130c_pins.h"

#define NRF_CLOCK_ENABLED 1
#define CLOCK_CONFIG_LF_SRC 1
#define CLOCK_CONFIG_LF_CAL_ENABLED 0
#define CLOCK_CONFIG_IRQ_PRIORITY 6

#define ST7735_ENABLED 1
#define ST7735_SCK_PIN ID130C_PIN_LCD_CLK
#define ST7735_MISO_PIN ID130C_PIN_SPARE
#define ST7735_MOSI_PIN ID130C_PIN_LCD_DATA
#define ST7735_SS_PIN ID130C_PIN_LCD_CS
#define ST7735_IRQ_PRIORITY 6
#define ST7735_SPI_INSTANCE 0
// <0=> INITR_GREENTAB 
// <1=> INITR_REDTAB 
// <2=> INITR_BLACKTAB 
// <3=> INITR_144GREENTAB 
#define ST7735_TAB_COLOR 0
#define ST7735_DC_PIN ID130C_PIN_LCD_DC
#define ST7735_HEIGHT 160
#define ST7735_WIDTH 80

#define NRF_GFX_ENABLED 1

#define SPI_ENABLED 1
#define SPI0_ENABLED 1
#define SPI0_USE_EASY_DMA 1
#define SPI_DEFAULT_CONFIG_IRQ_PRIORITY 6
#define NRFX_SPIM_MISO_PULL_CFG 1

#define NRF_LOG_BACKEND_RTT_ENABLED 1
#define NRF_LOG_BACKEND_RTT_TEMP_BUFFER_SIZE 128
#define NRF_LOG_BACKEND_RTT_TX_RETRY_DELAY_MS 1
#define NRF_LOG_BACKEND_RTT_TX_RETRY_CNT 3
#define NRF_LOG_BACKEND_UART_ENABLED 0
#define NRF_LOG_ENABLED 0
#define NRF_LOG_MSGPOOL_ELEMENT_SIZE 20
#define NRF_LOG_MSGPOOL_ELEMENT_COUNT 8
#define NRF_LOG_ALLOW_OVERFLOW 1
#define NRF_LOG_BUFSIZE 8192
#define NRF_LOG_CLI_CMDS 0
#define NRF_LOG_DEFAULT_LEVEL 3
#define NRF_LOG_DEFERRED 1
#define NRF_LOG_FILTERS_ENABLED 0
#define NRF_LOG_NON_DEFFERED_CRITICAL_REGION_ENABLED 0
#define NRF_LOG_STR_PUSH_BUFFER_SIZE 128
#define NRF_LOG_STR_PUSH_BUFFER_SIZE 128
#define NRF_LOG_USES_COLORS 0
#define NRF_LOG_USES_TIMESTAMP 0
#define NRF_LOG_TIMESTAMP_DEFAULT_FREQUENCY 0
#define NRF_LOG_STR_FORMATTER_TIMESTAMP_FORMAT_ENABLED 1

#define SEGGER_RTT_CONFIG_BUFFER_SIZE_UP 512
#define SEGGER_RTT_CONFIG_MAX_NUM_UP_BUFFERS 2
#define SEGGER_RTT_CONFIG_BUFFER_SIZE_DOWN 16
#define SEGGER_RTT_CONFIG_MAX_NUM_DOWN_BUFFERS 2
#define SEGGER_RTT_CONFIG_DEFAULT_MODE 2

#define NRF_FPRINTF_ENABLED 1
#define NRF_FPRINTF_FLAG_AUTOMATIC_CR_ON_LF_ENABLED 1
#define NRF_FPRINTF_DOUBLE_ENABLED 0

#define NRF_BALLOC_ENABLED 1
#define NRF_BALLOC_CONFIG_DEBUG_ENABLED 0
#define NRF_BALLOC_CONFIG_HEAD_GUARD_WORDS 1
#define NRF_BALLOC_CONFIG_TAIL_GUARD_WORDS 1
#define NRF_BALLOC_CONFIG_BASIC_CHECKS_ENABLED 0
#define NRF_BALLOC_CONFIG_DOUBLE_FREE_CHECK_ENABLED 0
#define NRF_BALLOC_CONFIG_DATA_TRASHING_CHECK_ENABLED 0
#define NRF_BALLOC_CLI_CMDS 0

#define NRF_MEMOBJ_ENABLED 1


