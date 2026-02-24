#define _XOPEN_SOURCE

#include "Defines.hpp"
#include "Assert.hpp"

#include <iostream>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <signal.h>
#include <poll.h>

#ifndef _HAS_OPENPT
int posix_openpt(int oflag)
{
   int fdm;
   fdm = open("/dev/ptmx", oflag);
   return fdm;
}
#endif

#ifndef _HAS_PTSNAME
char* ptsname(int fdm)
{
   int sminor;
   static char pts_name[16];

   if (ioctl(fdm, TIOCGPTN, &sminor) < 0)
      return NULL;

   snprintf(pts_name, sizeof(pts_name), "/dev/pts/%d", sminor);
   return pts_name;
}
#endif

#ifndef _HAS_GRANTPT
int grantpt(int fdm)
{
   char *pts_name;
   pts_name = ptsname(fdm);
   return chmod(pts_name, S_IRUSR | S_IWUSR | S_IWGRP);
}
#endif

#ifndef _HAS_UNLOCKPT
int unlockpt(int fdm)
{
   int lock = 0;
   return ioctl(fdm, TIOCSPTLCK, &lock);
}
#endif

int ptym_open(char *pts_name, int pts_namesz)
{
   char *ptr;
   int fdm;

   /*
   * Return the name of the master device so that on failure
   * the caller can print an error message. Null terminate
   * to handle case where string length > pts_namesz.
   */
   strncpy(pts_name, "/dev/ptmx", pts_namesz);
   pts_name[pts_namesz - 1] = '\0';

   if ((fdm = posix_openpt(O_RDWR)) < 0)
      return -1;

   if (grantpt(fdm) < 0) { /* grant access to slave */
      close(fdm);
      return -2;
   }

   if (unlockpt(fdm) < 0) { /* clear slave's lock flag */
      close(fdm);
      return -3;
   }
   
   if ((ptr = ptsname(fdm)) == NULL) { /* get slave's name */
      close(fdm);
      return -4;
   }

   /*
   * Return name of slave. Null terminate to handle case
   * where strlen(ptr) > pts_namesz.
   */
   strncpy(pts_name, ptr, pts_namesz);
   pts_name[pts_namesz - 1] = '\0';

   return fdm; /* return fd of master */
}

int ptys_open(char *pts_name)
{
   int fds;

   if ((fds = open(pts_name, O_RDWR)) < 0)
      return -5;

   return fds;
}

pid_t pty_fork(int* ptrfdm, char* slave_name, int slave_namesz,
               const struct termios* slave_termios,
               const struct winsize* slave_winsize)
{
   int fdm, fds;
   pid_t pid;
   char pts_name[20];

   if ((fdm = ptym_open(pts_name, sizeof(pts_name))) < 0) {
      THR_HARD_ASSERT_LOG(false, "Failed to open master pty");
   }

   if (slave_name != NULL) {
      /*
      * Return name of slave. Null terminate to handle case
      * where strlen(pts_name) > slave_namesz.
      */
      strncpy(slave_name, pts_name, slave_namesz);
      slave_name[slave_namesz - 1] = '\0';
   }

   if ((pid = fork()) < 0) {
      return -1;
   } 
   else if (pid == 0) { /* child */
      if (setsid() < 0)
         fprintf(stderr, "setsid error");
         
      /*
      * System V acquires controlling terminal on open().
      */
      if ((fds = ptys_open(pts_name)) < 0)
         fprintf(stderr, "Can't open slave pty");

      close(fdm); /* all done with master in child */
      
#if defined(TIOCSCTTY)
      /*
      * TIOCSCTTY is the BSD way to acquire a controlling terminal.
      */
      if (ioctl(fds, TIOCSCTTY, nullptr) < 0)
         fprintf(stderr, "TIOCSCTTY error");
#endif

      /*
      * Set slave's termios and window size.
      */
      if (slave_termios != NULL) {
         if (tcsetattr(fds, TCSANOW, slave_termios) < 0)
            fprintf(stderr, "tcsetattr error on slave pty");
      }

      if (slave_winsize != NULL) {
         if (ioctl(fds, TIOCSWINSZ, slave_winsize) < 0)
            fprintf(stderr, "TIOCSWINSZ error on slave pty");
      }
 
      /*
      * Slave becomes stdin/stdout/stderr of child.
      */
      if (dup2(fds, STDIN_FILENO) != STDIN_FILENO)
         fprintf(stderr, "dup2 error to stdin");

      if (dup2(fds, STDOUT_FILENO) != STDOUT_FILENO)
         fprintf(stderr, "dup2 error to stdout");
      
      if (dup2(fds, STDERR_FILENO) != STDERR_FILENO)
         fprintf(stderr, "dup2 error to stderr");

      if (fds != STDIN_FILENO && fds != STDOUT_FILENO && fds != STDERR_FILENO)
         close(fds);

      return 0; /* child returns 0 just like fork() */
   } 
   else { /* parent */
      *ptrfdm = fdm; /* return fd of master */
      return pid; /* parent returns pid of child */
   }
}

#define BUFFSIZE 512

static void sig_term(int);
static volatile sig_atomic_t sigcaught; /* set by signal handler */

int writen(int fd, const void* buff, int n)
{
   ssize_t written = 0;
   ssize_t left = n;

   const char* ptr = (const char*)buff;

   while (left > 0) {
      if ((written = write(fd, ptr, left)) < 0) {
         if (left == n)
            return -1;
         else 
            break;
      } 
      else if (written == 0) {
         break;
      }

      ptr += written;
      left -= written;
   }

   return n - left;
}

