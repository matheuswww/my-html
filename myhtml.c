/* myhtml.c */
#include "myhtml.h"
#include "tokens.h"

int8 *showtoken(Token) {
    int8 *ret;
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

int main(int argc, char *argv[]) {
    Tuple t;
    String *s;
    int8 c;

    s = mkstring($1 "Hello world");
    t = get(s);
    if(!t.c) {
        printf("err\n");
        return -1;
    }
    c = t.c;

    printf("c = '%c'\nnew = '%s'\n", c, t.s->cur);
    return 0;
}