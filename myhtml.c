/* myhtml.c */
#include "lexer.h"
#include "myhtml.h"
#include "parser.h"

Map tagmap[] = {
  { $1 "html", html },
  { $1 "body", body },
  { $1 "b", b },
  { $1 "br", br }
};

bool stringcompare(int8 *x, int8 *y) {
    int16 max, n;
    int8 *px, *py;
    max = min(
    stringlen(x),
    stringlen(y)
    );

    for (n=max, px =x, py=y; n; n--, py++, px++)
        if(*px != *py)
            return false;

    return true;
}

Tag findtype(int8 *str) {
    int16 n;
    for(n=0; n < sizeof(tagmap) / sizeof(tagmap[0]); n++)
        if (stringcompare(tagmap[n].str, str))
            return tagmap[n].tag;

    return (Tag )0;
}

Tokens *tcons(Garbage *g, Token x, Tokens* xs) {
    int16 size;
    Token *x_, *ts;
    Tokens *xs_;

    assert(g && x.type && xs);
    switch(x.type) {
        case text:       x_ = mktoken(g, x.type, x.contents.texttoken->value); break;
        case tagstart:   x_ = mktoken(g, x.type, x.contents.start->value); break;
        case tagend:     x_ = mktoken(g, x.type, x.contents.end->value); break;
        case selfclosed: x_ = mktoken(g, x.type, x.contents.self->value); break;
        default: x_ = mktoken(g, x.type, x.contents.texttoken->value);
    }
    if(!x_)
        return (Tokens *)0;
    if (!xs->length) {
        xs_ = mktokens(g);
        xs_->length = 1;
        size = sizeof(struct s_token);
        ts = (Token *)malloc($i size);
        zero($1 ts, size);
        *ts = *x_;
        xs_->ts = ts;
        addgc(g, xs);
        
        return xs_;
    }

    xs_ = tcopy(g, xs);
    xs_->length++;
    size = sizeof(struct s_token) * xs_->length;
    ts = (Token *)realloc(xs->ts, size);
    addgc(g, xs);
    if(!ts) 
        return (Tokens *)0;
    xs_->ts = ts;
    xs_->ts[xs->length] = *x_;

    return xs_;
}

int8 *showtokens(Garbage *g, Tokens ts) {
    int8 *p, *cur;
    static int8 buf[20480];
    int16 total, n, i;
    Token *t;

    assert(g && ts.length);
    total = 0;
    cur = buf;
    zero(buf, sizeof(buf));

    for (i=ts.length, t=ts.ts; i; i--, t++) {
        p = showtoken(g, *t);
        if (!p)
            break;
        if (!(*p)) 
            continue;
            
        n = stringlen(p);
        total += n;
        if (total >= sizeof(buf)) {
            break;
        }
        stringcopy(cur, p, n);
        cur += n;
    }

    return buf;
}

int8 *showtoken(Garbage *g, Token t) {
    int8 *ret;
    int8 *tmp;

    assert(t.type);
    tmp = (int8 *)malloc(256);
    addgc(g, tmp);

    ret = tmp;
    zero(tmp, 256);
    switch(t.type) {
        case text:       snprintf($c tmp, 255, "%s", t.contents.texttoken->value); break;
        case tagstart:   snprintf($c tmp, 255, "<%s>", t.contents.start->value); break;
        case tagend:     snprintf($c tmp, 255, "</%s>", t.contents.end->value); break;
        case selfclosed: snprintf($c tmp, 255, "<%s />", t.contents.self->value); break;
        default: break;
    }

    return ret;
}

int16 stringlen(int8 *str) {
    int16 n;
    int8 *p;
    assert(str);
    for(p = str, n = 0; *p; p++, n++);
    
    return n;
}

void stringcopy(int8 *dst, int8 *src, int16 size) {
    int16 n;
    int8 *d, *s;

    assert(src && dst && size);
    for (d=dst, s=src, n=size; n; d++, s++, n--)
        *d = *s;
    return;
}

Tokens *tcopy(Garbage *g, Tokens *old) {
    Tokens *new;
    Token *t;
    int16 size;

    assert(old && old->length);

    size = sizeof(struct s_tokens);
    new = (Tokens *)malloc($i size);
    assert(new);

    zero($1 new, size);
    new->length = old->length;
    size = (new->length * sizeof(struct s_token));
    t = (Token *)malloc($i size);
    assert(t);
    zero($1 t, size);

    memorycopy(t, old->ts, size);
    new->ts = t;

    //addgc(g, old->ts);
    addgc(g, old);

    return new;
}

void memorycopy(void *dst, void *src, int16 size) {
    int8 *d, *s;
    int16 n;

    for (d=$1 dst, s=$1 src, n=size; n; d++, s++, n--)
        *d = *s;
    
    return;
}

String *scopy(String *s) {
    String *p;
    int16 size;

    assert(s && s->length);

    size = sizeof(struct s_string) + s->length;
    p = (String *)malloc($i size);
    assert(p);

    zero($1 p, size);
    p->length = s->length;
    stringcopy(p->data, s->cur, s->length);
    p->cur = p->data;

    return p;
}

