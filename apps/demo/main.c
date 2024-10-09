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

#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "boards.h"
#include "app_error.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_drv_spi.h"

#include "crowd-supply-icon.c"

#if 1
// PCA10040 pin definitions
#undef ID130C_PIN_LCD_RST_N
#define ID130C_PIN_LCD_RST_N 20
#undef ID130C_PIN_LCD_CS_N
#define ID130C_PIN_LCD_CS_N 23
#undef ID130C_PIN_LCD_DC
#define ID130C_PIN_LCD_DC 19
#undef ID130C_PIN_LCD_CLK
#define ID130C_PIN_LCD_CLK 24
#undef ID130C_PIN_LCD_DATA
#define ID130C_PIN_LCD_DATA 22
#undef ID130C_PIN_LCD_BL_EN
#define ID130C_PIN_LCD_BL_EN 12
#endif

#define ID130C_PIXELS_X 80
#define ID130C_PIXELS_Y 160

#define SPI_INSTANCE 0
static const nrfx_spi_t spi_instance = NRFX_SPI_INSTANCE(SPI_INSTANCE);

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

static const struct {
  uint8_t data;
  uint8_t dc;
} sequence0[] = {
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
} sequence1[] = {
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
};

static void fill_screen(uint16_t colour) {
  for (unsigned idx=0; idx < sizeof(sequence1)/sizeof(sequence1[0]); ++idx) {
      nrf_gpio_pin_write(ID130C_PIN_LCD_DC, sequence1[idx].dc);
      uint8_t buffer[] = { sequence1[idx].data  };
      nrfx_spi_xfer_desc_t spi_xfer_desc = {
	.p_tx_buffer = buffer,
	.tx_length   = sizeof(buffer),
	.p_rx_buffer = NULL,
	.rx_length   = 0
      };
      nrfx_spi_xfer(&spi_instance, &spi_xfer_desc, 0);
  }
  {
    const uint8_t buffer[] = { ST7735_RAMWR };
    nrfx_spi_xfer_desc_t spi_xfer_desc = {
      .p_tx_buffer = buffer,
      .tx_length   = sizeof(buffer),
      .p_rx_buffer = NULL,
      .rx_length   = 0
    };
    nrf_gpio_pin_write(ID130C_PIN_LCD_DC, 0);
    nrfx_spi_xfer(&spi_instance, &spi_xfer_desc, 0);
    nrf_gpio_pin_write(ID130C_PIN_LCD_DC, 1);
  }
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
      nrfx_spi_xfer(&spi_instance, &spi_xfer_desc, 0);
    }
  }
}

static void fill_logo(void) {
  for (unsigned idx=0; idx < sizeof(sequence1)/sizeof(sequence1[0]); ++idx) {
      nrf_gpio_pin_write(ID130C_PIN_LCD_DC, sequence1[idx].dc);
      uint8_t buffer[] = { sequence1[idx].data  };
      nrfx_spi_xfer_desc_t spi_xfer_desc = {
	.p_tx_buffer = buffer,
	.tx_length   = sizeof(buffer),
	.p_rx_buffer = NULL,
	.rx_length   = 0
      };
      nrfx_spi_xfer(&spi_instance, &spi_xfer_desc, 0);
  }
  {
    const uint8_t buffer[] = { ST7735_RAMWR };
    nrfx_spi_xfer_desc_t spi_xfer_desc = {
      .p_tx_buffer = buffer,
      .tx_length   = sizeof(buffer),
      .p_rx_buffer = NULL,
      .rx_length   = 0
    };
    nrf_gpio_pin_write(ID130C_PIN_LCD_DC, 0);
    nrfx_spi_xfer(&spi_instance, &spi_xfer_desc, 0);
    nrf_gpio_pin_write(ID130C_PIN_LCD_DC, 1);
  }
  {
    nrfx_spi_xfer_desc_t spi_xfer_desc = {
      .p_tx_buffer = (uint8_t*) image,
      .tx_length   = sizeof(image)-1,
      .p_rx_buffer = NULL,
      .rx_length   = 0
    };
    nrfx_spi_xfer(&spi_instance, &spi_xfer_desc, 0);
  }
}

