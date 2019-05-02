// https://forum.pjrc.com/threads/54711-Teensy-4-0-First-Beta-Test?p=204748&viewfull=1#post204748
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

// One of these must be defined, usually via the Makefile
//#define MACOSX
//#define LINUX
#define WINDOWS

#if defined(MACOSX) || defined(LINUX)
#include <termios.h>
#include <sys/select.h>
#define PORTTYPE int
#define BAUD B115200
#if defined(LINUX)
#include <sys/ioctl.h>
#include <linux/serial.h>
#endif
#elif defined(WINDOWS)
#include <windows.h>
#define PORTTYPE HANDLE
#define BAUD 115200
#define gettimeofday gettimeofday_
#else
#error "You must define the operating system\n"
#endif

// function prototypes
PORTTYPE open_port_and_set_baud_or_die(const char *name, long baud);
void close_port(PORTTYPE port);
int receive_bytes_X(PORTTYPE port, int cnt);
void delay(double sec);
void die(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

int gettimeofday(struct timeval * tp, struct timezone * tzp);

int surge = 0;

int receive_bytes_X(PORTTYPE port, int cnt)
{
	int count=0;
	char buf[65537];
	int len;
	len = sizeof(buf)-1;
	int once=1;
#if defined(MACOSX) || defined(LINUX)
	int r;
	int retry=0;

	// non-blocking read mode
	fcntl(port, F_SETFL, fcntl(port, F_GETFL) | O_NONBLOCK);
	for ( int ii=0; ii<cnt; ii++ ) {
		count = 0;
		once=1;
		while (count < len) {
			r = read(port, buf + count, len - count);
			//printf("read, r = %d\n", r);
			if (r < 0 && errno != EAGAIN && errno != EINTR) return -1;
			else if (r > 0) {
				count += r;
				if ( once && count >1000 ) {
					int pp=0;
					once =0;
					while ( '\n' != buf[once] ) once++;
					once++;
					while ( '\n' != buf[once+pp] ) pp++;
					pp--;
					buf[once+pp]=0;
					printf( "#%d : __>> %s <<__\n", ii, &buf[once] );
					once=0;
				}
			}
			else {
				// no data available right now, must wait
				if ( surge ) delay(surge*0.003);
				fd_set fds;
				struct timeval t;
				FD_ZERO(&fds);
				FD_SET(port, &fds);
				t.tv_sec = 1;
				t.tv_usec = 0;
				r = select(port+1, &fds, NULL, NULL, &t);
				//printf("select, r = %d\n", r);
				if (r < 0) return -1;
				if (r == 0) return count; // timeout
			}
			retry++;
			if (retry > 1000) return -100; // no input
		}
		if ( surge ) delay(surge*0.03);
	}
	fcntl(port, F_SETFL, fcntl(port, F_GETFL) & ~O_NONBLOCK);
#elif defined(WINDOWS)
	COMMTIMEOUTS timeout;
	DWORD n;
	BOOL r;
	int waiting=0;

	GetCommTimeouts(port, &timeout);
	timeout.ReadIntervalTimeout = MAXDWORD; // non-blocking
	timeout.ReadTotalTimeoutMultiplier = 0;
	timeout.ReadTotalTimeoutConstant = 0;
	SetCommTimeouts(port, &timeout);
	for ( int ii=0; ii<cnt; ii++ ) {
		count = 0;
		once=1;
		while (count < len) {
			r = ReadFile(port, buf + count, len - count, &n, NULL);
			if (!r) die("read error\n");
			if (n > 0) {
				count += n;
				if ( once && count >1000 ) {
					int pp=0;
					once =0;
					while ( '\n' != buf[once] ) once++;
					once++;
					while ( '\n' != buf[once+pp] ) pp++;
					pp--;
					buf[once+pp]=0;
					printf( "#%d : __>> %s <<__\n", ii, &buf[once] );
					once=0;
				}
			}
			else {
				if ( surge ) delay(surge*0.003);
				if (waiting) {
					printf( "\t_brk_\n" ); // not seeing this - just wondered
					break;  // 1 sec timeout
				}
				timeout.ReadIntervalTimeout = MAXDWORD;
				timeout.ReadTotalTimeoutMultiplier = MAXDWORD;
				timeout.ReadTotalTimeoutConstant = 1000;
				SetCommTimeouts(port, &timeout);
				waiting = 1;
			}
		}
		if ( surge ) delay(surge*0.03);
	}
#endif
	return count;
}

int main(int argc, char **argv)
{
	PORTTYPE fd;
	int blkcnt = 20;

	if (argc < 2) die("Usage: :) latency_test3 <comport>\n");
	fd = open_port_and_set_baud_or_die(argv[1], BAUD);
	if (argc > 2 && argv[2][0] >'0' && argv[2][0] <= '9' ) blkcnt *= (argv[2][0] - '0');
	if (argc > 3 && argv[3][0] >'0' && argv[3][0] <= '9' ) surge += (argv[3][0] - '0');
	printf("port %s opened\n", argv[1]);
	printf("repeat %d \n", blkcnt);
	printf("surge %d delay\n", surge);

	struct timeval begin, end;
	double elapsed;
	int totBytes;

	int r;
	// test begin
	gettimeofday(&begin, NULL);
	r = receive_bytes_X( fd, blkcnt );
	gettimeofday(&end, NULL);
	elapsed = (double)(end.tv_sec - begin.tv_sec) * 1000.0;
	elapsed += (double)(end.tv_usec - begin.tv_usec) / 1000.0;
	printf(" ------\t elapsed time %.3f secs for %d bytes\n", elapsed/1000.0, blkcnt*8096 );
	close_port(fd);
	return 0;
}


/**********************************/
/*  Serial Port Functions         */
/**********************************/


PORTTYPE open_port_and_set_baud_or_die(const char *name, long baud)
{
	PORTTYPE fd;
#if defined(MACOSX)
	struct termios tinfo;
	fd = open(name, O_RDWR | O_NONBLOCK);
	if (fd < 0) die("unable to open port %s\n", name);
	if (tcgetattr(fd, &tinfo) < 0) die("unable to get serial parms\n");
	if (cfsetspeed(&tinfo, baud) < 0) die("error in cfsetspeed\n");
	tinfo.c_cflag |= CLOCAL;
	if (tcsetattr(fd, TCSANOW, &tinfo) < 0) die("unable to set baud rate\n");
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_NONBLOCK);
#elif defined(LINUX)
	struct termios tinfo;
	struct serial_struct kernel_serial_settings;
	int r;
	fd = open(name, O_RDWR);
	if (fd < 0) die("unable to open port %s\n", name);
	if (tcgetattr(fd, &tinfo) < 0) die("unable to get serial parms\n");
	if (cfsetspeed(&tinfo, baud) < 0) die("error in cfsetspeed\n");
	if (tcsetattr(fd, TCSANOW, &tinfo) < 0) die("unable to set baud rate\n");
	r = ioctl(fd, TIOCGSERIAL, &kernel_serial_settings);
	if (r >= 0) {
		kernel_serial_settings.flags |= ASYNC_LOW_LATENCY;
		r = ioctl(fd, TIOCSSERIAL, &kernel_serial_settings);
		if (r >= 0) printf("set linux low latency mode\n");
	}
#elif defined(WINDOWS)
	COMMCONFIG cfg;
	COMMTIMEOUTS timeout;
	DWORD n;
	char portname[256];
	int num;
	if (sscanf(name, "COM%d", &num) == 1) {
		sprintf(portname, "\\\\.\\COM%d", num); // Microsoft KB115831
	} else {
		strncpy(portname, name, sizeof(portname)-1);
		portname[n-1] = 0;
	}
	fd = CreateFile(portname, GENERIC_READ | GENERIC_WRITE,
		0, 0, OPEN_EXISTING, 0, NULL);
	if (fd == INVALID_HANDLE_VALUE) die("unable to open port %s\n", name);
	GetCommConfig(fd, &cfg, &n);
	//cfg.dcb.BaudRate = baud;
	cfg.dcb.BaudRate = 115200;
	cfg.dcb.fBinary = TRUE;
	cfg.dcb.fParity = FALSE;
	cfg.dcb.fOutxCtsFlow = FALSE;
	cfg.dcb.fOutxDsrFlow = FALSE;
	cfg.dcb.fOutX = FALSE;
	cfg.dcb.fInX = FALSE;
	cfg.dcb.fErrorChar = FALSE;
	cfg.dcb.fNull = FALSE;
	cfg.dcb.fRtsControl = RTS_CONTROL_ENABLE;
	cfg.dcb.fAbortOnError = FALSE;
	cfg.dcb.ByteSize = 8;
	cfg.dcb.Parity = NOPARITY;
	cfg.dcb.StopBits = ONESTOPBIT;
	cfg.dcb.fDtrControl = DTR_CONTROL_ENABLE;
	SetCommConfig(fd, &cfg, n);
	GetCommTimeouts(fd, &timeout);
	timeout.ReadIntervalTimeout = 0;
	timeout.ReadTotalTimeoutMultiplier = 0;
	timeout.ReadTotalTimeoutConstant = 1000;
	timeout.WriteTotalTimeoutConstant = 0;
	timeout.WriteTotalTimeoutMultiplier = 0;
	SetCommTimeouts(fd, &timeout);
#endif
	return fd;

}



void close_port(PORTTYPE port)
{
#if defined(MACOSX) || defined(LINUX)
	close(port);
#elif defined(WINDOWS)
	CloseHandle(port);
#endif
}


/**********************************/
/*  Misc. Functions               */
/**********************************/

void delay(double sec)
{
#if defined(MACOSX) || defined(LINUX)
	usleep(sec * 1000000);
#elif defined(WINDOWS)
	Sleep(sec * 1000);
#endif
}


void die(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	exit(1);
}

#if defined(WINDOWS)
static const unsigned __int64 epoch = ((unsigned __int64) 116444736000000000ULL);
int
gettimeofday_(struct timeval * tp, struct timezone * tzp)
{
    FILETIME    file_time;
    SYSTEMTIME  system_time;
    ULARGE_INTEGER ularge;

    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    ularge.LowPart = file_time.dwLowDateTime;
    ularge.HighPart = file_time.dwHighDateTime;

    tp->tv_sec = (long) ((ularge.QuadPart - epoch) / 10000000L);
    tp->tv_usec = (long) (system_time.wMilliseconds * 1000);

    return 0;
}
#endif