#include "lexer.h"

State *mkstate() {
  State *s;
  int16 size;
  
  size = sizeof(struct s_state);
  s = (State *)malloc($i size);
  assert(s);
  zero($1 s, size);

  s->cur = $1 0;
  s->stage = none;
  s->type = 0;

  return s;
}


Tokens *lexer(String* s) {
  Garbage *g;
  Tokens *xs;
  State *state;

  assert(s);
  if (!s->length)
    return (Tokens *)0;
  
  g = mkgarbage();
  xs = mktokens((Garbage *)0);
  if (!g) {
    if (xs)
      free(xs);
    return (Tokens *)0; 
  }
  if (!xs) {
    if (g)
      free(g);
    return (Tokens *)0;
  }
  state = mkstate();
  state->stage = newtoken;
  addgc(g, state);

  return lexer_(g, s, xs, state);
}

Tokens *lexer_(Garbage *g, String *s, Tokens *xs, State *state) {
  Token *t;
  Tuple tuple;
  int8 c, cc;
  String *s_;
  Tokentype type;
  Tokens *xs_;

  tuple = get(s);
  c = tuple.c;
  s_ = tuple.s;
  addgc(g, s);
  if (!c && !s_) {
    /* we are done */
  }

  switch (state->stage) {
    case none:
      break;

    case newtoken:
      if (c == '<') {
        cc = peek(s_);
        if (cc == '/')
          type = tagend;
        else
          type = tagstart;
      } else {
        type = text;
      }
      state->type = type;
      zero(state->buf, 256);
      state->cur = state->buf;
      state->stage = readtoken;
      return lexer_(g, s_, xs, state);

      break;

    case readtoken:
      if (c == '/')
        return lexer_(g, s_, xs, state);
      if (c == ' ') {
        cc = peek(s);
        if (cc == '/') {
          state->type = selfclosed;
        }

        return lexer_(g, s, xs, state);
      } else if ((c == '>') || (state->type != text)) {
        if (state->type == tagstart || (state->type == tagend) || (state->type == selfclosed)) {
          t = mktoken(g, state->type, state->buf);
          if (!t)
            return (Token *)0;
          xs_ = tcons(g, *t, xs);
          if (!xs_)
            return (Token *)0;
          zero(state->buf, 256);
          state->stage = newtoken;
          state->cur = state->buf;

          return lexer_(g, s_, xs_, state);
        }
      }

      if ((state->cur-&state->buf) >= 254)
        return (Token *)0;

      state->cur++;
      *state->cur = c;
      return lexer_(g, s_, xs, state);

      break;

    default:
      return (Tokens *)0;
      break;
  }
}