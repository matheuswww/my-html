
#include "tokens.h"

typedef String *(*function)(String*,Token*);

#define empty(x) (!(x)->fun)
#define copyentry(dst, src); do {                                         \
  memorycopy($1 &(dst->token), $1 &(src->token), sizeof(struct s_token)); \
  (dst)->fun = (src)->fun;                                                 \
} while(false)
#define first(x) (!(x).prev)

struct s_stack {
  Token token;
  function fun;
  int16 length;
  struct s_stack *next;
  struct s_stack *prev;
};
typedef struct s_stack Stack;

struct s_tuple {
  Stack *xs;
  Token x;
};
typedef struct s_tuple STuple;

/* constructor */
Stack *mkstack(int16);
Stack *mkentry(void);

void printstack(Stack*);
Stack *sindex(Stack*, signed short int);
Stack *push(Garbage*,Stack*,Token);
STuple *apop(Garbage*,Stack*,Tag);
Stack *findlast(Stack*);
Stack *stackcopy(Garbage*,Stack*);
String *id(String *s,Token*);
function findfun(Token);
