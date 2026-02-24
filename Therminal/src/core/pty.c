#include "pty.h"

/* Fragments of that code were rewritten from the book:
*  "Advanced Programming in the UNIX® Environment: Second Edition"
*  By W. Richard Stevens, Stephen A. Rago
*/

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

int ptym_open(char* pts_name, int pts_namesz)
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
      return PTYM_OPENPT_ERR;

   if (grantpt(fdm) < 0) { /* grant access to slave */
      close(fdm);
      return PTYM_GRANTPT_ERR;
   }

   if (unlockpt(fdm) < 0) { /* clear slave's lock flag */
      close(fdm);
      return PTYM_UNLOCKPT_ERR;
   }
   
   if ((ptr = ptsname(fdm)) == NULL) { /* get slave's name */
      close(fdm);
      return PTYM_PTSNAME_ERR;
   }

   /*
   * Return name of slave. Null terminate to handle case
   * where strlen(ptr) > pts_namesz.
   */
   strncpy(pts_name, ptr, pts_namesz);
   pts_name[pts_namesz - 1] = '\0';

   return fdm; /* return fd of master */
}

int ptys_open(char* pts_name)
{
   int fds;

   if ((fds = open(pts_name, O_RDWR)) < 0)
      return PTYS_OPEN_ERR;

   return fds;
}

pid_t pty_fork(int* ptrfdm, char* slave_name, int slave_namesz,
               const struct termios* slave_termios,
               const struct winsize* slave_winsize)
{
   int fdm, fds, err;
   pid_t pid;
   char pts_name[20];

   if ((fdm = ptym_open(pts_name, sizeof(pts_name))) < 0) {
      fprintf(stderr, "Failed to open master pty");
      return PTYF_MPTY_OPEN_ERR;
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
      return PTYF_FORK_ERR;
   } 
   /* child control flow  */
   else if (pid == 0) {
      if (setsid() < 0) {
         fprintf(stderr, "setsid error");
         return PTYF_SETSID_ERR;
      }
         
      /*
      * System V acquires controlling terminal on open().
      */
      if ((fds = ptys_open(pts_name)) < 0)
         fprintf(stderr, "Can't open slave pty");

      close(fdm); /* all done with master in child */

      if (fds < 0) {
         err = fds;
         return err;
      }

#if defined(TIOCSCTTY)
      /*
      * TIOCSCTTY is the BSD way to acquire a controlling terminal.
      */
      if (ioctl(fds, TIOCSCTTY, NULL) < 0)
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

   /* parent control flow */
   
   *ptrfdm = fdm; /* return fd of master */
   return pid; /* parent returns pid of child */
}
