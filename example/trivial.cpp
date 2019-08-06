
#include <stdio.h>
#include <unistd.h>

#include <string>

#include "iassert.hpp"

int main() {
  I_setup();

  int i=0;
  int j = 1;

  std::string s1 = "potato";
  std::string s2 = "bar";

  I(s1 != s2);

  I(i==0);
  I(j==2,"this fails");
  GI(true,true, "jeje");
  printf("foo %d\n",j);
  GI(true,true);
  I(j==7,"this fails gail");

  int a_signed = 1;
  unsigned int a_unsigned = 1;
  I(a_signed == a_unsigned); // No warning
  I(a_signed < s1.size());   // No warning

  printf("Hello %d\n",j);
}
