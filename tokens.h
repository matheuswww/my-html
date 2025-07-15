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
  Tag type;
  int8 value[];
};
typedef struct s_tagend Tagend;

struct s_selfclosed {
  Tag type;
  int8 value[];
};
typedef struct s_selfclosed Selfclosed;

struct s_texttoken {
  Tag type;
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
  Tokentype type;
  union {
    Text *texttoken;
    Tagstart *start;
    Tagend *end;
    Selfclosed *self;
  } contents;
};
typedef struct s_token Token;

struct s_tokens {
  int16 length;
  Token *ts;
};
typedef struct s_tokens Tokens;

#define destroytoken(t) free(t);

int8 *showtoken(Token);
int8 * showtokens(Tokens);

/* Constructor */
Token *mktoken(Garbage*, Tokentype,int8*);
Token *mktext(int8*);
Token *mktagstart(int8*);
Token *mktagend(int8*);
Token *mkselfclosed(int8*);