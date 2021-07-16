#include<stdio.h>
#include<assert.h>

typedef unsigned char *byte_pointer;

unsigned replace_byte(unsigned x,int i, unsigned char b){
    byte_pointer p = (byte_pointer) &x;
    p[i] = b;
    return x;
}

int main(int agrc,char *argv[]){
    unsigned x = replace_byte(0x12345678,2,0xAB);
    unsigned y = replace_byte(0x12345678,0,0xAB);
    assert(x == 0x12AB5678);
    assert(y == 0x123456AB);
    return 0;
}