#ifndef _PTY_HEADER
#define _PTY_HEADER

#include "core_common.h"

#define PTYM_OPENPT_ERR    (-1)
#define PTYM_GRANTPT_ERR   (-2)
#define PTYM_UNLOCKPT_ERR  (-3)
#define PTYM_PTSNAME_ERR   (-4)
#define PTYS_OPEN_ERR      (-5)
#define PTYF_FORK_ERR      (-6)
#define PTYF_MPTY_OPEN_ERR (-7)
#define PTYF_SETSID_ERR    (-8)

#ifdef __cplusplus
extern "C" {
#endif

/* Open available master pseudo-terminal file and
*  return associated slave filename via 'pts_name' buffer.
*  Returns fd of master on success. 
*  On failure, returns negative value.
*/
int ptym_open(char* pts_name, int pts_namesz);

/* Opens associated slave file.
*/
int ptys_open(char* pts_name);

/* Creating a fork and connecting it to the slave pty 'slave_name'.
*  When 'slave_termios' and 'slave_winsize' are not NULL, we're setting
*  terminal termios settings as in 'slave_termios' and 'slave_winsize'.
*/
pid_t pty_fork(int* ptrfdm, char* slave_name, int slave_namesz,
               const struct termios* slave_termios,
               const struct winsize* slave_winsize);

#ifdef __cplusplus
}
#endif

#endif // _PTY_HEADER
