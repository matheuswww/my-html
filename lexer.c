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
  Tokens *xs, *xs_;
  State *state;

  assert(s);
  if (!s->length)
    return (Tokens *)0;
  
  g = mkgarbage();
  xs = mktokens(g);
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

  xs_ = lexer_(g, s, xs, state);
  g = gc(g);

  return xs_;
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
  if (!c && !s_) {
    addgc(g, s);
    return xs;
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
      if (type == text) {
        *state->cur = c;
        state->cur++;
      }
      return lexer_(g, s_, xs, state);

      break;

    case readtoken:
      if (state->type == text) {
        if ((!s_->length) || (cc = peek(s_)) == '<') {
          *state->cur = c;
          state->cur++;
          t = mktoken(g, state->type, state->buf);
          if (!t)
            return (Tokens *)0;
          xs_ = tcons(g, *t, xs);
          if (!xs_)
            return (Tokens *)0; 
          addgc(g, xs);
          zero(state->buf, 256);
          state->stage = newtoken;
          state->cur = state->buf;

          return lexer_(g, s_, xs_, state);
        }
      }
      if (c == '/')
        return lexer_(g, s_, xs, state);
      if ((c == ' ') && (state->type != text)) { 
        cc = peek(s_);
        if (cc == '/') {
          state->type = selfclosed;
        }

        return lexer_(g, s_, xs, state);
      } else if ((c == '>')) {
        if ((state->type == tagstart) || (state->type == tagend) || (state->type == selfclosed)) {
          t = mktoken(g, state->type, state->buf);
          if (!t)
            return (Tokens *)0;
          xs_ = tcons(g, *t, xs);
          if (!xs_)
            return (Tokens *)0;
          addgc(g, xs);
          zero(state->buf, 256);
          state->stage = newtoken;
          state->cur = state->buf;

          return lexer_(g, s_, xs_, state);
        }
      }

      if (((void*)state->cur-(void*)&state->buf) >= 254)
        return (Tokens *)0;

      *state->cur = c;
      state->cur++;
      return lexer_(g, s_, xs, state);

      break;

    default:
     break;
  }

  return (Tokens *)0;
}