void loop(int ptym)
{
   int nread;
   char buf[BUFFSIZE];

   struct pollfd pfds[2];

   /* We're copying from stdin and ptym */
   pfds[0].fd = ptym;
   pfds[0].events = POLLIN;
   pfds[1].fd = STDIN_FILENO;
   pfds[1].events = POLLIN;

   for (;;) {
      if (poll(pfds, 2, -1) <= 0)
         break;

      if (pfds[0].revents & (POLLIN | POLLHUP | POLLERR)) { /* copies stdin to ptym */
         if ((nread = read(pfds[0].fd, buf, sizeof(buf))) <= 0)
            break;

         if (writen(STDOUT_FILENO, buf, nread) != nread)
            fprintf(stderr, "writen error to stdout");
      }

      if (pfds[1].revents & (POLLIN | POLLHUP | POLLERR)) { /* copies ptym to stdout */
         if ((nread = read(pfds[1].fd, buf, sizeof(buf))) < 0)
            fprintf(stderr, "read error from stdin");

         else if (nread == 0)
            break; /* EOF on stdin means we're done */

         if (writen(ptym, buf, nread) != nread)
            fprintf(stderr, "writen error to master pty");
      }   
   }

   /*
   *  We should terminate.
   */
}

/*
 * The child sends us SIGTERM when it gets EOF on the pty slave or
 * when read() fails. We probably interrupted the read() of ptym.
 */
static void sig_term(int)
{
   printf("Catching SIGTERM signal");
   sigcaught = 1; /* just set flag and return */
}

static void set_noecho(int fd) /* turn off echo (for slave pty) */
{
   struct termios stermios;

   if (tcgetattr(fd, &stermios) < 0)
      fprintf(stderr, "tcgetattr error");
   
   stermios.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
   
   /*
   * Also turn off NL to CR/NL mapping on output.
   */
   //stermios.c_oflag &= ~ONLCR;

   if (tcsetattr(fd, TCSANOW, &stermios) < 0)
      fprintf(stderr, "tcsetattr error");
}

static struct termios save_termios;
static int ttysavefd = -1;

enum ttyState { 
   RESET, 
   RAW,
}; 

static ttyState ttystate = RESET;

int tty_raw(int fd) /* put terminal into a raw mode */
{
   int err;
   struct termios buf;

   if (ttystate != RESET) {
      errno = EINVAL;
      return(-1);
   }

   if (tcgetattr(fd, &buf) < 0)
      return(-1);

   save_termios = buf; /* structure copy */
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
      return(-1);

   /*
   * Verify that the changes stuck. tcsetattr can return 0 on
   * partial success.
   */
   if (tcgetattr(fd, &buf) < 0) {
      err = errno;
      tcsetattr(fd, TCSAFLUSH, &save_termios);
      errno = err;
      return(-1);
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
      tcsetattr(fd, TCSAFLUSH, &save_termios);
      errno = EINVAL;
      return(-1);
   }

   ttystate = RAW;
   ttysavefd = fd;

   return(0);
}

int tty_reset(int fd) /* restore terminal's mode */
{
   if (ttystate == RESET)
      return(0);
   
   if (tcsetattr(fd, TCSAFLUSH, &save_termios) < 0)
      return(-1);  

   ttystate = RESET;
   return(0);
}

void tty_atexit() /* can be set up by atexit(tty_atexit) */
{
   if (ttysavefd >= 0)
      tty_reset(ttysavefd);
}

int main(int argc, char* argv[])
{
   int fdm, interactive, noecho, verbose;
   pid_t pid;
   char slave_name[20];

   struct termios orig_termios;
   struct winsize size;

   interactive = isatty(STDIN_FILENO);
   noecho = 0;
   verbose = 1;

   if (interactive) { /* fetch current termios and window size */
      if (tcgetattr(STDIN_FILENO, &orig_termios) < 0)
         fprintf(stderr, "tcgetattr error on stdin");

      if (ioctl(STDIN_FILENO, TIOCGWINSZ, (char*)&size) < 0)
         fprintf(stderr, "TIOCGWINSZ error");

      pid = pty_fork(&fdm, slave_name, sizeof(slave_name),
                     &orig_termios, &size);
   } 
   else {
      pid = pty_fork(&fdm, slave_name, sizeof(slave_name),
                     NULL, NULL);
   }

   if (pid < 0) {
      THR_HARD_ASSERT_LOG(false, "fork error");
   } 
   else if (pid == 0) { /* child */ 
      if (noecho)
         set_noecho(STDIN_FILENO); /* stdin is slave pty */
      
      char shell[] = "zsh";

      if (execlp(shell, "-zsh", "-l", (char*)0) < 0) {
         fprintf(stderr, "Can't execute: %s", shell);
         exit(-1);
      }
   }

   if (verbose) {
      fprintf(stderr, "slave name = %s\n", slave_name);
   }

   fprintf(stderr, "Interactive = %d\n", interactive);

   if (interactive) {
      if (tty_raw(STDIN_FILENO) < 0) /* user's tty to raw mode */
         fprintf(stderr, "tty_raw error");

      if (atexit(tty_atexit) < 0) /* reset user's tty on exit */
         fprintf(stderr, "atexit error");
   }

   loop(fdm); /* copies stdin -> ptym, ptym -> stdout */

   exit(0);
}
