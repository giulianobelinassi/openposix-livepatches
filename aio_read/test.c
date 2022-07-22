#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <aio.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>

static volatile bool gate = false;

void sig_handler(int signum)
{
  if (signum == SIGUSR1)
    gate = !gate;
}


#define BUF_SIZE 20     /* Size of buffers for read operations */

#define errExit(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

struct ioRequest {      /* Application-defined structure for tracking
                          I/O requests */
   int           reqNum;
   int           status;
   struct aiocb *aiocbp;
};

static volatile sig_atomic_t gotSIGQUIT = 0;
                       /* On delivery of SIGQUIT, we attempt to
                          cancel all outstanding I/O requests */

static void             /* Handler for SIGQUIT */
quitHandler(int sig)
{
   gotSIGQUIT = 1;
}

#define IO_SIGNAL SIGUSR2   /* Signal used to notify I/O completion */

static void                 /* Handler for I/O completion signal */
aioSigHandler(int sig, siginfo_t *si, void *ucontext)
{
   if (si->si_code == SI_ASYNCIO) {
       write(STDOUT_FILENO, "I/O completion signal received\n", 31);

       /* The corresponding ioRequest structure would be available as
              struct ioRequest *ioReq = si->si_value.sival_ptr;
          and the file descriptor would then be available via
              ioReq->aiocbp->aio_fildes */
   }
}

int
main(int argc, char *argv[])
{
   struct sigaction sa;
   int s;
   int numReqs;        /* Total number of queued I/O requests */
   int openReqs;       /* Number of I/O requests still in progress */

  signal(SIGUSR1, sig_handler);

  printf("Waiting for SIGUSR1\n");

  while (gate == false)
    usleep(1000);

   puts(strdup(NULL));
   numReqs = 1;

   /* Allocate our arrays. */

   struct ioRequest *ioList = calloc(numReqs, sizeof(*ioList));
   if (ioList == NULL)
       errExit("calloc");

   struct aiocb *aiocbList = calloc(numReqs, sizeof(*aiocbList));
   if (aiocbList == NULL)
       errExit("calloc");

   /* Establish handlers for SIGQUIT and the I/O completion signal. */

   sa.sa_flags = SA_RESTART;
   sigemptyset(&sa.sa_mask);

   sa.sa_handler = quitHandler;
   if (sigaction(SIGQUIT, &sa, NULL) == -1)
       errExit("sigaction");

   sa.sa_flags = SA_RESTART | SA_SIGINFO;
   sa.sa_sigaction = aioSigHandler;
   if (sigaction(IO_SIGNAL, &sa, NULL) == -1)
       errExit("sigaction");

   /* Open each file specified on the command line, and queue
      a read request on the resulting file descriptor. */

   for (int j = 0; j < numReqs; j++) {
       ioList[j].reqNum = j;
       ioList[j].status = EINPROGRESS;
       ioList[j].aiocbp = &aiocbList[j];

       ioList[j].aiocbp->aio_fildes = open("librt_livepatch1.c", O_RDONLY);
       if (ioList[j].aiocbp->aio_fildes == -1)
           errExit("open");
       printf("opened %s on descriptor %d\n", "librt_livepatch1.c",
               ioList[j].aiocbp->aio_fildes);

       ioList[j].aiocbp->aio_buf = malloc(BUF_SIZE);
       if (ioList[j].aiocbp->aio_buf == NULL)
           errExit("malloc");

       ioList[j].aiocbp->aio_nbytes = BUF_SIZE;
       ioList[j].aiocbp->aio_reqprio = 0;
       ioList[j].aiocbp->aio_offset = 0;
       ioList[j].aiocbp->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
       ioList[j].aiocbp->aio_sigevent.sigev_signo = IO_SIGNAL;
       ioList[j].aiocbp->aio_sigevent.sigev_value.sival_ptr =
                               &ioList[j];

       s = aio_read(ioList[j].aiocbp);
       if (s == -1)
           errExit("aio_read");
   }

   openReqs = numReqs;

   /* Loop, monitoring status of I/O requests. */

   while (openReqs > 0) {
       sleep(3);       /* Delay between each monitoring step */

       if (gotSIGQUIT) {

           /* On receipt of SIGQUIT, attempt to cancel each of the
              outstanding I/O requests, and display status returned
              from the cancellation requests. */

           printf("got SIGQUIT; canceling I/O requests: \n");

           for (int j = 0; j < numReqs; j++) {
               if (ioList[j].status == EINPROGRESS) {
                   printf("    Request %d on descriptor %d:", j,
                           ioList[j].aiocbp->aio_fildes);
                   s = aio_cancel(ioList[j].aiocbp->aio_fildes,
                           ioList[j].aiocbp);
                   if (s == AIO_CANCELED)
                       printf("I/O canceled\n");
                   else if (s == AIO_NOTCANCELED)
                       printf("I/O not canceled\n");
                   else if (s == AIO_ALLDONE)
                       printf("I/O all done\n");
                   else
                       perror("aio_cancel");
               }
           }

           gotSIGQUIT = 0;
       }

       /* Check the status of each I/O request that is still
          in progress. */

       printf("aio_error():\n");
       for (int j = 0; j < numReqs; j++) {
           if (ioList[j].status == EINPROGRESS) {
               printf("    for request %d (descriptor %d): ",
                       j, ioList[j].aiocbp->aio_fildes);
               ioList[j].status = aio_error(ioList[j].aiocbp);

               switch (ioList[j].status) {
               case 0:
                   printf("I/O succeeded\n");
                   break;
               case EINPROGRESS:
                   printf("In progress\n");
                   break;
               case ECANCELED:
                   printf("Canceled\n");
                   break;
               default:
                   perror("aio_error");
                   break;
               }

               if (ioList[j].status != EINPROGRESS)
                   openReqs--;
           }
       }
   }

   printf("All I/O requests completed\n");

   /* Check status return of all I/O requests. */

   printf("aio_return():\n");
   for (int j = 0; j < numReqs; j++) {
       ssize_t s;

       s = aio_return(ioList[j].aiocbp);
       printf("    for request %d (descriptor %d): %zd\n",
               j, ioList[j].aiocbp->aio_fildes, s);
   }

   exit(EXIT_SUCCESS);
}

