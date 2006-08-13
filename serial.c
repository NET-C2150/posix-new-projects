/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* serial port functions
 *
 * Author: Chris Toshok <toshok@ximian.com>
 */

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/ioctl.h>

#include <glib.h>

/* This is for FIONREAD on solaris */
#if defined(sun)
#include <sys/filio.h>
#endif

/* This is a copy of System.IO.Ports.Handshake */
typedef enum {
	NoneHandshake = 0,
	XOnXOff = 1,
	RequestToSend = 2,
	RequestToSendXOnXOff = 3
} MonoHandshake;

/* This is a copy of System.IO.Ports.Parity */
typedef enum {
	NoneParity = 0,
	Odd = 1,
	Even = 2,
	Mark = 3,
	Space = 4
} MonoParity;

/* This is a copy of System.IO.Ports.StopBits */
typedef enum {
	NoneStopBits = 0,
	One = 1,
	Two = 2,
	OnePointFive = 3
} MonoStopBits;

/* This is a copy of System.IO.Ports.SerialSignal */
typedef enum {
	NoneSignal,
	Cd = 1, /* Carrier detect */
	Cts = 2, /* Clear to send */
	Dsr = 4, /* Data set ready */
	Dtr = 8, /* Data terminal ready */
	Rts = 16  /* Request to send */
} MonoSerialSignal;

int
open_serial (char* devfile)
{
	int fd;
	struct termios newtio;

	fd = open (devfile, O_RDWR);

	if (fd == -1)
		return -1;

	newtio.c_cflag = CLOCAL | CREAD;
	newtio.c_iflag = 0;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;

	tcflush(fd, TCIOFLUSH);
	tcsetattr(fd,TCSANOW,&newtio);

	return fd;
}

void
close_serial (int unix_fd)
{
	close (unix_fd);
}

guint32
read_serial (int fd, guchar *buffer, int offset, int count)
{
	guint32 n;
 
	n = read (fd, buffer + offset, count);

	return (guint32) n;
}

void
write_serial (int fd, guchar *buffer, int offset, int count, int timeout)
{
	guint32 n;

	struct pollfd ufd;

	ufd.fd = fd;
	ufd.events = POLLHUP | POLLOUT | POLLERR;

	poll (&ufd, 1, timeout);

	if ((ufd.revents & POLLOUT) != POLLOUT) {
		return;
	}
 
	n = write (fd, buffer + offset, count);
}

void
discard_buffer (int fd, gboolean input)
{
	tcflush(fd, input ? TCIFLUSH : TCOFLUSH);
}

gint32
get_bytes_in_buffer (int fd, gboolean input)
{
	gint32 retval;

	if (ioctl (fd, input ? FIONREAD : TIOCOUTQ, &retval) == -1) {
		return -1;
	}

	return retval;
}

gboolean
set_attributes (int fd, int baud_rate, MonoParity parity, int dataBits, MonoStopBits stopBits, MonoHandshake handshake)
{
	struct termios newtio;

	tcgetattr (fd, &newtio);

	switch (baud_rate) {
	case 230400: baud_rate = B230400; break;
	case 115200: baud_rate = B115200; break;
	case 57600: baud_rate = B57600; break;
	case 38400: baud_rate = B38400; break;
	case 19200: baud_rate = B19200; break;
	case 9600: baud_rate = B9600; break;
	case 4800: baud_rate = B4800; break;
	case 2400: baud_rate = B2400; break;
	case 1800: baud_rate = B1800; break;
	case 1200: baud_rate = B1200; break;
	case 600: baud_rate = B600; break;
	case 300: baud_rate = B300; break;
	case 200: baud_rate = B200; break;
	case 150: baud_rate = B150; break;
	case 134: baud_rate = B134; break;
	case 110: baud_rate = B110; break;
	case 75: baud_rate = B75; break;
	case 50:
	case 0:
	default:
		baud_rate = B9600;
		break;
	}

	switch (parity) {
	case NoneParity: /* None */
		newtio.c_iflag |= IGNPAR;
		newtio.c_cflag &= ~(PARENB | PARODD);
		break;
	case Odd: /* Odd */
		newtio.c_iflag &= ~IGNPAR;
		newtio.c_cflag |= PARENB | PARODD;
		break;
	case Even: /* Even */
		newtio.c_iflag &= ~(IGNPAR | PARODD);
		newtio.c_cflag |= PARENB;
		break;
	case Mark: /* Mark */
		/* XXX unhandled */
		break;
	case Space: /* Space */
		/* XXX unhandled */
		break;
	}

	newtio.c_cflag &= ~CSIZE;
	switch (dataBits) {
	case 5: newtio.c_cflag |= CS5; break;
	case 6: newtio.c_cflag |= CS6; break;
	case 7: newtio.c_cflag |= CS7; break;
	case 8:
	default:
		newtio.c_cflag |= CS8;
		break;
	}

	newtio.c_cflag &= ~CSTOPB;
	switch (stopBits) {
	case NoneStopBits:
		/* Unhandled */
		break;
	case One: /* One */
		/* do nothing, the default is one stop bit */
		break;
	case Two: /* Two */
		newtio.c_cflag |= CSTOPB;
		break;
	case OnePointFive: /* OnePointFive */
		/* XXX unhandled */
		break;
	}

	newtio.c_iflag &= ~IXOFF;
	newtio.c_oflag &= ~IXON;
#ifdef CRTSCTS
	newtio.c_cflag &= ~CRTSCTS;
#endif /* def CRTSCTS */
	switch (handshake) {
	case NoneHandshake: /* None */
		/* do nothing */
		break;
	case RequestToSend: /* RequestToSend (RTS) */
#ifdef CRTSCTS
		newtio.c_cflag |= CRTSCTS;
#endif /* def CRTSCTS */
		break;
	case RequestToSendXOnXOff: /* RequestToSendXOnXOff (RTS + XON/XOFF) */
#ifdef CRTSCTS
		newtio.c_cflag |= CRTSCTS;
#endif /* def CRTSCTS */
		/* fall through */
	case XOnXOff: /* XOnXOff */
		newtio.c_iflag |= IXOFF;
		//		newtio.c_oflag |= IXON;
		break;
	}
	
	if (cfsetospeed (&newtio, baud_rate) < 0 || cfsetispeed (&newtio, baud_rate) < 0 ||
			tcsetattr (fd, TCSADRAIN, &newtio) < 0)
		return FALSE;

	return TRUE;
}

