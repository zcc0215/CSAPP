    if (Fork() == 0) {
      Close(listenfd);
      doit(connfd);                                             //line:netp:tiny:doit
      Close(connfd);                                            //line:netp:tiny:close
      exit(0);
    }
    Close(connfd);