#define OVERCLOCK	//if set, 33mhz, else 32

#ifdef OVERCLOCK
	#define _XTAL_FREQ 33000000
#else
	#define _XTAL_FREQ 32000000
#endif
#include <xc.h>