#include "tokens.h"

enum e_stage {
  none = 0,
  newtoken = 1,
  readtoken = 2,
};
typedef enum e_stage Stage;

struct s_state {
  Stage stage;
  Tokentype type;
  int8 buf[256];
  int8 *cur;
};
typedef struct s_state State;

/* constructor */

Tokens *lexer(String*);
Tokens *lexer_(Garbage*, String*, Tokens*, State*);