static void lcd_init(void) {
  // setup spi
  nrfx_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
  spi_config.frequency = NRF_DRV_SPI_FREQ_8M;
  spi_config.ss_pin   = NRFX_SPI_PIN_NOT_USED;
  spi_config.miso_pin = NRFX_SPI_PIN_NOT_USED;
  spi_config.mosi_pin = ID130C_PIN_LCD_DATA;
  spi_config.sck_pin  = ID130C_PIN_LCD_CLK;
  APP_ERROR_CHECK(nrfx_spi_init(&spi_instance, &spi_config, NULL, NULL));
  
  // backlight off
  nrf_gpio_pin_clear(ID130C_PIN_LCD_BL_EN);
  nrf_gpio_cfg_output(ID130C_PIN_LCD_BL_EN);

  // pin defaults
  nrf_gpio_pin_clear(ID130C_PIN_LCD_DC);
  nrf_gpio_pin_clear(ID130C_PIN_LCD_RST_N);
  nrf_gpio_pin_clear(ID130C_PIN_LCD_CS_N);
  nrf_gpio_cfg_output(ID130C_PIN_LCD_RST_N);
  nrf_gpio_cfg_output(ID130C_PIN_LCD_DC);
  nrf_gpio_cfg_output(ID130C_PIN_LCD_CS_N);

  // power on
  nrf_gpio_pin_clear(ID130C_PIN_LCD_PWR_N);
  nrf_gpio_cfg_output(ID130C_PIN_LCD_PWR_N);

  // reset
  nrf_gpio_pin_set(ID130C_PIN_LCD_RST_N);
  nrf_delay_ms(5);
  nrf_gpio_pin_clear(ID130C_PIN_LCD_RST_N);
  nrf_delay_ms(5);
  nrf_gpio_pin_set(ID130C_PIN_LCD_RST_N);

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
    nrfx_spi_xfer(&spi_instance, &spi_xfer_desc, 0);
  }

  // pause
  nrf_delay_ms(120);

  // sequence 0
  for (unsigned idx=0; idx < sizeof(sequence0)/sizeof(sequence0[0]); ++idx) {
      nrf_gpio_pin_write(ID130C_PIN_LCD_DC, sequence0[idx].dc);
      uint8_t buffer[] = { sequence0[idx].data  };
      nrfx_spi_xfer_desc_t spi_xfer_desc = {
	.p_tx_buffer = buffer,
	.tx_length   = sizeof(buffer),
	.p_rx_buffer = NULL,
	.rx_length   = 0
      };
      NRF_LOG_INFO("%d: %08x %d", idx, sequence0[idx].data, sequence0[idx].dc);
      nrfx_spi_xfer(&spi_instance, &spi_xfer_desc, 0);
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
    nrfx_spi_xfer(&spi_instance, &spi_xfer_desc, 0);
  }

  // pause
  nrf_delay_ms(120);

  // sequence 1
  for (unsigned idx=0; idx < sizeof(sequence1)/sizeof(sequence1[0]); ++idx) {
     nrf_gpio_pin_write(ID130C_PIN_LCD_DC, sequence1[idx].dc);
      uint8_t buffer[] = { sequence1[idx].data  };
      nrfx_spi_xfer_desc_t spi_xfer_desc = {
	.p_tx_buffer = buffer,
	.tx_length   = sizeof(buffer),
	.p_rx_buffer = NULL,
	.rx_length   = 0
      };
      NRF_LOG_INFO("%d: %08x %d", idx, sequence1[idx].data, sequence1[idx].dc);
      nrfx_spi_xfer(&spi_instance, &spi_xfer_desc, 0);
  }

  // pause
  nrf_delay_ms(120);
}

int main(void)
{
    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    NRF_LOG_INFO("Demo application started.")
    NRF_LOG_FLUSH();

    lcd_init();

    while (1)
    {
      // Fill screen
      nrf_gpio_pin_clear(ID130C_PIN_LCD_BL_EN);
      fill_screen(0xf800);
      nrf_gpio_pin_set(ID130C_PIN_LCD_BL_EN);
      nrf_delay_ms(1000);
      // Fill screen
      nrf_gpio_pin_clear(ID130C_PIN_LCD_BL_EN);
      fill_screen(0x07e0);
      nrf_gpio_pin_set(ID130C_PIN_LCD_BL_EN);
      nrf_delay_ms(1000);
      // Fill screen
      nrf_gpio_pin_clear(ID130C_PIN_LCD_BL_EN);
      fill_screen(0x001f);
      nrf_gpio_pin_set(ID130C_PIN_LCD_BL_EN);
      nrf_delay_ms(1000);
      // Fill screen with logo
      nrf_gpio_pin_clear(ID130C_PIN_LCD_BL_EN);
      fill_logo();
      nrf_gpio_pin_set(ID130C_PIN_LCD_BL_EN);
      nrf_delay_ms(1000);
    }
}

