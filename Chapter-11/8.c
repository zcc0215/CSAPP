void sigchild_handler(int sig) {
  int old_errno = errno;
  int status;
  pid_t pid;
  while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
  }
  errno = old_errno;
}