static gint32
get_signal_code (MonoSerialSignal signal)
{
	switch (signal) {
		case Cd:
			return TIOCM_CAR;
		case Cts:
			return TIOCM_CTS;
		case Dsr:
			return TIOCM_DSR;
		case Dtr:
			return TIOCM_DTR;
		case Rts:
			return TIOCM_RTS;
		default:
			return 0;
	}

	/* Not reached */
	return 0;
}

static MonoSerialSignal
get_mono_signal_codes (int signals)
{
	MonoSerialSignal retval = NoneSignal;

	if ((signals & TIOCM_CAR) != 0)
		retval |= Cd;
	if ((signals & TIOCM_CTS) != 0)
		retval |= Cts;
	if ((signals & TIOCM_DSR) != 0)
		retval |= Dsr;
	if ((signals & TIOCM_DTR) != 0)
		retval |= Dtr;
	if ((signals & TIOCM_RTS) != 0)
		retval |= Rts;

	return retval;
}

MonoSerialSignal
get_signals (int fd, gint32 *error)
{
	int signals;

	*error = 0;
	
	if (ioctl (fd, TIOCMGET, &signals) == -1) {
		*error = -1;
		return NoneSignal;
	}
	
	return get_mono_signal_codes (signals);
}

gint32
set_signal (int fd, MonoSerialSignal signal, gboolean value)
{
	int signals, expected, activated;

	expected = get_signal_code (signal);
	if (ioctl (fd, TIOCMGET, &signals) == -1)
		return -1;
	
	activated = (signals & expected) != 0;
	if (activated == value) /* Already set */
		return 1;
	
	if (value)
		signals |= expected;
	else
		signals &= ~expected;
	
	if (ioctl (fd, TIOCMSET, &signals) == -1)
		return -1;
	
	return 1;
}

gboolean
poll_serial (int fd, gint32 *error)
{
	struct pollfd pinfo;
	
	*error = 0;
	
	pinfo.fd = fd;
	pinfo.events = POLLIN;
	pinfo.revents = 0;

	if (poll (&pinfo, 1, 0) == -1) {
		*error = -1;
		return FALSE;
	}

	return (pinfo.revents & POLLIN) != 0 ? 1 : 0;
}

/*
 * mono internals should not be used here.
 * this serial stuff needs to be implemented with icalls.
 * make this at least compile until the code is moved elsewhere
 * defined(linux) is wrong, too
 */
void*
list_serial_devices (void)
{
	return NULL;
}

#if 0
MonoArray *
list_serial_devices (void)
{
	MonoArray *array;
#if defined(linux)
	/* Linux serial files are of the form ttyS[0-9]+ */
	GSList *l, *list = NULL;
	GDir* dir = g_dir_open ("/dev", 0, NULL);
	const char *filename;
	int i = 0;

	while ((filename = g_dir_read_name (dir))) {
		if (filename) {
			if (!strncmp (filename, "ttyS", 4))
				list = g_slist_append (list, g_strconcat ("/dev/", filename, NULL));
		}
	}

	g_dir_close (dir);
  
	array = mono_array_new (mono_domain_get (), mono_get_string_class (), g_slist_length (list));
	for (l = list; l; l = l->next) {
		mono_array_set (array, gpointer, i++, mono_string_new (mono_domain_get (), (char*)l->data));
		g_free (l->data);
	}

	g_slist_free (list);

#else
#warning "list_serial_devices isn't ported to this OS"
#endif

	return array;
}
#endif

