#include<stdio.h>
#include <assert.h>

unsigned srl(unsigned x,int k){
    unsigned xsra = (int) x>>k;
    int w = sizeof(int) <<3;
    int mark = (int)-1 << (w -k);
    return xsra & (~mark);
}

int sra(int x,int k){
    int xsrl = (unsigned) x>>k;
    int w = sizeof(int) <<3;
    int mark = (int)-1 << (w -k);
    int h = (int) 1 << (w -1);
    mark &= !(h & x) -1;
    return xsrl | mark;
}

int main(int argc, char* argv[]) {
  unsigned test_unsigned = 0x12345678;
  int test_int = 0x12345678;

  assert(srl(test_unsigned, 4) == test_unsigned >> 4);
  assert(sra(test_int, 4) == test_int >> 4);

  test_unsigned = 0x87654321;
  test_int = 0x87654321;

  assert (srl (test_unsigned, 4) == test_unsigned >> 4);
  assert (sra (test_int, 4) == test_int >> 4);
  
  return 0;
}