/*
BSD 2-Clause License

Copyright (c) 2024, Tom Keddie

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdint.h>

#include "nrf.h"
#include "nrf_log.h"
#include "nrf_delay.h"
#include "nrf_drv_spi.h"
#include "nrfx_gpiote.h"

#include "id130c_pins.h"

#include "crowd_supply_icon.h"

#define LCD_SPI_INSTANCE 0
static const nrfx_spi_t lcd_spi_instance = NRFX_SPI_INSTANCE(LCD_SPI_INSTANCE);

#define ID130C_PIXELS_X 80
#define ID130C_PIXELS_Y 160

// Set of commands described in ST7735 data sheet.
#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09

#define ST7735_SLPIN   0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON   0x12
#define ST7735_NORON   0x13

#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_RAMRD   0x2E

#define ST7735_PTLAR   0x30
#define ST7735_COLMOD  0x3A
#define ST7735_MADCTL  0x36

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5
#define ST7735_VMOFCTR 0xC7

#define ST7735_RDID1   0xDA
#define ST7735_RDID2   0xDB
#define ST7735_RDID3   0xDC
#define ST7735_RDID4   0xDD

#define ST7735_PWCTR6  0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

#define ST7735_MADCTL_MY  0x80
#define ST7735_MADCTL_MX  0x40
#define ST7735_MADCTL_MV  0x20
#define ST7735_MADCTL_ML  0x10
#define ST7735_MADCTL_RGB 0x00
#define ST7735_MADCTL_BGR 0x08
#define ST7735_MADCTL_MH  0x04

// This sequence was reverse engineered from a working band.  Some of
// the commands arent in the data sheet but they've been left here for
// now.
static const struct {
  uint8_t data;
  uint8_t dc;
} init_sequence[] = {
  {0xF0, 0},
  {0x11, 1},
  {0xF0, 0},
  {0xCA, 1},
  {0x78, 1},
  {0x64, 1},
  {ST7735_FRMCTR1, 0},
  {0x05, 1},
  {0x3C, 1},
  {0x3C, 1},
  {ST7735_FRMCTR2, 0},
  {0x05, 1},
  {0x3C, 1},
  {0x3C, 1},
  {ST7735_FRMCTR3, 0},
  {0x05, 1},
  {0x3C, 1},
  {0x3C, 1},
  {0x05, 1},
  {0x3C, 1},
  {0x3C, 1},
  {ST7735_INVCTR, 0},
  {0x03, 1},
  {ST7735_PWCTR1, 0},
  {0xE0, 1},
  {0x00, 1},
  {0x07, 1},
  {ST7735_PWCTR2, 0},
  {0xC5, 1},
  {ST7735_PWCTR3, 0},
  {0x0A, 1},
  {0x00, 1},
  {ST7735_PWCTR5, 0},
  {0x8D, 1},
  {0xEE, 1},
  {ST7735_VMCTR1, 0},
  {0x03, 1},
  {ST7735_VMOFCTR, 0},
  {0x76, 1},
  {ST7735_MADCTL, 0},
  {0xC8, 1},
  {ST7735_COLMOD, 0},
  {0x05, 1},
  {ST7735_GMCTRP1, 0},
  {0x27, 1},
  {0x0E, 1},
  {0x07, 1},
  {0x04, 1},
  {0x11, 1},
  {0x0B, 1},
  {0x06, 1},
  {0x0C, 1},
  {0x0E, 1},
  {0x14, 1},
  {0x1B, 1},
  {0x3E, 1},
  {0x06, 1},
  {0x25, 1},
  {0x07, 1},
  {0x1F, 1},
  {ST7735_GMCTRN1, 0},
  {0x27, 1},
  {0x0E, 1},
  {0x07, 1},
  {0x04, 1},
  {0x11, 1},
  {0x0B, 1},
  {0x06, 1},
  {0x0C, 1},
  {0x0E, 1},
  {0x14, 1},
  {0x1B, 1},
  {0x3E, 1},
  {0x30, 1},
  {0x25, 1},
  {0x07, 1},
  {0x1F, 1},
  {ST7735_NORON, 0},
};

static const struct {
  uint8_t data;
  uint8_t dc;
} sequence_ramwr[] = {
  // 103 - 24 + 1 = 80 columns
  {ST7735_CASET, 0},
  {0x00, 1},
  {0x18, 1},  // 24
  {0x00, 1},
  {0x67, 1},  // 103
  // 159 - 0 + 1 = 160
  {ST7735_RASET, 0},
  {0x00, 1},
  {0x00, 1},  // 0
  {0x00, 1},
  {0x9F, 1},  // 159
  {ST7735_RAMWR, 0},
};

static void fill_screen(uint16_t colour) {
  for (unsigned idx=0; idx < sizeof(sequence_ramwr)/sizeof(sequence_ramwr[0]); ++idx) {
    if (sequence_ramwr[idx].dc) nrfx_gpiote_out_set(ID130C_PIN_LCD_DC); else  nrfx_gpiote_out_clear(ID130C_PIN_LCD_DC);
    uint8_t buffer[] = { sequence_ramwr[idx].data  };
    nrfx_spi_xfer_desc_t spi_xfer_desc = {
      .p_tx_buffer = buffer,
      .tx_length   = sizeof(buffer),
      .p_rx_buffer = NULL,
      .rx_length   = 0
    };
    nrfx_spi_xfer(&lcd_spi_instance, &spi_xfer_desc, 0);
  }
  nrfx_gpiote_out_set(ID130C_PIN_LCD_DC);
  {
    uint16_t fill[ID130C_PIXELS_X];
    for (unsigned ix=0; ix < sizeof(fill)/sizeof(fill[0]); ++ix) {
      fill[ix] = colour;
    }
    nrfx_spi_xfer_desc_t spi_xfer_desc = {
      .p_tx_buffer = (uint8_t*) fill,
      .tx_length   = sizeof(fill),
      .p_rx_buffer = NULL,
      .rx_length   = 0
    };
    for (unsigned idx=0; idx < ID130C_PIXELS_Y; ++idx) {
      nrfx_spi_xfer(&lcd_spi_instance, &spi_xfer_desc, 0);
    }
  }
}

static void fill_logo(void) {
  for (unsigned idx=0; idx < sizeof(sequence_ramwr)/sizeof(sequence_ramwr[0]); ++idx) {
    if (sequence_ramwr[idx].dc) nrfx_gpiote_out_set(ID130C_PIN_LCD_DC); else  nrfx_gpiote_out_clear(ID130C_PIN_LCD_DC);
    uint8_t buffer[] = { sequence_ramwr[idx].data  };
    nrfx_spi_xfer_desc_t spi_xfer_desc = {
      .p_tx_buffer = buffer,
      .tx_length   = sizeof(buffer),
      .p_rx_buffer = NULL,
      .rx_length   = 0
    };
    nrfx_spi_xfer(&lcd_spi_instance, &spi_xfer_desc, 0);
  }
  nrfx_gpiote_out_set(ID130C_PIN_LCD_DC);
  const unsigned char* image = crowd_supply_icon();
  {
    nrfx_spi_xfer_desc_t spi_xfer_desc = {
      .p_tx_buffer = (uint8_t*) image,
      .tx_length   = sizeof(uint16_t)*ID130C_PIXELS_X*ID130C_PIXELS_Y/2,
      .p_rx_buffer = NULL,
      .rx_length   = 0
    };
    nrfx_spi_xfer(&lcd_spi_instance, &spi_xfer_desc, 0);
  }
}

static void fill_info(void) {
#if 0  
  uint32_t mac = NRF_FICR->DEVICEADDR[0];
#endif
}

void id130c_lcd_demo(void) {
    // Fill screen with blue
    fill_screen(0xf800);
    nrf_delay_ms(1000);
    // Fill screen with red
    fill_screen(0x07e0);
    nrf_delay_ms(1000);
    // Fill screen with green
    fill_screen(0x001f);
    nrf_delay_ms(1000);
    // Fill screen with white
    fill_screen(0xffff);
    nrf_delay_ms(1000);
    // Fill screen with logo
    fill_logo();
    fill_info();
    nrf_delay_ms(1000);
}

void id130c_lcd_init(void) {
  // setup spi
  nrfx_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
  spi_config.frequency = NRF_DRV_SPI_FREQ_8M;
  spi_config.ss_pin   = NRFX_SPI_PIN_NOT_USED;
  spi_config.miso_pin = NRFX_SPI_PIN_NOT_USED;
  spi_config.mosi_pin = ID130C_PIN_LCD_DATA;
  spi_config.sck_pin  = ID130C_PIN_LCD_CLK;
  APP_ERROR_CHECK(nrfx_spi_init(&lcd_spi_instance, &spi_config, NULL, NULL));

  nrfx_gpiote_out_config_t out_config = NRFX_GPIOTE_CONFIG_OUT_SIMPLE(false);

  // backlight off
  out_config.init_state = NRF_GPIOTE_INITIAL_VALUE_HIGH;
  nrfx_gpiote_out_init(ID130C_PIN_LCD_BL_EN, &out_config);

  // pin defaults
  out_config.init_state = NRF_GPIOTE_INITIAL_VALUE_LOW;
  nrfx_gpiote_out_init(ID130C_PIN_LCD_DC, &out_config);
  nrfx_gpiote_out_init(ID130C_PIN_LCD_RST_N, &out_config);
  nrfx_gpiote_out_init(ID130C_PIN_LCD_CS_N, &out_config);

  // power on
  out_config.init_state = NRF_GPIOTE_INITIAL_VALUE_LOW;
  nrfx_gpiote_out_init(ID130C_PIN_LCD_PWR_N, &out_config);

  // reset
  nrfx_gpiote_out_set(ID130C_PIN_LCD_RST_N);
  nrf_delay_ms(5);
  nrfx_gpiote_out_clear(ID130C_PIN_LCD_RST_N);
  nrf_delay_ms(5);
  nrfx_gpiote_out_set(ID130C_PIN_LCD_RST_N);

  // pause then SLPOUT
  nrf_delay_ms(140);
  {
    const uint8_t buffer[] = { ST7735_SLPOUT };
    nrfx_spi_xfer_desc_t spi_xfer_desc = {
      .p_tx_buffer = buffer,
      .tx_length   = sizeof(buffer),
      .p_rx_buffer = NULL,
      .rx_length   = 0
    };
    nrfx_spi_xfer(&lcd_spi_instance, &spi_xfer_desc, 0);
  }

  // pause then init sequence
  nrf_delay_ms(120);
  for (unsigned idx=0; idx < sizeof(init_sequence)/sizeof(init_sequence[0]); ++idx) {
    if (init_sequence[idx].dc) nrfx_gpiote_out_set(ID130C_PIN_LCD_DC); else  nrfx_gpiote_out_clear(ID130C_PIN_LCD_DC);
    uint8_t buffer[] = { init_sequence[idx].data  };
    nrfx_spi_xfer_desc_t spi_xfer_desc = {
      .p_tx_buffer = buffer,
      .tx_length   = sizeof(buffer),
      .p_rx_buffer = NULL,
      .rx_length   = 0
    };
    nrfx_spi_xfer(&lcd_spi_instance, &spi_xfer_desc, 0);
  }

  // pause and turn on
  nrf_delay_ms(10);
  {
    const uint8_t buffer[] = { ST7735_DISPON };
    nrfx_spi_xfer_desc_t spi_xfer_desc = {
      .p_tx_buffer = buffer,
      .tx_length   = sizeof(buffer),
      .p_rx_buffer = NULL,
      .rx_length   = 0
    };
    nrfx_spi_xfer(&lcd_spi_instance, &spi_xfer_desc, 0);
  }

  // pause
  nrf_delay_ms(120);
}
