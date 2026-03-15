#include "tty_man.h"
#include <errno.h>

#if defined(THR_PLATFORM_WINDOWS)
// WINDOWS IMPLEMENTATION HERE
#else

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

int interactive_termios(struct termios* buf)
{
   // TODO: check out that

   if (buf == NULL)
      return NULL_POINTER_ERR;

   memset(buf, 0, sizeof(buf));

   buf->c_iflag = (ICRNL | IXON | IUTF8);
   buf->c_oflag = (OPOST | ONLCR);
   buf->c_cflag = (CS8 | CREAD);
   buf->c_lflag = (ISIG | ICANON | ECHO | ECHOE | ECHOK | IEXTEN);

   buf->c_cc[VINTR]    = 0x03;
   buf->c_cc[VQUIT]    = 0x1C;
   buf->c_cc[VKILL]    = 0x15;
   buf->c_cc[VEOF]     = 0x04;
   buf->c_cc[VSTART]   = 0x11;
   buf->c_cc[VSTOP]    = 0x13;
   buf->c_cc[VSUSP]    = 0x1A;
   buf->c_cc[VREPRINT] = 0x12;
   buf->c_cc[VWERASE]  = 0x17;
   buf->c_cc[VLNEXT]   = 0x16;

   buf->c_cc[VMIN]     = 1;
   buf->c_cc[VTIME]    = 0;

   cfsetispeed(buf, B38400);
   cfsetospeed(buf, B38400);

   return 0;
}

int tty_interactive(int fd)
{
   int err;
   struct termios buf, stermios;

   if ((err = save_termios(fd, &buf)) < 0)
      return err;

   stermios = buf;

   int r = 0;

   if ((r = interactive_termios(&buf)) < 0)
      return r;

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
   
   if (!(buf.c_lflag & (ICRNL | IXON | IUTF8)) ||
       !(buf.c_iflag & (OPOST | ONLCR)) ||
       !(buf.c_cflag & (CS8 | CREAD)) ||
       !(buf.c_oflag & (ISIG | ICANON | ECHO | ECHOE | ECHOK | IEXTEN)) || 
        buf.c_cc[VMIN] != 1 ||
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

#endif // THR_PLATFORM_WINDOWS
