 void clienterror(int fd, char *cause, char *errnum,
     char *shortmsg, char *longmsg);
 
 void *thread(void *vargp);
 
 int main(int argc, char **argv)
 {
   int listenfd, connfd;
   int *connfdp;
   pthread_t tid;
   char hostname[MAXLINE], port[MAXLINE];
   socklen_t clientlen;
   struct sockaddr_storage clientaddr;
   Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE,
    port, MAXLINE, 0);
   printf("Accepted connection from (%s, %s)\n", hostname, port);
   connfdp = (int*)Malloc(sizeof(int));
   *connfdp = connfd;
   Pthread_create(&tid, NULL, thread, connfdp);
   }
 }
 
 void *thread(void *vargp) {
   int connfd = *(int*)vargp;
   Pthread_detach(Pthread_self());
   Free(vargp);
 
   doit(connfd);
   Close(connfd);
   return NULL;
 }