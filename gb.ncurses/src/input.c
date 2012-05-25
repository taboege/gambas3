/*
 * input.c - gb.ncurses opaque input routines
 *
 * Copyright (C) 2012 Tobias Boege <tobias@gambas-buch.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#define __INPUT_C

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <linux/kd.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

#include <ncurses.h>

#include "gambas.h"
#include "gb_common.h"

#include "main.h"
#include "input.h"
#include "c_window.h"

#define E_UNSUPP	"Unsupported value"
#define E_NO_NODELAY	"Could not initialise NoDelay mode"

static int _input = -1;
static char _watching = 0;

/* Note that this is not safe for functions that are used to change the mode
 * settings, in particular INPUT_init_nodelay() and INPUT_exit_nodelay(),
 * because @_input is updated after this function */
#define IN_NODELAY	(_input == INPUT_NODELAY)

static struct {
	struct {
		struct termios term;
		int kbmode;
		void (*error_hook)();
	} old;
	int fd;
	unsigned short pressed;
	unsigned int delay;
	GB_TIMER *timer;
} no_delay;
static char _exiting_nodelay = 0;

/**
 * Input initialisation
 */
int INPUT_init()
{
	INPUT_mode(INPUT_CBREAK);
	INPUT_repeater_delay(100);
	return 0;
}

/**
 * Input cleanup
 */
void INPUT_exit()
{
	/* If we are still in NoDelay mode, exit it */
	if (IN_NODELAY) {
		INPUT_mode(INPUT_CBREAK);
	}
}

/**
 * Begin or stop watching the input queue in question depending on @_input
 */
static int INPUT_watch(char start)
{
	int fd = IN_NODELAY ? no_delay.fd : 0;

	if (!start && !_watching)
		return 0;
	if (start && _watching)
		INPUT_watch(!start);
	GB.Watch(fd, start ? GB_WATCH_READ : GB_WATCH_NONE,
		INPUT_callback, 0);
	_watching = start;
	return 0;
}

/**
 * Function to be called by Gambas when data arrives
 */
static void INPUT_callback(int fd, int flag, intptr_t arg)
{
	WINDOW_raise_read(NULL);
}

/**
 * Return if the given fd can be used with console_ioctls
 * @fd: file descriptor to test
 * The idea was derived from "kbd" package, getfd.c, is_a_console()
 */
static inline char is_cons(int fd)
{
	char type;

	if (fd != -1 && isatty(fd) && ioctl(fd, KDGKBTYPE, &type) != -1
	    && (type == KB_101 || type == KB_84))
		return 1;
	return 0;
}

/**
 * Returns an fd that can be used with console_ioctls or -1 if none
 * available
 */
int INPUT_consolefd()
{
	int fd;

	if (is_cons(0))
		return 0;

	fd = open("/dev/tty", O_RDWR);
	if (fd == -1)
		return -1;
	if (is_cons(fd))
		return fd;

	close(fd);
	return -1;
}

/**
 * Init NoDelay mode
 * We save old settings and prepare the TTY driver and Gambas
 */
static int INPUT_init_nodelay()
{
	int fd = INPUT_consolefd();
	struct termios term;

	if (fd == -1)
		return -1;

	/* TODO: implement switching between vts, need available signals to
	 * be sent */

	tcgetattr(fd, &no_delay.old.term);
	ioctl(fd, KDGKBMODE, &no_delay.old.kbmode);

	memcpy(&term, &no_delay.old.term, sizeof(term));
	term.c_lflag &= ~(ICANON | ECHO | ISIG);
	term.c_iflag &= ~(ISTRIP | IGNCR | ICRNL | INLCR | IXOFF | IXON);
	/* Have no timeout per default */
	term.c_cc[VMIN] = 0;
	term.c_cc[VTIME] = -1;
	tcsetattr(fd, TCSAFLUSH, &term);
	no_delay.old.error_hook = GB.Hook(GB_HOOK_ERROR,
		INPUT_nodelay_error_hook);

	no_delay.fd = fd;

	no_delay.timer = NULL;

	/* Switch to K_MEDIUMRAW now. Could not have been done on-the-fly
	 * when reading from the console fd, because our key repeat code
	 * relies on data maybe present on that fd (to determine if we can
	 * safely inject new events for the currently pressed key or shall
	 * examine if there is another keypress) and there wouldn't be
	 * anything if we switch on-the-fly */
	ioctl(no_delay.fd, KDSKBMODE, K_MEDIUMRAW);

	return 0;
}

