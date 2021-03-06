 /* If the descriptor is ready, echo a text line from it */
     if ((connfd > 0) && (FD_ISSET(connfd, &p->ready_set))) {
       p->nready--;
       if ((n = rio_readlineb(&rio, buf, MAXLINE)) > 0) {
         byte_cnt += n; //line:conc:echoservers:beginecho
         printf("Server received %d (%d total) bytes on fd %d\n",
             n, byte_cnt, connfd);
         Rio_writen(connfd, buf, n); //line:conc:echoservers:endecho
       }

       /* EOF detected, remove descriptor from pool */
       else if (n == 0) {
         Close(connfd); //line:conc:echoservers:closeconnfd
         FD_CLR(connfd, &p->read_set); //line:conc:echoservers:beginremove
         p->clientfd[i] = -1;          //line:conc:echoservers:endremove
       }
       /* n == -1, it's an error */
       else {
         fprintf(stderr, "error in fd %d, close fd %d connection\n", connfd, connfd);
         Close(connfd); //line:conc:echoservers:closeconnfd
         FD_CLR(connfd, &p->read_set); //line:conc:echoservers:beginremove
         p->clientfd[i] = -1;          //line:conc:echoservers:endremove