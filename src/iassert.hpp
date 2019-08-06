//  This file is distributed under the BSD 3-Clause License. See LICENSE for details.

// Better way to handle assertions:
//
// Invariant: like assert, cond must be always true
//   I(cond,message)
//   I(cond)
//
// Guarded Invariant: like assert, check only if first condition is true
//
//   GI(cond1, cond2, message)
//   GI(cond1, cond2)
//
// GDB support for remote debug if I_GDB=1

#pragma once

void I_internal(const char *file, int line, const char *condition, const char *message);
void I_setup();
void I_gdb_continuation();

#define I_0()                     I_2_shared(true,"")
#define I_1(A)                    I_2_shared(A,"")
#define I_2(A,B)                  I_2_shared(A,B)
#define I_3(A,B,C)                I_2_shared(false,"invalid I with 3 arguments")

#define GI_0()                    I_2_shared(false,"invalid GI with no arguments")
#define GI_1(A)                   I_2_shared(false,"invalid GI with one argument")
#define GI_2(A,B)                 do{ if (A) I_2_shared(B,""); }while(0)
#define GI_3(A,B,C)               do{ if (A) I_2_shared(B,C);  }while(0)

#define IX_X(x,A,B,C,FUNC, ...)  FUNC

#define I(...) \
  do{ _Pragma("GCC diagnostic push"); _Pragma("GCC diagnostic ignored \"-Wsign-compare\""); \
    IX_X(,##__VA_ARGS__,\
        I_3(__VA_ARGS__),\
        I_2(__VA_ARGS__),\
        I_1(__VA_ARGS__),\
        I_0(__VA_ARGS__)\
        ); \
    _Pragma("GCC diagnostic pop"); }while(0)

#define GI(...) \
  do{ _Pragma("GCC diagnostic push"); _Pragma("GCC diagnostic ignored \"-Wsign-compare\""); \
    IX_X(,##__VA_ARGS__,\
        GI_3(__VA_ARGS__),\
        GI_2(__VA_ARGS__),\
        GI_1(__VA_ARGS__),\
        GI_0(__VA_ARGS__)\
        ); \
    _Pragma("GCC diagnostic pop"); }while(0)

#ifdef NDEBUG
// Keep the (void) to avoid warnings when in release the variable is not used
#define I_2_shared(condition, message) (void)(condition)
#else

#define I_2_shared(condition, message) \
  do{ if(!(condition)) { I_internal(__FILE__ , __LINE__ , #condition, message); I_gdb_continuation(); } }while(0)
#endif

