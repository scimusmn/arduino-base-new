/* @INCLUDE */
#include <avr/io.h>

/* @HEADER */
/* determine current architecture */
#if defined(__AVR_ATmega640__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)
	/* arduino mega */
#	define SMM_ARCH_MEGA

#elif defined(__AVR_ATmega48A__) || defined(__AVR_ATmega48PA__) || \
	defined(__AVR_ATmega88A__) || defined(__AVR_ATmega88PA__) || \
	defined(__AVR_ATmega168A__) || defined(__AVR_ATmega168PA__) || \
	defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__)
	/* arduino uno (and other things too?) */
#	define SMM_ARCH_UNO

#elif defined(__IMXRT1062__)
#    define SMM_ARCH_TEENSY4

#else
	/* other arduinos */
#	define SMM_ARCH_UNKNOWN

#endif
/* @END */