Tuple get(String *s) {
    String *new;
    int8 c;
    assert(s);
    if(!s->length)
    goto fail;
    
    c = *s->cur;
    new = scopy(s);
    if (!new)
        goto fail;
    new->cur++;
    new->length--;

    Tuple ret = {
        .s = new,
        .c = c
    };
    sdestroy(s);

    return ret;

    fail:
        Tuple err = {0};

        return err;
}

TTuple tget(Garbage *g, Tokens *old) {
    Tokens *new;
    Token *x;

    assert(g && old);
    if (!old->length)
     goto fail;

    switch(old->ts->type) {
        case text:       x = mktoken(g, old->ts->type, old->ts->contents.texttoken->value); break;
        case tagstart:   x = mktoken(g, old->ts->type, old->ts->contents.start->value); break;
        case tagend:     x = mktoken(g, old->ts->type, old->ts->contents.end->value); break;
        case selfclosed: x = mktoken(g, old->ts->type, old->ts->contents.self->value); break;
        default: x = mktoken(g, old->ts->type, old->ts->contents.texttoken->value);
    }
    new = tcopy(g, old);
    if (!new)
        goto fail;
    new->ts++;
    new->length--;

    TTuple ret = {
        .xs = new,
        .x = *x
    };

    return ret;

    fail:
        TTuple err = {0};

        return err;
}

int8 peek(String *s) {
    assert(s);

    if (!s->length)
        return (int8 )0;
    else
        return *s->cur;
}

String *mkstring(int8 *str) {
    String *p;
    int16 n, size;

    assert(str);
    n = stringlen(str);
    assert(n);
    size = sizeof(struct s_string) + n;
    p = (String *)malloc($i size);
    assert(p);

    zero($1 p, size);
    p->length = n;
    stringcopy(p->data, str, n);
    p->cur = p->data;

    return p;
}

Token *mktoken(Garbage *g, Tokentype type, int8 *value) {
    Token *ret;
    ret = (Token *)0;

    switch (type) {
        case text: ret = mktext(g, value); break;
        case tagstart: ret = mktagstart(g, value); break;
        case tagend: ret = mktagend(g, value); break;
        case selfclosed: ret = mkselfclosed(g, value); break;
        default:
            fprintf(stderr, "mktoken(): bad input\n");
            exit(-1);
            
            break;
    }

    if (!ret) {
        return (Token *)0;
    }

    return ret;
}

Token *mktagstart(Garbage *g, int8 *value) {
    int16 size, msize;
    Tagstart *p;
    Token *ret;
    
    size = stringlen(value);
    msize = sizeof(struct s_tagstart) + size;
    p = (Tagstart *)malloc($i msize);
    assert(p);

    zero($1 p, msize);
    stringcopy(p->value, value, size);
    p->type = findtype(p->value);

    size = sizeof(struct s_token);
    ret = (Token *)malloc($i size);
    zero($1 ret, size);
    ret->type = tagstart;
    ret->contents.start = p;
    addgc(g, ret);

    return ret;
}

Token *mktagend(Garbage *g, int8 *value) {
    int16 size, msize;
    Tagend *p;
    Token *ret;
    
    size = stringlen(value);
    msize = sizeof(struct s_tagend) + size;
    p = (Tagend *)malloc($i msize);
    assert(p);
    p->type = findtype(p->value);

    zero($1 p, sizeof(struct s_tagend));
    stringcopy(p->value, value, size);
    
    size = sizeof(struct s_token);
    ret = (Token *)malloc($i size);
    zero($1 ret, size);
    ret->type = tagend;
    ret->contents.end = p;
    addgc(g, ret);

    return ret;
}

Token *mkselfclosed(Garbage *g, int8 *value) {
    int16 size, msize;
    Selfclosed *p;
    Token *ret;
    
    size = stringlen(value);
    msize = sizeof(struct s_selfclosed) + size;
    p = (Selfclosed *)malloc($i msize);
    assert(p);

    zero($1 p, sizeof(struct s_selfclosed));
    stringcopy(p->value, value, size);
    p->type = findtype(p->value);
    
    size = sizeof(struct s_token);
    ret = (Token *)malloc($i size);
    zero($1 ret, size);
    ret->type = selfclosed;
    ret->contents.self = p;
    addgc(g, ret);

    return ret;
}

Token *mktext(Garbage *g, int8 *value) {
    int16 size, msize;
    Text *p;
    Token *ret;
    size = stringlen(value);
    msize = sizeof(struct s_texttoken) + size;
    p = (Text *)malloc($i msize);
    assert(p);
    
    zero($1 p, msize);
    stringcopy(p->value, value, size);
    p->type = findtype(p->value);
    
    size = sizeof(struct s_token);
    ret = (Token *)malloc($i size);
    zero($1 ret, size);
    ret->type = text;
    ret->contents.texttoken = p;
    addgc(g, ret);
    return ret;
}

