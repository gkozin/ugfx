/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/gdisp/HX8347D/board_HX8347D_stm32f4discovery.h
 * @brief   GDISP Graphic Driver subsystem board SPI interface for the HX8347D display.
 */

#ifndef _GDISP_LLD_BOARD_H
#define _GDISP_LLD_BOARD_H

// For a multiple display configuration we would put all this in a structure and then
//	set g->priv to that structure.

/* Pin assignments */
#define SET_RST		palSetPad(GPIOB, 8)
#define CLR_RST		palClearPad(GPIOB, 8)
#define SET_DATA	palSetPad(GPIOB, 9)
#define CLR_DATA	palClearPad(GPIOB, 9)
#define SET_CS		palSetPad(GPIOA, 4)
#define CLR_CS		palClearPad(GPIOA, 4)

/* PWM configuration structure. We use timer 4 channel 2 (orange LED on board). */
static const PWMConfig pwmcfg = {
	1000000,		/* 1 MHz PWM clock frequency. */
	100,			/* PWM period is 100 cycles. */
	NULL,
	{
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL}
	},
	0
};

/*
 * SPI1 configuration structure.
 * Speed 42MHz, CPHA=0, CPOL=0, 8bits frames, MSb transmitted first.
 * The slave select line is the pin 4 on the port GPIOA.
 */
static const SPIConfig spi1cfg_8bit = {
	NULL,
	/* HW dependent part.*/
	GPIOA,
	4,
	0 //SPI_CR1_BR_0
};

/*
 * SPI1 configuration structure.
 * Speed 42MHz, CPHA=0, CPOL=0, 16bits frames, MSb transmitted first.
 * The slave select line is the pin 4 on the port GPIOA.
 */
static const SPIConfig spi1cfg_16bit = {
	NULL,
	/* HW dependent part.*/
	GPIOA,
	4,
	SPI_CR1_DFF //SPI_CR1_BR_0
};

static inline void init_board(GDisplay *g, unsigned display) {

	// As we are not using multiple displays we set g->priv to NULL as we don't use it.
	g->priv = 0;

	if (display == 0) {

		/**
		 * Set up for Display 0
		 */

		/* Display backlight control */
		/* TIM4 is an alternate function 2 (AF2) */
		pwmStart(&PWMD4, &pwmcfg);
		palSetPadMode(GPIOD, 13, PAL_MODE_ALTERNATE(2));
		pwmEnableChannel(&PWMD4, 1, 100);

		palSetPadMode(GPIOB, 8, PAL_MODE_OUTPUT_PUSHPULL|PAL_STM32_OSPEED_HIGHEST);			/* RST    */
		palSetPadMode(GPIOB, 9, PAL_MODE_OUTPUT_PUSHPULL|PAL_STM32_OSPEED_HIGHEST);			/* RS     */
		/*
		 * Initializes the SPI driver 1. The SPI1 signals are routed as follow:
		 * PB12 - NSS.
		 * PB13 - SCK.
		 * PB14 - MISO.
		 * PB15 - MOSI.
		 */
		SET_CS; SET_DATA;
		spiStart(&SPID1, &spi1cfg_8bit);
		palSetPadMode(GPIOA, 4, PAL_MODE_OUTPUT_PUSHPULL|PAL_STM32_OSPEED_HIGHEST);			/* NSS.     */
		palSetPadMode(GPIOA, 5, PAL_MODE_ALTERNATE(5)|PAL_STM32_OSPEED_HIGHEST);			/* SCK.     */
		palSetPadMode(GPIOA, 6, PAL_MODE_ALTERNATE(5));										/* MISO.    */
		palSetPadMode(GPIOA, 7, PAL_MODE_ALTERNATE(5)|PAL_STM32_OSPEED_HIGHEST);			/* MOSI.    */
	}
}

static inline void post_init_board(GDisplay *g) {
	(void) g;
}

static inline void setpin_reset(GDisplay *g, bool_t state) {
	(void) g;
	if (state) {
		CLR_RST;
	} else {
		SET_RST;
	}
}

static inline void set_backlight(GDisplay *g, uint8_t percent) {
	(void) g;
	pwmEnableChannel(&PWMD4, 1, percent);
}

static inline void acquire_bus(GDisplay *g) {
	(void) g;
	spiAcquireBus(&SPID1);
    while(((SPI1->SR & SPI_SR_TXE) == 0) || ((SPI1->SR & SPI_SR_BSY) != 0));		// Safety
	CLR_CS;
}

static inline void release_bus(GDisplay *g) {
	(void) g;
	SET_CS;
	spiReleaseBus(&SPID1);
}

static inline void busmode16(GDisplay *g) {
	(void) g;
	spiStart(&SPID1, &spi1cfg_16bit);
}

static inline void busmode8(GDisplay *g) {
	(void) g;
	spiStart(&SPID1, &spi1cfg_8bit);
}

static inline void write_index(GDisplay *g, uint8_t index) {
	(void) g;
    CLR_DATA;
    SPI1->DR = index;
    while(((SPI1->SR & SPI_SR_TXE) == 0) || ((SPI1->SR & SPI_SR_BSY) != 0));
    SET_DATA;
}

static inline void write_data(GDisplay *g, uint8_t data) {
	(void) g;
    SPI1->DR = data;
    while(((SPI1->SR & SPI_SR_TXE) == 0) || ((SPI1->SR & SPI_SR_BSY) != 0));
}

static inline void write_ram16(GDisplay *g, uint16_t data) {
	(void) g;
    SPI1->DR      = data;
    while((SPI1->SR & SPI_SR_TXE) == 0);
}

#endif /* _GDISP_LLD_BOARD_H */
