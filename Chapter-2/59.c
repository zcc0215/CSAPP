#include<stdio.h>
#include<assert.h>

int combin(int x,int y){
    x = x & 0xff;
    y = y & (~0xff);
    int z = x | y;
    return z;
}
int main(int agrc, char *argv[]){
    int x = 0x89ABCDEF;
    int y = 0x76543210;
    int z = combin(x,y);
    assert(z == 0x765432EF);
    return 0;
}