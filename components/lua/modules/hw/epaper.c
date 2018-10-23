#include "sdkconfig.h"

#if CONFIG_LUA_RTOS_LUA_USE_EPAPER

#define EPD_WIDTH       200
#define EPD_HEIGHT      200

#define PANEL_SETTING                               0x00
#define POWER_SETTING                               0x01
#define POWER_OFF                                   0x02
#define POWER_OFF_SEQUENCE_SETTING                  0x03
#define POWER_ON                                    0x04
#define POWER_ON_MEASURE                            0x05
#define BOOSTER_SOFT_START                          0x06
#define DEEP_SLEEP                                  0x07
#define DATA_START_TRANSMISSION_1                   0x10
#define DATA_STOP                                   0x11
#define DISPLAY_REFRESH                             0x12
#define DATA_START_TRANSMISSION_2                   0x13
#define PLL_CONTROL                                 0x30
#define TEMPERATURE_SENSOR_COMMAND                  0x40
#define TEMPERATURE_SENSOR_CALIBRATION              0x41
#define TEMPERATURE_SENSOR_WRITE                    0x42
#define TEMPERATURE_SENSOR_READ                     0x43
#define VCOM_AND_DATA_INTERVAL_SETTING              0x50
#define LOW_POWER_DETECTION                         0x51
#define TCON_SETTING                                0x60
#define TCON_RESOLUTION                             0x61
#define SOURCE_AND_GATE_START_SETTING               0x62
#define GET_STATUS                                  0x71
#define AUTO_MEASURE_VCOM                           0x80
#define VCOM_VALUE                                  0x81
#define VCM_DC_SETTING_REGISTER                     0x82
#define PROGRAM_MODE                                0xA0
#define ACTIVE_PROGRAM                              0xA1
#define READ_OTP_DATA                               0xA2


const unsigned char lut_vcom0[] =
{
  0x0E, 0x14, 0x01, 0x0A, 0x06, 0x04, 0x0A, 0x0A,
  0x0F, 0x03, 0x03, 0x0C, 0x06, 0x0A, 0x00
};

const unsigned char lut_w[] =
{
  0x0E, 0x14, 0x01, 0x0A, 0x46, 0x04, 0x8A, 0x4A,
  0x0F, 0x83, 0x43, 0x0C, 0x86, 0x0A, 0x04
};

const unsigned char lut_b[] =
{
  0x0E, 0x14, 0x01, 0x8A, 0x06, 0x04, 0x8A, 0x4A,
  0x0F, 0x83, 0x43, 0x0C, 0x06, 0x4A, 0x04
};

const unsigned char lut_g1[] =
{
  0x8E, 0x94, 0x01, 0x8A, 0x06, 0x04, 0x8A, 0x4A,
  0x0F, 0x83, 0x43, 0x0C, 0x06, 0x0A, 0x04
};

const unsigned char lut_g2[] =
{
  0x8E, 0x94, 0x01, 0x8A, 0x06, 0x04, 0x8A, 0x4A,
  0x0F, 0x83, 0x43, 0x0C, 0x06, 0x0A, 0x04
};

