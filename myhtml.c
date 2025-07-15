/* myhtml.c */
#include "myhtml.h"
#include "tokens.h"

int8 *showtokens(Tokens ts) {
    int8 *p, *cur;
    static int8 buf[20480];
    int16 total, n, i;
    Token *t;

    total = 0;
    cur = buf;
    zero(buf, sizeof(buf));

    for (i=ts.length, t=ts.ts; i; i--, t++) {
        p = showtoken(*t);
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

int8 *showtoken(Token t) {
    int8 *ret;
    static int8 tmp[256];

    assert(t.type);

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

/*
<body>
 <b>Text</b>
</body>

Lexer

Token       Value
_________________
tagstart    "body"
tagstart    "b"
text        "Text"
tagend      "b"
tagend      "body"
*/

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
    void *ptr;
    Token *ret;

    ret = (Token *)0;
    switch (type) {
        case text: ret = mktext(value); break;
        case tagstart: ret = mktagstart(value); break;
        case tagend: ret = mktagend(value); break;
        case selfclosed: ret = mkselfclosed(value); break;
        default:
            fprintf(stderr, "mktoken(): bad input\n");
            exit(-1);
            
            break;
    }

    if (!ret) {
        return (Token *)0;
    }

    ptr = ret->contents.texttoken;
    addgc(g, ptr);

    return ret;
}

Token *mktagstart(int8 *value) {
    int16 msize, size;
    Tagstart *p;
    
    size = stringlen(value);
    msize = sizeof(struct s_tagstart) + size;
    p = (Tagstart *)malloc($i msize);
    assert(p);

    zero($1 p, msize);
    stringcopy(p->value, $1 value, msize);
    
    static Token html = { 
        .type = tagstart,
    };
    html.contents.start = p;

    return (Token *)&html;
}


Token *mktagend(int8 *value) {
    int16 size, msize;
    Tagend *p;
    
    size = stringlen(value);
    msize = sizeof(struct s_tagend) + size;
    p = (Tagend *)malloc($i msize);
    assert(p);

    zero($1 p, sizeof(struct s_tagend));
    stringcopy(p->value, value, msize);
    
    static Token html = { 
        .type = tagend,
    };
    html.contents.end = p;

    return (Token *)&html;
}

Token *mkselfclosed(int8 *value) {
    int16 size, msize;
    Selfclosed *p;
    
    size = stringlen(value);
    msize = sizeof(struct s_selfclosed) + size;
    p = (Selfclosed *)malloc($i msize);
    assert(p);

    zero($1 p, sizeof(struct s_selfclosed));
    stringcopy(p->value, value, msize);
    
    static Token html = { 
        .type = selfclosed,
    };
    html.contents.self = p;

    return (Token *)&html;
}

Token *mktext(int8 *value) {
    int16 size, msize;
    Text *p;
    
    size = stringlen(value);
    msize = sizeof(struct s_texttoken) + size;
    p = (Text *)malloc($i msize);
    assert(p);

    zero($1 p, msize);
    stringcopy(p->value, value, size);
    
    static Token html = { 
        .type = text
    };
    html.contents.texttoken = p;

    return (Token *)&html;
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
    int16 size, gcbs;
    assert(g && ptr);

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
        free(g->p[n]);
    free(g);

    p = mkgarbage();

    return p;
}

int main() {

    int16 size;
    Token *t;
    Token *t1, *t2, *t3, *t4, *t5, *t6;
    Garbage * garb;

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

    Tokens ts = {
        .length = 6,
        .ts = t,
    };

    printf("'%s'\n", showtokens(ts));

    gc(garb);

    return 0;
}