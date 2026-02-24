#include "tty_man.h"
#include <errno.h>

/* Fragments of that code were rewritten from the book:
*  "Advanced Programming in the UNIX® Environment: Second Edition"
*  By W. Richard Stevens, Stephen A. Rago
*/

int save_termios(int fd, struct termios* termios) {
   struct termios stermios;

   if (tcgetattr(fd, &stermios) < 0) {
      fprintf(stderr, "Failed to get termios attributes on fd = %d", fd);
      return TERMIOS_GETATTR_ERR;
   }

   *termios = stermios;
   return 0;
}  

int save_winsize(int fd, struct winsize* winsize) {
   struct winsize swinsize;

   if (ioctl(STDIN_FILENO, TIOCGWINSZ, (char*)&swinsize) < 0) {
      fprintf(stderr, "Failed to get winsize attribute on fd = %d", fd);
      return WINSIZE_GETATTR_ERR;
   }

   return 0;
}

void set_noecho(int fd)
{
   struct termios stermios;

   if (tcgetattr(fd, &stermios) < 0)
      fprintf(stderr, "tcgetattr error");
   
   stermios.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
   
   /*
   * Also turn off NL to CR/NL mapping on output.
   */
   stermios.c_oflag &= ~ONLCR;

   if (tcsetattr(fd, TCSANOW, &stermios) < 0)
      fprintf(stderr, "tcsetattr error");
}

int tty_raw(int fd)
{
   int err;
   struct termios buf, stermios;

   if ((err = save_termios(fd, &buf)) < 0)
      return err;

   stermios = buf;

   /*
   * Echo off, canonical mode off, extended input
   * processing off, signal chars off.
   */
   buf.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
   /*
   * No SIGINT on BREAK, CR-to-NL off, input parity
   * check off, don't strip 8th bit on input, output
   * flow control off.
   */
   buf.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
   /*
   * Clear size bits, parity checking off.
   */
   buf.c_cflag &= ~(CSIZE | PARENB);
   /*
   * Set 8 bits/char.
   */
   buf.c_cflag |= CS8;
   /*
   * Output processing off.
   */
   buf.c_oflag &= ~(OPOST);
   /*
   * Case B: 1 byte at a time, no timer.
   */
   buf.c_cc[VMIN] = 1;
   buf.c_cc[VTIME] = 0;

   if (tcsetattr(fd, TCSAFLUSH, &buf) < 0)
      return TERMIOS_SETATTR_ERR;

   /*
   * Verify that the changes stuck. tcsetattr can return 0 on
   * partial success.
   */
   if (save_termios(fd, &buf) < 0) {
      err = errno;
      tcsetattr(fd, TCSAFLUSH, &stermios);
      errno = err;
      return TERMIOS_SETATTR_ERR;
   }
   
   if ((buf.c_lflag & (ECHO | ICANON | IEXTEN | ISIG)) ||
       (buf.c_iflag & (BRKINT | ICRNL | INPCK | ISTRIP | IXON)) ||
       (buf.c_cflag & (CSIZE | PARENB | CS8)) != CS8 ||
       (buf.c_oflag & OPOST) || buf.c_cc[VMIN] != 1 ||
        buf.c_cc[VTIME] != 0) {
      /*
      * Only some of the changes were made. Restore the
      * original settings.
      */
      tcsetattr(fd, TCSAFLUSH, &stermios);
      errno = EINVAL;
      return TERMIOS_SETATTR_ERR;
   }

   return 0;
}

int tty_set(int fd, struct termios* termios, struct winsize* winsize) {
   if (termios != NULL && 
       tcsetattr(fd, TCSAFLUSH, termios) < 0) {
      fprintf(stderr, "Failed to set termios attributes on fd = %d", fd);
      return TERMIOS_SETATTR_ERR;
   }

   if (winsize != NULL && 
       ioctl(fd, TIOCSWINSZ, winsize) < 0) {
      fprintf(stderr, "Failed to set winsize attribute on fd = %d", fd);
      return WINSIZE_SETATTR_ERR;
   }

   return 0;
}