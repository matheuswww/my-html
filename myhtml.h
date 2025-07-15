#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>
#include "utils.h"

#define GCblocksize 1024;

struct s_string {
  int16 length;
  int8 *cur;
  int8 data[];
};
typedef struct s_string String;

struct s_tuple {
  String *s;
  int8 c;
};
typedef struct s_tuple Tuple;

struct s_garbage {
  int16 capacity;
  int16 size;
  void *p[];
};
typedef struct s_garbage Garbage;

typedef unsigned char int8;
typedef unsigned short int int16;
typedef unsigned int int32;
typedef unsigned long long int int64;

#define $1 (int8 *)
#define $2 (int16)
#define $4 (int32)
#define $8 (int64)
#define $c (char *)
#define $i (int)

#define sdestroy(s) free(s)

// constructor
String *mkstring(int8*);
Garbage *mkgarbage(void);

void addgc(Garbage *, void *);
Garbage *gc(Garbage *);
String *scopy(String *s);
Tuple get(String*);
int16 stringlen(int8*);
void stringcopy(int8*, int8*, int16);
int main(void);