/**
 * Cleanup NoDelay mode
 * Restore old settings
 * This assumes that @_input reflects the current settings
 */
static int INPUT_exit_nodelay()
{
	/* @_input must be updated after this function, if even, after this
	 * function */
	if (!IN_NODELAY)
		return 0;
	if (_exiting_nodelay)
		return 0;

	_exiting_nodelay = 1;
	ioctl(no_delay.fd, KDSKBMODE, no_delay.old.kbmode);
	tcsetattr(no_delay.fd, TCSANOW, &no_delay.old.term);
	GB.Hook(GB_HOOK_ERROR, no_delay.old.error_hook);
	if (no_delay.timer)
		GB.Unref((void **) &no_delay.timer);
	close(no_delay.fd);
	_exiting_nodelay = 0;
	return 0;
}

/**
 * The NoDelay mode error hook
 * This calls the former error hook, saved by INPUT_init_nodelay() to not
 * disturb any piece code
 */
static void INPUT_nodelay_error_hook()
{
	if (_exiting_nodelay)
		return;
	INPUT_exit_nodelay();
	no_delay.old.error_hook();
}

/**
 * Return or set the repeater delay
 * @val: value to set the delay to. This value must be at least 1 or an
 *       error is returned. If it is REPEATER_RETURN, the current value is
 *       returned to the caller.
 * Note that this setting affects the repeater function itself, that gets
 * called in this interval to generate events and the INPUT_get_nodelay()
 * function which will wait to return the amount of milliseconds if it is to
 * return the pressed key.
 */
int INPUT_repeater_delay(int val)
{
	if (val == REPEATER_RETURN)
		return no_delay.delay;
	if (val < 1)
		return -1;
	no_delay.delay = (unsigned int) val;
	return 0;
}

/**
 * NoDelay mode event repeater
 * Used to insert Window_Read events if there is a key pressed
 */
static int INPUT_nodelay_repeater()
{
	fprintf(stderr, "here\n");
	if (!no_delay.pressed)
		return TRUE;
	WINDOW_raise_read(NULL);
	return FALSE;
}

/**
 * Return or set the current input mode
 * @mode: one of INPUT_* enums
 */
int INPUT_mode(int mode)
{
	if (mode == INPUT_RETURN)
		return _input;

	if (mode == _input)
		return 0;

	INPUT_watch(0);

	if (_input == INPUT_NODELAY)
		INPUT_exit_nodelay();

	switch (mode) {
		case INPUT_COOKED:
			noraw();
			nocbreak();
			break;
		case INPUT_CBREAK:
			noraw();
			cbreak();
			break;
		case INPUT_RAW:
			nocbreak();
			raw();
			break;
		case INPUT_NODELAY:
			if (INPUT_init_nodelay() == -1) {
				GB.Error(E_NO_NODELAY);
				/* We return 0 to not override the previous
				 * error message with the one emitted by the
				 * caller if we return error */
				return 0;
			}
			break;
		default:
			GB.Error(E_UNSUPP);
			return -1;
	}
	_input = mode;

	INPUT_watch(1);

	return 0;
}

/**
 * Retrieve input from ncurses
 */
static int INPUT_get_ncurses(int timeout)
{
	int ret;

	if (timeout >= 0)
		timeout(timeout);
	ret = getch();
	if (ret == ERR) {
		/* Had a timeout, the manual doesn't define any errors to
		   happen for wgetch() besides NULL pointer arguments. The
		   only source of ERR is timeout expired. */
		if (timeout >= 0)
			ret = 0;
	}

	if (timeout >= 0)
		timeout(-1);
	return ret;
}

/*
 * Return codes from INPUT_trans_keycode()
 */
enum {
	TRANS_NEED_MORE = -1,
	TRANS_KEY_MIN
};

/*
 * States of the modifier keys that we recognise
 */
enum {
	MOD_NONE = 0,
	MOD_SHIFT = 1,
	MOD_CTRL = 2,
	MOD_ALT = 4
};

