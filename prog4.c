/*

Nicholas Carnevale
CSE202 PROG4
12/05/2023

*/


#include <stdio.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <unistd.h> 
#include <stdlib.h>
#include <signal.h>
#include <wait.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <time.h>

//declare global variable as volatile 
volatile int avg = 0; /* Global variables to hold the final average */

/* function to initialize an array of integers with random values */
void initialize(int*);

/* Wrapper function prototypes for the system calls */
void unix_error(const char *msg);
pid_t Fork();
pid_t Wait(int *status);
pid_t Waitpid(pid_t pid, int *status, int options);
int Sigqueue(pid_t pid, int signum, union sigval value);
int Sigemptyset(sigset_t *set);
int Sigfillset(sigset_t *set);
int Sigaction(int signum, const struct sigaction *new_act, struct sigaction *old_act);
int Sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
ssize_t Write(int d, const void *buffer, size_t nbytes);
typedef void handler_t;
handler_t *Signal(int signum, handler_t *handler);

/* Prototype of the handler */
void sigusr1_handler(int sig, siginfo_t *value, void *ucontext);

/* main function */
int main(){
  int A[N];
  initialize(A);
  /* install a portable handler for SIGUSR1 using the wrapper function Signal */

  struct sigaction sa;
  sa.sa_sigaction = &sigusr1_handler;
  Sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_SIGINFO | SA_RESTART;
  Sigaction(SIGUSR1, &sa, NULL);

  /* print the message for installing SIGUSR1 handler*/
  printf("Parent process %d installing SIGUSR1 handler\n", getpid());
  //so we don't repeat message in child processes
  fflush(stdout);

  /* create (P child processes) to calculate a partial average and send the signal SIGUSR1 to the parent*/
  for(int i = 0; i < P; i++){
    pid_t pid = Fork();

    //if child
    if(pid == 0){
      int partialAverage = 0;
      int left = i*N/P;
      int right = (i == P-1) ? N : left + N/P;
      printf("Child process %d finding the average from %d to %d\n", getpid(), left, right-1);

      for(int j = left; j < right; j++){
        partialAverage += A[j];
      }
      partialAverage /= right-left;
      
      printf("Child process %d sending SIGUSR1 to parent process with the partial average %d\n", getpid(), partialAverage);

      //from Google Docs
      union sigval value; // sigval union
      value.sival_int = partialAverage; /* set the field sival_int to the integer value you want to send with the signal*/ 
      Sigqueue(getppid(), SIGUSR1, value);/*send SIGUSR1 to the process pid with the integer stored in value*/

      exit(EXIT_SUCCESS);
    

    }else{/* reap the (P) children */

      int status;
      pid_t pid = Wait(&status);

      if(status == 0){
        printf("Child process %d terminated normally with exit status %d\n", pid, status);
      }else{
        printf("Child process %d terminated abnormally\n", pid);
      }

      //so message is not repeated
      fflush(stdout);
    }
  }
  
  
  /* print the array A if the macro TEST is defined */
  #ifdef TEST
    printf("A = {");
    for(int i=0; i<N-1; i++){
      printf("%d, ", A[i]);
    }
    printf("%d}\n", A[N-1]);
  #endif

  /* print the final average */
  printf("Final Average = %d\n", avg / P);

  exit(0);
}


/* Definition of the function initialize */
void initialize(int M[N]){
    int i;
    srand(time(NULL));
    for(i=0; i<N; i++){
        M[i] = rand() % N;
    }
}
/* Define the Handler for SIGUSR1 here */
void sigusr1_handler(int sig, siginfo_t *value, void *ucontext){
    /* Follow the guidelines to write safe signal handlers*/
    int partialAverage = value->si_value.sival_int;
    avg += partialAverage;

    //Can't use printf because it is not safe
    //printf("Parent process caught SIGUSR1 with partial average: %d\n", partialAverage);
    //use Write() according to the slides

    char msg[64];
    int len = snprintf(msg, sizeof(msg), "Parent process caught SIGUSR1 with partial average: %d\n", partialAverage);
    Write(STDOUT_FILENO, msg, len);

}

/* Define the Wrapper functions for the system calls */

// unix_error
void unix_error(const char *msg){
  fprintf(stderr, "%s: %s\n", msg, strerror(errno));
  exit(EXIT_FAILURE);
}

// Fork
pid_t Fork(){
  pid_t pid = fork();
  if(pid == -1){
    unix_error("Fork Error");
  }
  return pid;
}

// Wait
pid_t Wait(int *status){
  pid_t pid = wait(status);
  if(pid == -1){
    unix_error("Wait Error");
  }
  return pid;
}

// Waitpid
pid_t Waitpid(pid_t pid, int *status, int options){
  pid_t child_pid = waitpid(pid, status, options);
  if(child_pid == -1){
    unix_error("Waitpid Error");
  }
  return child_pid;
}

// Sigqueue
int Sigqueue(pid_t pid, int signum, union sigval value){
  int r = sigqueue(pid, signum, value);
  if(r == -1){
    unix_error("Sigqueue Error");
  }
  return r;
}

// Sigemptyset
int Sigemptyset(sigset_t *set){
  int r = sigemptyset(set);
  if(r == -1){
    unix_error("Sigemptyset Error");
  }
  return r;
}

// Sigfillset
int Sigfillset(sigset_t *set){
  int r = sigfillset(set);
  if(r == -1){
    unix_error("Sigfillset error");
  }
  return r;
}

// Sigaction
int Sigaction(int signum, const struct sigaction *new_act, struct sigaction *old_act){
  int r = sigaction(signum, new_act, old_act);
  if (r == -1){
    unix_error("Sigaction Error");
  }
  return r;
}

// Sigprocmask
int Sigprocmask(int how, const sigset_t *set, sigset_t *oldset){
  int r = sigprocmask(how, set, oldset);
  if (r == -1){
    unix_error("Sigprocmask Error");
  }
  return r;
}

// Write
ssize_t Write(int d, const void *buffer, size_t nbytes){
  ssize_t bytes = write(d, buffer, nbytes);
  if (bytes == -1){
    unix_error("Write Error");
  }
  return bytes;
}

// Signal
handler_t *Signal(int signum, handler_t *handler){
  if (signal(signum, handler) == SIG_ERR){
    unix_error("Signal error");
  }
  return handler;
}