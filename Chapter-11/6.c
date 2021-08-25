void echo(int connfd) {
  size_t n;
  char buf[MAXLINE];
  rio_t rio;

  Rio_readinitb(&rio, connfd);
  while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
    if (strcmp(buf, "\r\n") == 0)
      break;
    Rio_writen(connfd, buf, n);
  }
}