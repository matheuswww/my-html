#include "parser.h"

String *id(String *s,Token*) {
  return s;
}

function findfun(Token t) {
  return (function )0;
}

Stack *findlast(Stack* s) {
  Stack *p, *last;

  if(!s)
    return (Stack *)0;
  for (p = s; p; p = p->next)
    last = p;

  return p ? p : last;
}

Stack *mkentry() {
  int16 size;
  Stack *p;

  size = sizeof(struct s_stack);
  p = (Stack *)malloc($i size);
  assert(p);
  zero($1 p, size);

  return p;
}

void printstack(Stack *s) {
  Stack *p;
  int16 n;
  int8* value = (int8 *)0;

  assert(s && s->length);

  for (n = 0, p=s; n < s->length; n++, p = p->next) {
    value = (int8 *)0;
    if (p->token.type) { 
      switch (p->token.type) { 
        case tagstart: 
          value = p->token.contents.start->value; 
        break; 
        case tagend: 
          value = p->token.contents.end->value;
        break; 
        case selfclosed: 
          value = p->token.contents.self->value; 
        break; 
        default: 
          value = p->token.contents.texttoken->value; 
        break; 
      }
    }
    /*
    printf(
      "entry at 0x%x\n"
      " .token = '%s'\n"
      " .next  = 0x%x\n"
      " .prev = 0x%x\n\n",
        $i p, (value)?value:$1 "",
        $i p->next, $i p->prev
    );
    */
   printf("<%s>", value);
  }
  printf("\n");

  return;
}

STuple *apop(Garbage *g, Stack *s, Tag type) {
  int16 size;
  STuple *ret;
  Stack *p, *s_;
  int16 x;
  
  assert(s && s->length && type);
  size = sizeof(struct s_tuple);
  ret = (STuple *)malloc($i size);
  assert(ret);
  s_ = stackcopy(g, s);
  if (!s_)
    goto error;
  if (s_->length == 1) {
    if (s_->token.contents.start->type != type) 
      goto error;
    ret->xs = (Stack *)0;
    memorycopy($1 &ret->x, $1 &s_->token, sizeof(struct s_token));
    if (g)
      addgc(g, s_);
    
    goto end;
  }

  for (x=0, p=findlast(s_); x < s_->length; x++, p = p->prev)
    if (p->token.contents.start->type == type)
      break;
 
  if (!p || p->token.contents.start->type != type)
    goto error; 

  memorycopy($1 &ret->x, $1 &p->token, sizeof(struct s_token));
  if (!p->next) {
    p->prev->next = (Stack*)0;
    s_->length--;
    if (g)
      addgc(g, p);
    ret->xs = s_;
  }
  else if (!p->prev) {
    p = p->next;
    ret->xs = p;
    p->prev = (Stack *)0;
    p->length = s_->length;
    p->length--;
    if (g)
      addgc(g, s_);
  } else {
    p->prev->next = p->next;
    p->next->prev = p->prev;
    s_->length--;
    ret->xs = s_;
  }
  goto end;

  error:
    ret->xs = (Stack *)0;
    ret->x.contents.start->type =  0;

  end:
  
  return ret;
}

Stack *push(Garbage *g, Stack *s, Token t) {
  Stack *last, *s_, *entry;

  assert(s && t.type && s->length);
  s_ = stackcopy(g, s);
  if (!s_)
    return (Stack *)0;
  
  entry = mkentry();
  last = sindex(s_, -1);
  if (!last)
    return (Stack *)0;
  last->next = entry;
  entry->prev = last;
  s_->length++;
  memorycopy(&entry->token, &t, sizeof(struct s_token));
  entry->fun = findfun(t);
  return s_;
}

Stack *mkstack(int16 size) {
  int16 n;
  Stack *p, *last, *first;

  assert(size);
  last = (Stack *)0;
  first = (Stack *)0;
  for (n = size; n; n--) {
    p = mkentry();
    if (!first)
      first = p;
    p->prev = last;
    
    if (last)
      last->next = p;
    last = p;
  }
  first->length = size;

  return first;
}

Stack *sindex(Stack *s, signed short int idx) {
  signed short int n;
  Stack *p;

  assert(s);
  assert(s->length > idx);

  if (s->length == 1)
    return s;

  if (idx < 0) {
    n = -1;
    p = findlast(s);
    do {
      n--;
      if (n > idx)
        p = p->prev;
    } while (n > idx);
  }

  else
    for (n=0, p = s; n < idx; n++, p = p->next);

  return p;
}

Stack *stackcopy(Garbage *g, Stack *s) {
  int16 n;
  Stack *np, *op, *new;

  assert(s && s->length);
  new = mkstack(s->length);
  if (!new) 
    return (Stack *)0; 

  for (n = 0, np = new, op = s; n < s->length; n++, np = np->next, op = op->next) {
    copyentry(np, op);
    if (g)
      addgc(g, op);
  }

  return  new;
}

bool isopen(Stack *s, Tag type) {
  int16 n;
  Stack *p;
  bool ret;

  assert(s);
  ret = false;

  if (!s->length)
    return false;
  for (n = 0, p = s; n < s->length; n++, p = p->next)
    if (type == p->token.contents.start->type) {
      ret = true;
      break;
    }

  return ret;
}

Stack *parse(Tokens* xs) {
  Garbage *g;
  Stack *s, *s_;
  TTuple tuple;

  assert(xs && xs->length);
  g = mkgarbage();
  s = mkstack(1);
  assert(s && s->length);

  tuple = tget(g, xs);
  if(!tuple.xs)
    return (Stack *)0;

  assert(tuple.x.type == tagstart);
  
  memorycopy(&s->token, &tuple.x, sizeof(struct s_token));
  if (!tuple.xs->length)
    return s;

  s_ = parse_(g, tuple.xs, s);
  if (!s_)
    return s;

  gc(g);

  return s_;
}

Stack *parse_(Garbage *g, Tokens *xs, Stack *s) {
  Tokens *xs_;
  Stack *s_, *s__;
  TTuple tuple;

  assert(s && s->length);

  if (!xs->length)
    return s;

  tuple = tget(g, xs);
  if (!tuple.xs)
    return (Stack *)0;

  xs_ = tuple.xs;
  s_ = push(g, s, tuple.x);
  if (!s_)
    return (Stack *)0;

  if (tuple.x.type == tagstart)
    return parse_(g, xs_, s_);
  else
    s__ = stackcopy(g, s);  
  return parse_(g, xs_, s__);
}