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
int transmit_bytes(PORTTYPE port, const char *data, int len);
int receive_bytes(PORTTYPE port, char *buf, int len);
void close_port(PORTTYPE port);
void delay(double sec);
void die(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

int gettimeofday(struct timeval * tp, struct timezone * tzp);


/************************************/
/*  Latency/Delay Test  Functions   */
/************************************/

int kk,loopNum=1;
double do_test(PORTTYPE port, int len)
{
	char buf[8192];
	struct timeval begin, end;
	double elapsed;
	int r;

	if (len > sizeof(buf) || len < 1) return 1000000;
	//memset(buf, '0', len);
	for ( int ii=0; ii<len; ii++)
		buf[ii]='A'+(ii%26);
	buf[len - 1] = 'x'; // end of packet marker
	// test begin
	gettimeofday(&begin, NULL);
	// send the data
	r = transmit_bytes(port, buf, len);
	if (r != len) die("unable to write, r = %d\n", r);
	//printf("write, r = %d\n", r);
	// receive the reply
	r = receive_bytes(port, buf, 4);  // response is always 4 bytes
	// test end
	gettimeofday(&end, NULL);
	elapsed = (double)(end.tv_sec - begin.tv_sec) * 1000.0;
	if (r < 1) printf("@%d error reading len=%d result, r=%d\n",kk ,len , r); // NOT DIE
	else
	{
	buf[4]=0;
	if (memcmp(buf, "012x", 4) != 0) printf("len=%d error: incorrect response buf=%s\n",len , buf); //NOT DIE
	elapsed += (double)(end.tv_usec - begin.tv_usec) / 1000.0;
	//printf("  len=%d, elased: %.2f ms\n", len, elapsed);
	}
	return elapsed;
}

void do_test_100_times(PORTTYPE port, int len)
{
	const int num = 100;
	int maxn=0;
	double ms, total=0, max2=9876543210, max=0;

	for (kk=0; kk<num; kk++) {
		ms = do_test(port, len);
		total += ms;
		if (ms > max) {
			if (ms != max2) max2 = max;
			max = ms;
			maxn++;
		}
		else if (ms == max && max >= max2) maxn++;
	}
	printf("latency @ %4d bytes: ", len);
	printf("%4.2f ms average, ", total / num);
	printf("\t%2d max hits, ", maxn);
	printf("\t%4.2f 2nd max, ", max2);
	printf("\t%4.2f maximum\n", max);
}

// wait for the Arduino board to boot up, since opening
// the board raises DTR, which resets the processor.
// as soon as it properly responds, we know it's running
void wait_online(PORTTYPE port)
{
	char buf[8];
	int r;

	printf("waiting for board to be ready:\n");
	while (1) {
		delay(0.1);
		printf(".");
		fflush(stdout);
		buf[0] = 'x';
		r = transmit_bytes(port, buf, 1);
		if (r != 1) die("unable to write, r = %d\n", r);
		r = receive_bytes(port, buf, 4);
		if (r == 4) break; // success, device online
	}
	printf("ok\n");
}

int lengths[] = { 1,2,12,16,30,31,63,64,65,71,126,127,128,129,500,512,640,1000,1278,1279,1280,1281,2000,2047,2048,2049,4000,4095,4096,4097,8000 };
int numLengths = sizeof( lengths )/sizeof( int );
int main(int argc, char **argv)
{
	PORTTYPE fd;

	if (argc < 2) die("Usage: :) latency_test3 <comport>\n");
	fd = open_port_and_set_baud_or_die(argv[1], BAUD);
	if (argc > 2 && argv[2][0] >'0' && argv[2][0] <= '9' ) loopNum = argv[2][0] - '0';
	if (argc > 3 && argv[3][0] >'0' && argv[3][0] <= '9' ) numLengths = ( numLengths / (argv[3][0] - '0'));
	printf("port %s opened\n", argv[1]);

	wait_online(fd);
	struct timeval begin, end;
	double elapsed;
	int totBytes;

	for ( int jj=0; jj< loopNum; jj++ ) {
		totBytes=0;
		// test begin
		gettimeofday(&begin, NULL);
		for ( int ii=0; ii< numLengths; ii++ ) {
			do_test_100_times( fd, lengths[ii] );
			totBytes += lengths[ii] * 100;
		}
		// test end
		gettimeofday(&end, NULL);
		elapsed = (double)(end.tv_sec - begin.tv_sec) * 1000.0;
		elapsed += (double)(end.tv_usec - begin.tv_usec) / 1000.0;
		printf(" UP ----- pass #%d\t elapsed time %.3f secs for %d bytes\n", 1+jj, elapsed/1000.0, totBytes );
		// test begin
		gettimeofday(&begin, NULL);
		for ( int ii=numLengths-1; ii>= 0; ii-- )
			do_test_100_times( fd, lengths[ii] );
		gettimeofday(&end, NULL);
		elapsed = (double)(end.tv_sec - begin.tv_sec) * 1000.0;
		elapsed += (double)(end.tv_usec - begin.tv_usec) / 1000.0;
		printf(" DOWN --- pass #%d\t elapsed time %.3f secs for %d bytes\n", 1+jj, elapsed/1000.0, totBytes );
	}
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

int receive_bytes(PORTTYPE port, char *buf, int len)
{
	int count=0;
#if defined(MACOSX) || defined(LINUX)
	int r;
	int retry=0;
	//char buf[512];

	if (len > sizeof(buf) || len < 1) return -1;
	// non-blocking read mode
	fcntl(port, F_SETFL, fcntl(port, F_GETFL) | O_NONBLOCK);
	while (count < len) {
		r = read(port, buf + count, len - count);
		//printf("read, r = %d\n", r);
		if (r < 0 && errno != EAGAIN && errno != EINTR) return -1;
		else if (r > 0) count += r;
		else {
			// no data available right now, must wait
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
	while (count < len) {
		r = ReadFile(port, buf + count, len - count, &n, NULL);
		if (!r) die("read error\n");
		if (n > 0) count += n;
		else {
			if (waiting) break;  // 1 sec timeout
			timeout.ReadIntervalTimeout = MAXDWORD;
			timeout.ReadTotalTimeoutMultiplier = MAXDWORD;
			timeout.ReadTotalTimeoutConstant = 1000;
			SetCommTimeouts(port, &timeout);
			waiting = 1;
		}
	}
#endif
	return count;
}


int transmit_bytes(PORTTYPE port, const char *data, int len)
{
#if defined(MACOSX) || defined(LINUX)
	return write(port, data, len);
#elif defined(WINDOWS)
	DWORD n;
	BOOL r;
	r = WriteFile(port, data, len, &n, NULL);
	if (!r) return 0;
	return n;
#endif
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