#define IS_BREAK(k)	((k) & 0x80)

/**
 * Translate a keycode (or a sequence) to an ncurses compatible int
 * @kc: keycode to translate
 * This function returns one of the above codes. TRANS_NEED_MORE means that
 * the given @kc is considered part of a multi-keycode sequence (or is a
 * Shift, Control, Alt), so we need more keys which are (in first case
 * likely) available without waiting from the tty driver then; in the latter
 * case (modifier keys), it does not count as a keypress anyway.
 * Note that after a usual tty key sequence is assembled it is passed to the
 * driver to try to get an escape sequence for it. If there is no escape
 * seqeuence for that key, we can simply return the plain value. Otherwise
 * we exploit ncurses key_defined() routine to translate the sequence to an
 * int for us. This gives an int like ncurses getch() would do.
 */
static int INPUT_trans_keycode(unsigned char kc)
{
	/* Pause/Break has the largest scancode: e1 1d 45 e1 9d c5 */
	static unsigned char codes[8];
	static int num = 0;
	static int modifiers = MOD_NONE;

	unsigned char seq[8];
	struct kbentry kbe;
	struct kbsentry kbs;
	register int mod;

/* TODO: Hope they're fixed */
#define KEYCODE_LCTRL	0x1d
#define KEYCODE_RCTRL	0x61
#define KEYCODE_ALT	0x38
#define KEYCODE_LSHIFT	0x2a
#define KEYCODE_RSHIFT	0x36
	/* Modifiers */
	switch (kc) {
	case KEYCODE_LCTRL:
	case KEYCODE_RCTRL:
		mod = MOD_CTRL;
		goto apply_mod;
	case KEYCODE_ALT:
		mod = MOD_ALT;
		goto apply_mod;
	case KEYCODE_LSHIFT:
	case KEYCODE_RSHIFT:
		mod = MOD_SHIFT;
		goto apply_mod;
	default:
		goto account_key;
	}
apply_mod:
	if (IS_BREAK(kc))
		modifiers &= ~mod;
	else
		modifiers |= mod;
	return TRANS_NEED_MORE;

account_key:
	codes[num++] = kc;
	/* Break key, sends make and break code together */
	if (codes[0] == '\xe1') {
		if (num == 6)
			return KEY_BREAK;
		else
			return TRANS_NEED_MORE;
	}
	/* Keys with two keycodes, no matter, those correspond to
	 * single-keycode keys, we can safely use @kc */
	if (codes[0] == '\xe0' && num != 2)
		return TRANS_NEED_MORE;

	/* TODO: what to do with ctrl- ? */

	/* Set table and get action code */
	if (modifiers & MOD_ALT) {
		if (modifiers & MOD_SHIFT)
			kbe.kb_table = K_ALTSHIFTTAB;
		else
			kbe.kb_table = K_ALTTAB;
	} else if (modifiers & MOD_SHIFT) {
		kbe.kb_table = K_SHIFTTAB;
	} else {
		kbe.kb_table = K_NORMTAB;
	}
	kbe.kb_index = kc & 0x7f;
	kbe.kb_value = 0;
	ioctl(no_delay.fd, KDGKBENT, &kbe);
	seq[0] = (unsigned char) kbe.kb_value;
	/* Has an escape sequence? */
	kbs.kb_func = seq[0];
	ioctl(no_delay.fd, KDGKBSENT, &kbs);
	if (kbs.kb_string[0]) {
		strncpy((char *) seq, (char *) kbs.kb_string, 7);
		seq[7] = '\0';
		return key_defined((char *) seq);
	} else {
		return (int) seq[0];
	}
}

/**
 * Change the currently pressed key
 * @key: current key. 0 means that no key is pressed
 * This install the repeater
 */
static void INPUT_change_pressed(int key)
{
	fprintf(stderr, "%d\n", no_delay.delay);
	if (key == 0) {
		GB.Unref((void **) no_delay.timer);
		no_delay.timer = NULL;
	} else {
		no_delay.timer = GB.Every(no_delay.delay,
			(GB_TIMER_CALLBACK) INPUT_nodelay_repeater, 0);
	}
	no_delay.pressed = key;
}