const unsigned char lut_vcom1[] =
{
  0x03, 0x1D, 0x01, 0x01, 0x08, 0x23, 0x37, 0x37,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char lut_red0[] =
{
  0x83, 0x5D, 0x01, 0x81, 0x48, 0x23, 0x77, 0x77,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char lut_red1[] =
{
  0x03, 0x1D, 0x01, 0x01, 0x08, 0x23, 0x37, 0x37,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#include "freertos/FreeRTOS.h"

#include <string.h>
#include <sys/delay.h>
#include <sys/driver.h>
#include <sys/syslog.h>

#include "epaper.h"
#include <drivers/gpio.h>
#include <drivers/spi.h>

int spi_device;
int bus=CPU_SPI3;
int mosi=GPIO25;
int clk=GPIO17;
int rst=GPIO12;
int dc=GPIO27;
int ce=GPIO14;
int busy=GPIO26;

uint8_t* fb;
uint8_t* rfb;

void epaper_reset()
{
  gpio_pin_clr(rst);
  delay(200);
  gpio_pin_set(rst);
  delay(20);
}

driver_error_t* send(uint8_t d,bool cmd)
{
  driver_error_t*error;
  if ((error = spi_select(spi_device))) {
      return error;
  }
  gpio_pin_clr(ce);
  if(cmd)
    gpio_pin_clr(dc);
  else
    gpio_pin_set(dc);
  if ((error = spi_bulk_write(spi_device,1,&d))) {
    return error;
  }
  if ((error = spi_deselect(spi_device))) {
    return error;
  }
  gpio_pin_set(ce);
  return NULL;
}
driver_error_t* send_data(uint8_t* data,uint32_t len)
{
  driver_error_t*error;
  if ((error = spi_select(spi_device))) {
      return error;
  }
  gpio_pin_clr(ce);
  gpio_pin_set(dc);
  if ((error = spi_bulk_write(spi_device,len,data))) {
    return error;
  }
  if ((error = spi_deselect(spi_device))) {
    return error;
  }
  gpio_pin_set(ce);
  return NULL;
}

void wait_idle()
{
  uint8_t val;
  do {
    delay(1);
    gpio_pin_get(busy,&val);
  }
  while(val==0);
}

static int lepaper_init(lua_State* L) {
  fb=(uint8_t*)malloc(200*200/4);
  rfb=(uint8_t*)malloc(200*200/8);
  if(fb==NULL) luaL_error("FB1 NULL");
  if(rfb==NULL) luaL_error("FB2 NULL");
  gpio_pin_output(rst);
  gpio_pin_output(dc);
  gpio_pin_output(ce);
  gpio_pin_input(busy);
  driver_error_t*error;
  if (( error=spi_pin_map(bus,-1,mosi,clk))){
    return luaL_driver_error(L, error);
  }
  if (( error=spi_setup(bus, 1, 0, 0, 8000000, SPI_FLAG_WRITE, &spi_device))) {
    return luaL_driver_error(L, error);
  }
  epaper_reset();
  if((error=send(POWER_SETTING,true))) return luaL_driver_error(L, error);
  if((error=send(0x7,false))) return luaL_driver_error(L, error);
  if((error=send(0x0,false))) return luaL_driver_error(L, error);
  if((error=send(0x8,false))) return luaL_driver_error(L, error);
  if((error=send(0x0,false))) return luaL_driver_error(L, error);
  if((error=send(BOOSTER_SOFT_START,true))) return luaL_driver_error(L, error);
  if((error=send(0x7,false))) return luaL_driver_error(L, error);
  if((error=send(0x7,false))) return luaL_driver_error(L, error);
  if((error=send(0x7,false))) return luaL_driver_error(L, error);
  if((error=send(POWER_ON,true))) return luaL_driver_error(L, error);
  wait_idle();
  if((error=send(PANEL_SETTING,true))) return luaL_driver_error(L, error);
  if((error=send(0xcf,false))) return luaL_driver_error(L, error);
  if((error=send(VCOM_AND_DATA_INTERVAL_SETTING,true))) return luaL_driver_error(L, error);
  if((error=send(0x17,false))) return luaL_driver_error(L, error);
  if((error=send(PLL_CONTROL,true))) return luaL_driver_error(L, error);
  if((error=send(0x39,false))) return luaL_driver_error(L, error);
  if((error=send(TCON_RESOLUTION,true))) return luaL_driver_error(L, error);
  if((error=send(0xc8,false))) return luaL_driver_error(L, error);
  if((error=send(0x00,false))) return luaL_driver_error(L, error);
  if((error=send(0xc8,false))) return luaL_driver_error(L, error);
  if((error=send(VCM_DC_SETTING_REGISTER,true))) return luaL_driver_error(L, error);
  if((error=send(0xe,false))) return luaL_driver_error(L, error);
  if((error=send(0x20,true))) return luaL_driver_error(L, error);
  if((error=send_data(lut_vcom0,16))) return luaL_driver_error(L, error);
  if((error=send(0x21,true))) return luaL_driver_error(L, error);
  if((error=send_data(lut_w,16))) return luaL_driver_error(L, error);
  if((error=send(0x22,true))) return luaL_driver_error(L, error);
  if((error=send_data(lut_b,16))) return luaL_driver_error(L, error);
  if((error=send(0x23,true))) return luaL_driver_error(L, error);
  if((error=send_data(lut_g1,16))) return luaL_driver_error(L, error);
  if((error=send(0x24,true))) return luaL_driver_error(L, error);
  if((error=send_data(lut_g2,16))) return luaL_driver_error(L, error);
  if((error=send(0x25,true))) return luaL_driver_error(L, error);
  if((error=send_data(lut_vcom1,16))) return luaL_driver_error(L, error);
  if((error=send(0x26,true))) return luaL_driver_error(L, error);
  if((error=send_data(lut_red0,16))) return luaL_driver_error(L, error);
  if((error=send(0x27,true))) return luaL_driver_error(L, error);
  if((error=send_data(lut_red1,16))) return luaL_driver_error(L, error);

  syslog(LOG_INFO, "ePaper start");
  return 0;
}

static int lepaper_clear(lua_State* L) {
  memset(fb,0xff,200*200/4);
  memset(rfb,0xff,200*200/8);
  return 0;
}

static int lepaper_refresh(lua_State* L) {
  driver_error_t*error;
  if((error=send(DATA_START_TRANSMISSION_1,true))) return luaL_driver_error(L, error);
  delay(2);
  if((error=send_data(fb,200*200/4))) return luaL_driver_error(L, error);
  delay(2);
  if((error=send(DATA_START_TRANSMISSION_2,true))) return luaL_driver_error(L, error);
  delay(2);
  if((error=send_data(rfb,200*200/8))) return luaL_driver_error(L, error);
  delay(2);
  if((error=send(DISPLAY_REFRESH,true))) return luaL_driver_error(L, error);
  wait_idle();
  return 0;
}

static int lepaper_setpixel(lua_State* L) {
  int x=luaL_checkinteger(L,1)-1;
  int y=luaL_checkinteger(L,2)-1;
  int c=luaL_optinteger(L,3,0);
  if(x<0) return 0;
  if(y<0) return 0;
  if(x>=200) return 0;
  if(y>=200) return 0;
  int pix=x+(y*200);
  int byte=pix/4;
  int pos=pix%4;
  if(c==1)
  {
    fb[byte]&=~(3<<((3-pos)*2));
  }
  else
  {
    fb[byte]|=(3<<((3-pos)*2));
  }
  byte=pix/8;
  pos=pix%8;
  if(c==2)
  {
    rfb[byte]&=~(1<<((7-pos)));
  }
  else
  {
    rfb[byte]|=(1<<((7-pos)));
  }
  return 0;
}

static const LUA_REG_TYPE lepaper_map[] = {
    { LSTRKEY( "init"         ),     LFUNCVAL( lepaper_init    ) },
    { LSTRKEY( "setpixel"   ),     LFUNCVAL( lepaper_setpixel) },
    { LSTRKEY( "clear"   ),     LFUNCVAL( lepaper_clear) },
    { LSTRKEY( "refresh"   ),     LFUNCVAL( lepaper_refresh) },
    { LSTRKEY( "RED"      ),     LINTVAL ( 2) },
    { LSTRKEY( "BLACK"       ),     LINTVAL ( 1 ) },
    { LSTRKEY( "WHITE"     ),     LINTVAL ( 0 ) },
    { LNILKEY, LNILVAL }
};


LUALIB_API int luaopen_epaper( lua_State *L ) {
    return 0;
}

MODULE_REGISTER_ROM(EPAPER, epaper, lepaper_map, luaopen_epaper, 1);


#endif
