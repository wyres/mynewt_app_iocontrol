#include "os/os.h"

#include "DS18B20.h"
#include "onewire.h"
/*
  send message to every sensor on the bus to take a reading
*/
bool ds18B20_broadcastConvert(int8_t pin) {
  //broadcast that temp conversions should begin, all at once so saves time
  if (!onewireInit(pin)) {
    return false;
  }
  onewireWriteByte(pin, 0xCC);
  onewireWriteByte(pin, 0x44);

  for(int i=0;i<10000;i++) {
    if (onewireReadBit(pin))
      return true;
  }
  return false;   // badness on the line...
}

/*
  retrieve temperatures from sensors
*/
float ds18B20_getTemperature(int8_t pin, unsigned char* address) {
  //get temperature from the device with address address
  float temperature;
  unsigned char scratchPad[9] = {0,0,0,0,0,0,0,0,0};

  if (onewireInit(pin)) {
    onewireWriteByte(pin, 0x55);
    unsigned char i;
    for (i = 0; i < 8; i++) {
      onewireWriteByte(pin, address[i]);
    }
    onewireWriteByte(pin, 0xBE);

    for (i = 0; i < 2; i++) {
      scratchPad[i] = onewireReadByte(pin);
    }
    onewireInit(pin);
    temperature = ((scratchPad[1] * 256) + scratchPad[0])*0.0625;

    return temperature;
  } else {
    // No device on the line
    return 0.0;
  }
}

int ds18B20_getTemperatureInt(int8_t pin, unsigned char* address) {
  //get temperature from the device with address address
  int temperature;
  unsigned char scratchPad[9] = {0,0,0,0,0,0,0,0,0};

  if (onewireInit(pin)) {
    onewireWriteByte(pin, 0x55);
    unsigned char i;
    for (i = 0; i < 8; i++) {
      onewireWriteByte(pin, address[i]);
    }
    onewireWriteByte(pin, 0xBE);

    for (i = 0; i < 2; i++) {
      scratchPad[i] = onewireReadByte(pin);
    }
    onewireInit(pin);
    temperature = ((scratchPad[1] * 256) + scratchPad[0]);

    return temperature;
  } else {
    return 0;
  }
}

/*
  retrieve address of 1 sensor 
*/
bool ds18B20_getSingleAddress(int8_t pin, unsigned char* address) {
  if (onewireInit(pin)) {
    //attach one sensor to port 25 and this will get it's address
    onewireWriteByte(pin, 0x33);
    unsigned char i;
    for (i = 0; i<8; i++) {
      address[i] = onewireReadByte(pin);
    }
    // check CRC
    if (onewireCRC(address, 7) != address[7]) {
      return false;
    }
    // Check its a DS18B20 or compatible
    // if (address[0] == 0x28 | 0x10 | 0x22) return true
    return true;
  } else {
    return false;
  }
}

#if 0
/*
  retrieve address of sensor and print to terminal
*/
void ds18B20_printSingleAddress() {
  onewireInit();
  //attach one sensor to port 25 and this will print out it's address
  unsigned char address[8]= {0,0,0,0,0,0,0,0};
  onewireWriteByte(0x33);
  unsigned char i;
  for (i = 0; i<8; i++)
    address[i] = onewireReadByte();
  for (i = 0; i<8; i++)
    printf("0x%x,",address[i]);
  
  //check crc
  unsigned char crc = onewireCRC(address, 7);
  printf("crc = %x \r\n",crc);
}
#endif