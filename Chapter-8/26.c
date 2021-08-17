#include <assert.h>
#include "../csapp.h"
#include "shell.h"
#include "job.h"

void eval(char *cmdline)
{
  char *argv[MAXARGS]; /* Argument list execve() */
  char buf[MAXLINE];   /* Holds modified command line */
  int bg;              /* Should the job run in bg or fg? */
  pid_t pid;           /* Process id */

  strcpy(buf, cmdline);
  bg = parse_line(buf, argv);
  if (argv[0] == NULL)
    return;   /* Ignore empty lines */

  if (!builtin_command(argv)) {
    sigset_t mask_one, prev_one;
    Sigemptyset(&mask_one);
    Sigaddset(&mask_one, SIGCHLD);

    /* block signal child */
    Sigprocmask(SIG_BLOCK, &mask_one, &prev_one);
    if ((pid = Fork()) == 0) {
      /* unblock in child process */
      Sigprocmask(SIG_SETMASK, &prev_one, NULL);

      /* set gid same like pid */
      Setpgid(0, 0);

      if (execve(argv[0], argv, environ) < 0) {
        printf("%s: Command not found.\n", argv[0]);
        exit(0);
      }
    }

    sigset_t mask_all, prev_all;
    Sigfillset(&mask_all);
    // save job info
    Sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
    Jid new_jid = new_job(pid, cmdline, !bg);
    Sigprocmask(SIG_SETMASK, &prev_all, NULL);

    if (!bg) {
      set_fg_pid(pid);
      while(get_fg_pid())
        sigsuspend(&prev_one);
    }
    else
      printf("[%d] %d %s \t %s\n", new_jid, pid, "Running", cmdline);

    /* unblock child signal */
    Sigprocmask(SIG_SETMASK, &prev_one, NULL);
  }
  return;
}

/*
 * If first arg is a builtin command, run it and return true;
 * else return false.
 */
int builtin_command(char **argv)
{
  if (!strcmp(argv[0], "quit")) /* quit command */
    exit(0);
  if (!strcmp(argv[0], "&"))    /* Ignore singleton & */
    return 1;
  if (!strcmp(argv[0], "jobs")) {
    print_jobs();
    return 1;
  }
  // > fg
  if (!strcmp(argv[0], "fg")) {
    int id;
    // right format: fg %ddd or fg ddd
    if ((id = parse_id(argv[1])) != -1 && argv[2] == NULL) {
      sigset_t mask_one, prev_one;
      Sigemptyset(&mask_one);
      Sigaddset(&mask_one, SIGCHLD);
      Sigprocmask(SIG_BLOCK, &mask_one, &prev_one);

      pid_t pid = id;
      // if param is jid
      if (argv[1][0] == '%') {
        JobPtr jp = find_job_by_jid(id);
        pid = jp->pid;
      }
      Kill(pid, SIGCONT);
      set_fg_pid(pid);
      while(get_fg_pid())
        sigsuspend(&prev_one);

      Sigprocmask(SIG_SETMASK, &prev_one, NULL);

    } else {
      printf("format error, e.g. fg %%12 || fg 1498\n");
    }

    return 1;
  }
  // > bg
  if (!strcmp(argv[0], "bg")) {
    int id;
    // right format: bg %ddd or bg ddd
    if ((id = parse_id(argv[1])) != -1 && argv[2] == NULL) {
      pid_t pid = id;
      // jid param
      if (argv[1][0] == '%') {
        JobPtr jp = find_job_by_jid(id);
        pid = jp->pid;
      }
      Kill(pid, SIGCONT);
    } else {
      printf("format error, e.g. bg %%12  or  bg 1498\n");
    }
    return 1;
  }

  return 0;                     /* Not a builtin command */
}

/* parse_line - Parse the command line and build the argv array */
int parse_line(char *buf, char **argv)
{
  char *delim;         /* Points to first space delimiter */
  int argc;            /* Number of args */
  int bg;              /* Background job? */

  buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
  while (*buf && (*buf == ' ')) /* Ignore leading spaces */
    buf++;

  /* Build the argv list */
  argc = 0;
  while ((delim = strchr(buf, ' '))) {
    argv[argc++] = buf;
    *delim = '\0';
    buf = delim + 1;
    while (*buf && (*buf == ' ')) /* Ignore spaces */
      buf++;
  }
  argv[argc] = NULL;

  if (argc == 0)  /* Ignore blank line */
    return 1;

  /* Should the job run in the background? */
  if ((bg = (*argv[argc-1] == '&')) != 0)
    argv[--argc] = NULL;

  return bg;
}

static int is_number_str(char* s) {
  int len = strlen(s);
  for (int i = 0; i < len; i++)
    if (!isdigit(s[i]))
      return 0;

  return 1;
}

int parse_id(char* s) {
  int error = -1;
  if (s == NULL)
    return error;

  /* format: %ddddd */
  if (s[0] == '%') {
    if (!is_number_str(s+1))
      return error;

    return atoi(s+1);
  }
  /* format: dddddd */
  if (is_number_str(s))
    return atoi(s);

  /* not right */
  return error;
}

