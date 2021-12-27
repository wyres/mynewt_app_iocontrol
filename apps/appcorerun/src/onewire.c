#include "os/os.h"
#include <hal/hal_gpio.h>
//#include <stm32l1xx_hal_gpio.h>

#include "onewire.h"

// Note not using gpiomgr as doesn't understand pins that switch between in and out like this

void __delay_us(int tus) {
    int64_t s = os_get_uptime_usec();
    uint32_t i = 0;
    while ((s+tus)>os_get_uptime_usec()) {
        i++;        // just to give it something to do
    }
}

void onewireWriteBit(int8_t pin, int b) {
  if (b & 0x01) {
    // init to out and Write : note its not what you write but when you write it that makes 1 or 0
    // Drive low briefly
    hal_gpio_init_out(pin, 0);
//    onewirePinDirection = 0;
//    onewirePin = 0;
    __delay_us(5);
    // Then drive it high to ensure its high 15uS after the previous falling edge
    hal_gpio_write(pin, 1);
    //hal_gpio_init_in(pin, HAL_GPIO_PULL_UP);
//    onewirePinDirection = 1;
    // and leave it high for enough time for slave to see it
    __delay_us(55);
  } else {
    // Write '0' bit by going low and staying low for 60us
    hal_gpio_init_out(pin, 0);
//    onewirePinDirection = 0;
//    onewirePin = 0;
    // and staying low when the slave reads the value about 15uS later
    __delay_us(60);
    // and force back high for 5uS (to be sure slave sees high) ready for next bit
    hal_gpio_write(pin, 1);
//    hal_gpio_init_in(pin, HAL_GPIO_PULL_UP);
//    onewirePinDirection = 1;
    __delay_us(5);
  }
}

unsigned char onewireReadBit(int8_t pin) {
  unsigned char result;

  // drive low 1-15uS to trigger operation
  hal_gpio_init_out(pin, 0);
//  onewirePinDirection = 0;
//  onewirePin = 0;
  __delay_us(5);    // was 1
  // and float so slave can drive it for 60uS
  hal_gpio_init_in(pin, HAL_GPIO_PULL_NONE);
//  onewirePinDirection = 1;
  __delay_us(10);   // 
  // read what the slave has done
  result = hal_gpio_read(pin);
//  result = onewirePin;
  // and wait for end of the 60uS slave driving (10+50)
  __delay_us(50);
  return result;

}

bool onewireInit(int8_t pin) {
  hal_gpio_init_in(pin, HAL_GPIO_PULL_NONE);
  // wait for it to float
  __delay_us(50);
  // Drive low
  hal_gpio_init_out(pin, 0);
  //onewirePinDirection = 0;
  //onewirePin = 0;
  __delay_us(480);
  // let it float high, devices should pull it low to say they exist in the next 60uS
  hal_gpio_init_in(pin, HAL_GPIO_PULL_NONE);
  //onewirePinDirection = 1;
  // wait for pull up if noone there
  __delay_us(20);
  // and read if any sensor drives the line low == presence
  //if (onewirePin == 0) {
  if (hal_gpio_read(pin) == 0) {
    // and wait before continuing to next transaction
    __delay_us(100);
    return true;
  }
  return false;
}

unsigned char onewireReadByte(int8_t pin) {
  unsigned char result = 0;

  for (unsigned char loop = 0; loop < 8; loop++) {
    // shift the result to get it ready for the next bit
    result >>= 1;

    // if result is one, then set MS bit
    if (onewireReadBit(pin))
      result |= 0x80;
  }
  return result;
}

void onewireWriteByte(int8_t pin, char data) {
  // Loop to write each bit in the byte, LS-bit first
  for (unsigned char loop = 0; loop < 8; loop++) {
    onewireWriteBit(pin, data & 0x01);

    // shift the data byte for the next bit
    data >>= 1;
  }
}

unsigned char onewireCRC(unsigned char* addr, unsigned char len) {
  unsigned char i, j;
  unsigned char crc = 0;

  for (i = 0; i < len; i++) {
    unsigned char inbyte = addr[i];
    for (j = 0; j < 8; j++) {
      unsigned char mix = (crc ^ inbyte) & 0x01;
      crc >>= 1;
      if (mix) crc ^= 0x8C;
      inbyte >>= 1;
    }
  }

  return crc;
}