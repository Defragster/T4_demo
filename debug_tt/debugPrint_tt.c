
// #include "debug/printf.h"
// #include "debug/printf.h"

#define PRINT_DEBUG_STUFF

#ifdef PRINT_DEBUG_STUFF
// defining printf this way breaks things like Serial.printf() in C++ :(
// #define printf_tt(...) printf_debug(__VA_ARGS__)
#ifdef __cplusplus
extern "C" {
#endif
// void printf_debug_init(void);
void printf_tt(const char *format, ...);
#ifdef __cplusplus
}
#endif

#else
//#define printf_init()
#define printf_tt(...)
//#define printf_debug_init()

#endif


// #include "debug/printf.h"

#if 0
#ifdef PRINT_DEBUG_STUFF
#include <stdarg.h>
#include "imxrt.h"

void putchar_tt(char c);
static void puint_debug(unsigned int num);


__attribute__((section(".progmem")))
void printf_tt(const char *format, ...)
{
	va_list args;
	unsigned int val;
	int n;

	va_start(args, format);
	for (; *format != 0; format++) { // no-frills stand-alone printf
		if (*format == '%') {
			++format;
			if (*format == '%') goto out;
			if (*format == '-') format++; // ignore size
			while (*format >= '0' && *format <= '9') format++; // ignore size
			if (*format == 'l') format++; // ignore long
			if (*format == '\0') break;
			if (*format == 's') {
				printf_tt((char *)va_arg(args, int));
			} else if (*format == 'd') {
				n = va_arg(args, int);
				if (n < 0) {
					n = -n;
					putchar_tt('-');
				}
				puint_debug(n);
			} else if (*format == 'u') {
				puint_debug(va_arg(args, unsigned int));
			} else if (*format == 'x' || *format == 'X') {
				val = va_arg(args, unsigned int);
				for (n=0; n < 8; n++) {
					unsigned int d = (val >> 28) & 15;
					putchar_tt((d < 10) ? d + '0' : d - 10 + 'A');
					val <<= 4;
				}
			} else if (*format == 'c' ) {
				putchar_tt((char)va_arg(args, int));
			}
		} else {
			out:
			if (*format == '\n') putchar_tt('\r');
			putchar_tt(*format);
		}
	}
	va_end(args);
}

static void puint_debug(unsigned int num)
{
	char buf[12];
	unsigned int i = sizeof(buf)-2;

	buf[sizeof(buf)-1] = 0;
	while (1) {
		buf[i] = (num % 10) + '0';
		num /= 10;
		if (num == 0) break;
		i--;
	}
	printf_tt(buf + i);
}

__attribute__((section(".progmem")))
void putchar_tt(char c)
{
	while (!(LPUART3_STAT & LPUART_STAT_TDRE)) ; // wait
	LPUART3_DATA = c;
}

__attribute__((section(".progmem")))
void printf_tt_init(void)
{
        CCM_CCGR0 |= CCM_CCGR0_LPUART3(CCM_CCGR_ON); // turn on Serial4
        IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_06 = 2; // Arduino pin 17
        // LPUART3_BAUD = LPUART_BAUD_OSR(25) | LPUART_BAUD_SBR(8); // ~115200 baud
		LPUART3_BAUD = LPUART_BAUD_OSR(12) | LPUART_BAUD_SBR(1);	// 1843200 baud
        LPUART3_CTRL = LPUART_CTRL_TE;
}





#endif // PRINT_DEBUG_STUFF
#endif // FUTURE
