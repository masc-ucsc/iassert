
#include <stdio.h>
#include <unistd.h>

#include <string>

#include "iassert.hpp"

int main() {
  I_setup();

  int i=0;
  int j = 1;

  static int pointer[3];
  //pointer[1001230] = 0;
  //
  std::string s1 = "potato";
  std::string s2 = "bar";

  I(s1 != s2);

  I(i==0);
  I(j==2,"this fails");
  GI(true,true, "jeje");
  GI(true,true);

  printf("Hello %d\n",j);
}
