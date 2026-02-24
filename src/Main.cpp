#include "core/pty.h" 
#include "core/tty_man.h"
#include "Defines.hpp"
#include "Assert.hpp"

#define BUFFSIZE 512

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

struct termios orig_termios;
struct winsize size;

void tty_atexit() {
   tty_set(STDIN_FILENO, &orig_termios, &size);
}

int main()
{
   int fdm, interactive, noecho, verbose;
   pid_t pid;
   char slave_name[20];

   interactive = isatty(STDIN_FILENO);
   noecho = 0;
   verbose = 1;

   if (interactive) { /* fetch current termios and window size */
      if (save_termios(STDIN_FILENO, &orig_termios) < 0)
         fprintf(stderr, "Failed to save origin termios attributes");

      if (save_winsize(STDIN_FILENO, &size) < 0)
         fprintf(stderr, "Failed to save origin winsize");

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
