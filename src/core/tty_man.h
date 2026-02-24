#ifndef _TTY_MAN_HEADER
#define _TTY_MAN_HEADER

#include "core_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TERMIOS_GETATTR_ERR (-1)
#define TERMIOS_SETATTR_ERR (-2)
#define WINSIZE_GETATTR_ERR (-3)
#define WINSIZE_SETATTR_ERR (-4)

/* Save current termios settings of the 'fd' terminal.
*  Returns 0 on success. On failure, returns negative value.
*/
int save_termios(int fd, struct termios* termios);

/* Save current winsize of the 'fd' terminal.
*  Returns 0 on success. On failure, returns negative value.
*/
int save_winsize(int fd, struct winsize* winsize);

/* Turn off echo for given 'fd' terminal file.
*/ 
void set_noecho(int fd);

/* Put 'fd' terminal into a raw mode.
*  Returns non-negative value on success.
*  On failure, returns negative value.
*/
int tty_raw(int fd);

/* Restore 'fd' terminal's mode.
*  Returns 0 on success. On failure, returns negative value.
*/
int tty_set(int fd, struct termios* termios, struct winsize* winsize);

#ifdef __cplusplus
}
#endif

#endif // _TTY_MAN_HEADER
