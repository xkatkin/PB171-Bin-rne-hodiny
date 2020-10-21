/*
 *	Simple (uncalibrated) delay functions
 *
 *	For precise delays, measure the result and tweak accordingly.
 * 
 *	Author: Martin Pavelek <he29@mail.muni.cz>
 *	Date:	2016-11-21
 */
#include <stdint.h>
//#include <delays.h>
#include <pic18.h>

/* delay in 10*x us
 * @16 MHz clock, 0 gives about 6.3 us
 */
void Delay100Us(unsigned int x){   
	unsigned int i;
    //for (i=0; i<x; i++){_delay(18);}  // 1 MHz
    for (i=0; i<x; i++){_delay(200);}   // 16 MHz   (0 gives about 6.3 us)
    for (i=0; i<x; i++){_delay(200);}
    //_delay(1);
};

void DelayMs(unsigned int x){
	unsigned int i;
	for (i=0; i<x; i++){Delay100Us(10);}
}