Garbage *mkgarbage() {
    Garbage *p;
    int16 size;

    size = sizeof(struct s_garbage) * GCblocksize;
    p = (Garbage *)malloc($i size);
    assert(p);
    zero($1 p, size);

    *p->p = (void *)0;
    p->capacity = GCblocksize;
    p->size = 0;

    return p;
}

void addgc(Garbage *g, void *ptr) {
    int16 size, gcbs, n;
    assert(g && ptr);
    bool exists;

    exists = false;
    for (n = 0; n < g->size; n++)
        if (g->p[n] == ptr) {
            exists = true;
            break;
        }

    if (exists)
        return;


    if(g->size >= g->capacity) {
        gcbs = GCblocksize;
        size = sizeof(struct s_garbage) * (g->capacity + gcbs);
        g = (Garbage *)realloc(g, $i size);
        assert(g);
        g->capacity += GCblocksize;
    }

    g->p[g->size] = ptr;
    g->size++;

    return;
}

Garbage *gc(Garbage *g) {
    int16 n;
    Garbage *p;

    for (n = g->size - 1; n; n--)
       if(g->p[n])
            free(g->p[n]);
    free(g);

    p = mkgarbage();

    return p;
}

Tokens *mktokens(Garbage *g) {
    int16 size;
    Tokens *p;

    size = sizeof(struct s_tokens);
    p = (Tokens *)malloc($i size); 
    if(!p)
        return (Tokens *)0;
    zero($1 p, size);

    p->length = 0;
    p->ts = (Token *)0;
    addgc(g, p);
    if (g)
        addgc(g, p);

    return p;
}

int main() {
    String *s;
    Tokens *xs;
    Garbage *g;
    Token *t;
    Stack *old, *new, *new1;
    STuple *tuple;

    g = mkgarbage();
    if (!g) {
      printf("No G\n");
      return -1;  
    }
    s = mkstring($1 "<html><body>abc<br />cde</body><b>sjdlkf</b></html>");
    xs = lexer(s);
    if (!xs) {
        printf("No XS!\n");
        return -1;
    }
    assert(xs->length);
    old = parse(xs);
    assert(old);
    assert(old->length);
    printstack(old);
    return 0;

    t = xs->ts;
    printf("%s\n", (*(t)).contents.start->value);
    old = mkstack(1);
    old->fun = findfun(*t);
    memorycopy($1 &old->token, $1 t, sizeof(struct s_token));
    new1 = push(g, old, *(t+1));
    if (new1) {
        new = push(g, new1, *(t+2));
        printstack(new);
    } else
    printf("ERR: push() failed \n");

 
    if (isopen(new, body))
        printf("<body> is on the stack\n");
    else
        printf("No <body>\n");

    tuple = apop(g, new, body);
    if (!tuple->x.contents.start->type)
        printf("apop() failed\n");
    else {
        printf("STuple type = %d\n", tuple->x.contents.self->type);
        if (tuple->xs) {
            printf("\n\n");
            printstack(tuple->xs);
            addgc(g, tuple->xs);
        }
    }
    
    if (isopen(tuple->xs, body))
        printf("<body> is on the stack\n");
    else
        printf("No <body>\n");

    gc(g);
    
    return 0;
}

/*
int main() {
    Token *x, *x2;
    Tokens *xs;
    Garbage *g;

    g = mkgarbage();
    xs = mktokens(g);
    x = mktoken(g, tagstart, $1 "html");
    x2 = mktoken(g, selfclosed,$1 "br");
    xs = tcons(g, *x2, xs);
    xs = tcons(g, *x, xs);

    printf("'%s'\n", showtokens(g, *xs));

    return 0;
}
*/

/*
int main() {

    int16 size;
    Token *t;
    Token *t1, *t2, *t3, *t4, *t5, *t6;
    Garbage * garb;
    Tokens *old;
    TTuple tt;

    garb = mkgarbage();
    t1 = mktoken(garb, tagstart,$1 "html");
    t2 = mktoken(garb, tagstart,$1 "body");
    t3 = mktoken(garb, text,$1 "Hello world");
    t4 = mktoken(garb, selfclosed,$1 "br");
    t5 = mktoken(garb, tagend,$1 "body");
    t6 = mktoken(garb, tagend,$1 "html");
    
    size = sizeof(Token)*6; 
    t = (Token *)malloc(size);
    assert(t);
    zero( $1 t, size);

    t[0] = *t1;
    t[1] = *t2;
    t[2] = *t3;
    t[3] = *t4;
    t[4] = *t5;
    t[5] = *t6;

    old = (Tokens *)malloc(sizeof(struct s_tokens));
    zero($1 old, sizeof(struct s_tokens));

    old->length = 6;
    old->ts = t;
    tt = tget(garb, old);
    printf("'%s'\n", showtoken(garb, tt.x));
    printf("'%s'\n", showtokens(garb, *tt.xs));

    gc(garb);

    return 0;
}
*/