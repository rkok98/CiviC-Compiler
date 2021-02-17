/**
 * Authors: Rene Kok & Aram Mutlu
 * Scanner for the (ab|ac)* regex
 */

char *scanner(FILE *stream)
{
    static char buffer[max]; //  fixed  size  buffer  for  token
    int pos = 0;
    char c;

// The first start/init state scans for the first character 'a'
state_init:
    switch (c = getc(stream))
    {
    case 'a':
        buffer[pos++] = c;
        goto state_0;
    default:
        goto state_err;
    }

// State to scan for the character 'b' OR 'c'
state_0:
    switch (c = getc(stream))
    {
    case 'b':
        buffer[pos++] = c;
        goto state_1;
    case 'c':
        buffer[pos++] = c;
        goto state_1;
    default:
        goto state_err;
    }

// State to scan for the character 'a' or end of file
state_1:
    switch (c = getc(stream))
    {
    case 'a':
        buffer[pos++] = c;
        goto state_0;
    default:
        goto state_succ;
    }

// State to return the buffer when reached
state_succ:
    return buffer;

// State to return NULL when error occures
state_err:
    return NULL;
}