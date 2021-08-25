void Im_rio_writen(int fd, void *usrbuf, size_t n) {
  if (rio_writen(fd, usrbuf, n) != n) {
    if (errno == EPIPE)
      fprintf(stderr, "EPIPE error");

    fprintf(stderr, "%s ", strerror(errno));
    unix_error("client side has ended connection");
  }
}