#ifndef EPAPER_H_
#define EPAPER_H_

#include "sdkconfig.h"

#if CONFIG_LUA_RTOS_LUA_USE_EPAPER

#include "luartos.h"

#include <drivers/spi.h>
#include <drivers/cpu.h>

typedef struct {
	int spi_device; // SPI device
	uint8_t  *buff; // Data buffer
	uint32_t len;   // Data buffer length
} epaper_userdata;


#endif
#endif
