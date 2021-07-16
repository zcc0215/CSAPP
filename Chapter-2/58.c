#include<stdio.h>
#include<assert.h>

typedef unsigned char *byte_pointer;

int is_little_endian(){
    int test = 0xff;
    byte_pointer p = (byte_pointer) &test;
    if(p[0] == 0xff){
        return 1;
    }
    return 0;
}

int main(int agrc,char *argv[]){
    assert(is_little_endian());
    return 0;
}