/**
 * Retrieve input in NoDelay mode, using console_ioctl(4).
 * If there is a key pressed and no input available on the input queue,
 * the pressed key is returned.
 * @timeout: timeout in milliseconds. If that timeout expires, we return 0.
 * Note that @timeout can only be in the scope of deciseconds and is
 * silently cut down to those.
 * The Repeater Delay applies to this function, too, in that we only return
 * a pressed key when we waited for that delay.
 * Note carefully, that there is an emergency exit: pressing ESC thrice
 * during one second will immediately abort NoDelay mode and enter CBreak.
 */
static int INPUT_get_nodelay(int timeout)
{
	static char esc = 0;
	struct termios old, new;
	unsigned char b;
	static time_t stamp;
	int ret, res, num;
	int key; /* This will already be ncurses compatible */
	struct timeval tv1, tv2;

	if (timeout > -1) {
		gettimeofday(&tv1, NULL);
		tcgetattr(no_delay.fd, &old);
		memcpy(&new, &old, sizeof(new));
	}

recalc_timeout:
#define USEC2MSEC(us)	(us / 1000)
#define MSEC2USEC(ms)	(ms * 1000)
#define MSEC2DSEC(ms)	(ms / 100)
#define DSEC2MSEC(ds)	(ds * 100)
#define SEC2MSEC(s)	(s * 1000)
#define MSEC2SEC(ms)	(ms / 1000)

	if (timeout > -1) {
		gettimeofday(&tv2, NULL);
		timeout -= USEC2MSEC(tv2.tv_usec - tv1.tv_usec);
		timeout -= SEC2MSEC(tv2.tv_sec - tv1.tv_sec);
		if (timeout < 0) {
			ret = 0;
			goto cleanup;
		}
	}

	/* Set timeout */
	ioctl(no_delay.fd, TIOCINQ, &num);
	/* We don't need to set any timeout if there are bytes available */
	if (!num && timeout > -1) {
		new.c_cc[VTIME] = MSEC2DSEC(timeout);
		tcsetattr(no_delay.fd, TCSANOW, &new);
		gettimeofday(&tv1, NULL);
	}

	/* Begin reading */
	if (no_delay.pressed && !num) {
		usleep(MSEC2USEC(no_delay.delay));
		ret = no_delay.pressed;
		goto cleanup;
	}
	/* Try to stick to the user-supplied timeout */
	ioctl(no_delay.fd, TIOCINQ, &num);
	if ((res = read(no_delay.fd, &b, 1)) == -1 && errno == EINTR) {
		goto recalc_timeout;
	} else if (res == 0) { /* Timeout expired */
		ret = 0;
		goto cleanup;
	} else { /* Got a key */
		/* Emergency exit from NoDelay mode */
#define KEYCODE_ESC	0x01
		if (b == KEYCODE_ESC) {
			if (time(NULL) - stamp > 0)
				esc = 0;
			if (++esc == 3) {
				INPUT_exit_nodelay();
				INPUT_mode(INPUT_CBREAK);
				return 0;
			}
			stamp = time(NULL);
		}
		/* We use ncurses keys for operations on our no_delay data */
		if ((key = INPUT_trans_keycode(b)) == TRANS_NEED_MORE)
			goto recalc_timeout;
		/* Ignore break codes, except when it is the currently
		   pressed key */
		if (IS_BREAK(b)) {
			if (no_delay.pressed == key)
				INPUT_change_pressed(0);
			/* Key release isn't visible to the gambas programmer
			 * and thus not really an event to gb.ncurses. If
			 * time is left, we try again reading another key */
			goto recalc_timeout;
		} else {
			INPUT_change_pressed(key);
		}
	}

	ret = key;

cleanup:
	if (timeout > -1)
		tcsetattr(no_delay.fd, TCSANOW, &old);
	return ret;
}

/**
 * Get a keypress within the given timeout
 * @timeout: number of milliseconds to wait. If no key is pressed during it,
 *           0 will be returned.
 */
int INPUT_get(int timeout)
{
	if (_input == INPUT_NODELAY)
		return INPUT_get_nodelay(timeout);
	else
		return INPUT_get_ncurses(timeout);
}