void test_shell() {
  // parse id
  assert(-1 == parse_id("ns"));
  assert(-1 == parse_id("%%"));
  assert(0 == parse_id("%0"));
  assert(0 == parse_id("0"));
  assert(98 == parse_id("%98"));
  assert(98 == parse_id("98"));
}
#include <stdio.h>
#include <assert.h>
#include "job.h"
#include "../csapp.h"

static volatile sig_atomic_t fg_pid;
static Job jobs[MAXJOBS];

int is_fg_pid(pid_t pid) {
  return fg_pid == pid;
}
pid_t get_fg_pid() {
  return fg_pid;
}
void set_fg_pid(pid_t pid) {
  fg_pid = pid;
}

/* SIGCONT signal */
void sigchild_handler(int sig) {
  int old_errno = errno;
  int status;
  pid_t pid;

  sigset_t mask_all, prev_all;
  Sigfillset(&mask_all);

  /* exit or be stopped or continue */
  while ((pid = waitpid(-1, &status, WNOHANG|WUNTRACED|WCONTINUED)) > 0) {
    /* exit normally */
    if (WIFEXITED(status) || WIFSIGNALED(status)) {
      if (is_fg_pid(pid)) {
        set_fg_pid(0);
      } else {
        Sio_puts("pid "); Sio_putl(pid); Sio_puts(" terminates\n");
      }
      Sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
      del_job_by_pid(pid);
      Sigprocmask(SIG_SETMASK, &prev_all, NULL);
    }

    /* be stopped */
    if (WIFSTOPPED(status)) {
      if (is_fg_pid(pid)) {
        set_fg_pid(0);
      }
      // set pid status stopped
      Sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
      JobPtr jp = find_job_by_pid(pid);
      set_job_status(jp, Stopped);
      Sigprocmask(SIG_SETMASK, &prev_all, NULL);

      Sio_puts("pid "); Sio_putl(pid); Sio_puts(" be stopped\n");
    }

    /* continue */
    if(WIFCONTINUED(status)) {
      set_fg_pid(pid);
      // set pid status running
      Sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
      JobPtr jp = find_job_by_pid(pid);
      set_job_status(jp, Running);
      Sigprocmask(SIG_SETMASK, &prev_all, NULL);

      Sio_puts("pid "); Sio_putl(pid); Sio_puts(" continue\n");
    }
  }

  errno = old_errno;
}

void sigint_handler(int sig) {
  /* when fg_pid == 0, stop shell itself, it'll be a dead loop */
  if (is_fg_pid(0)) {
    Signal(SIGINT, SIG_DFL);
    Kill(getpid(), SIGINT);
  } else {
    Kill(get_fg_pid(), SIGINT);
  }
}

void sigstop_handler(int sig) {
  /* same like int handler */
  if (is_fg_pid(0)) {
    Signal(SIGTSTP, SIG_DFL);
    Kill(getpid(), SIGTSTP);
  } else {
    Kill(get_fg_pid(), SIGTSTP);
  }
}

JobPtr find_job_by_jid(Jid jid) {
  return &(jobs[jid]);
}

JobPtr find_job_by_pid(pid_t pid) {
  for (int i = 0; i < MAXJOBS; i++) {
    Job j = jobs[i];
    if (j.using && j.pid == pid) {
      return &(jobs[i]);
    }
  }
  /* no such job */
  return NULL;
}

void set_job_status(JobPtr jp, enum JobStatus status) {
  if (jp)
    jp->status = status;
}


// seek a spare place for new job
static int find_spare_jid() {
  Jid jid = -1;
  for (int i = 0; i < MAXJOBS; i++) {
    if (jobs[i].using == 0) {
      jid = i;
      break;
    }
  }
  return jid;
}

int new_job(pid_t pid, char* cmdline, int fg) {
  // find a jid
  Jid jid = find_spare_jid();
  if (jid == -1)
    unix_error("no more jid to use");

  // save process info
  jobs[jid].jid = jid;
  jobs[jid].pid = pid;
  jobs[jid].status = Running;
  strcpy(jobs[jid].cmdline, cmdline);
  jobs[jid].using = 1;

  return jid;
}

void del_job_by_pid(pid_t pid) {
  // search job whose pid is pid
  for (int i = 0; i < MAXJOBS; i++) {
    if (jobs[i].using && jobs[i].pid == pid) {
      // delete job
      jobs[i].using = 0;
    }
  }
}

void print_jobs() {
  for (int i = 0; i < MAXJOBS; i++) {
    Job j = jobs[i];
    if (j.using) {
      printf("[%d] %d %s \t %s\n", j.jid, j.pid,
          j.status == Running ? "Running" : "Stopped", j.cmdline);
    }
  }
}

void init_jobs() {
  memset(jobs, 0, sizeof(jobs));
}

void test_job() {

}