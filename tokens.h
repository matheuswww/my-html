#pragma once

/* tokens.h */
typedef unsigned char int8;
typedef unsigned short int int16;
typedef unsigned int int32;

enum e_tag {
  html = 1,
  body = 2,
  b = 3,
  br = 4
};

typedef enum e_tag Tag;

typedef unsigned long long int int64;

struct s_tagstart {
  Tag  type;
  /* Attributes attrs; */
  int8 value[];
};
typedef struct s_tagstart Tagstart;

struct s_tagend {
  Tag type:3;
  int8 value[];
};
typedef struct s_tagend Tagend;

struct s_selfclosed {
  Tag type:3;
  int8 value[];
};
typedef struct s_selfclosed Selfclosed;

struct s_texttoken {
  Tag type:3;
  int8 value[];
};
typedef struct s_texttoken Text;

enum e_tokentype {
  text = 1,
  tagstart = 2,
  tagend = 3,
  selfclosed = 4  
};
typedef enum e_tokentype Tokentype;

struct s_token {
  Tokentype type:3;
  union {
    Text *texttoken;
    Tagstart *start;
    Tagend *end;
    Selfclosed *self;
  } contents;
};
typedef struct s_token Token;

struct s_tokens {
  int16 length:16;
  Token *ts;
};
typedef struct s_tokens Tokens;

struct s_ttuple {
  Tokens *xs;
  Token x;
};
typedef struct s_ttuple TTuple;

#define destroytoken(t) free(t);

int8 *showtoken(Garbage*, Token);
int8 *showtokens(Garbage *g, Tokens);
Tokens *tcopy(Garbage*, Tokens*);
TTuple tget(Garbage*, Tokens*);
Tokens *tcons(Garbage*, Token, Tokens*);

/* Constructor */
Token *mktoken(Garbage*, Tokentype,int8*);
Token *mktext(Garbage *g, int8*);
Token *mktagstart(Garbage *g, int8*);
Token *mktagend(Garbage *g, int8*);
Token *mkselfclosed(Garbage *g, int8*);
Tokens *mktokens(Garbage*);