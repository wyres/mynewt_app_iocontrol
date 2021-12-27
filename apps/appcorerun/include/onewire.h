#ifndef ONEWIRE_H_   /* Include guard */
#define ONEWIRE_H_

void onewireWriteBit(int8_t pin, int b);
unsigned char onewireReadBit(int8_t pin);
bool onewireInit(int8_t pin);
unsigned char onewireReadByte(int8_t pin);
void onewireWriteByte(int8_t pin, char data);
unsigned char onewireCRC(unsigned char* addr, unsigned char len);

#endif