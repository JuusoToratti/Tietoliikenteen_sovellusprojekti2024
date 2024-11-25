#ifndef ADC_H_KJJ
#define ADC_H_KJJ

struct Measurement
{
   uint32_t x;
   uint32_t y;
   uint32_t z;
};

int initializeADC(void);
struct Measurement readADCValue(void);
void printDebugInfo(void);


#endif



