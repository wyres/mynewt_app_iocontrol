#ifndef DS18B20_h
#define DS18B20_h

bool ds18B20_broadcastConvert(int8_t pin);
float ds18B20_getTemperature(int8_t pin, unsigned char* address);
int ds18B20_getTemperatureInt(int8_t pin, unsigned char* address);
bool ds18B20_getSingleAddress(int8_t pin, unsigned char* address);

#endif