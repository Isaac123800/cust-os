#include "../include/custc.h"
#include "../include/console.h"

static char *skip_spaces(char *p)
{
    while (*p == ' '  ||
           *p == '\t' ||
           *p == '\n' ||
           *p == '\r')
    {
        p++;
    }

    return p;
}

static bool expect(char **p, char expected)
{
    *p = skip_spaces(*p);

    if (**p != expected)
        return false;

    (*p)++;
    return true;
}

static bool parse_state(char **p)
{
    *p = skip_spaces(*p);

    // Expect the keyword "state".
    if ((*p)[0] != 's' ||
        (*p)[1] != 't' ||
        (*p)[2] != 'a' ||
        (*p)[3] != 't' ||
        (*p)[4] != 'e')
    {
        return false;
    }

    *p += 5;

    // Prevent accepting identifiers such as "statex".
    if (**p != '(' &&
        **p != ' ' &&
        **p != '\t' &&
        **p != '\n' &&
        **p != '\r')
    {
        return false;
    }

    if (!expect(p, '('))
        return false;

    if (!expect(p, '"'))
        return false;

    // Display the text inside the quotation marks.
    while (**p != '"' && **p != '\0')
    {
        if (**p == '\n' || **p == '\r')
            return false;

        putchar(**p);
        (*p)++;
    }

    if (**p != '"')
        return false;

    (*p)++;

    if (!expect(p, ')'))
        return false;

    if (!expect(p, ';'))
        return false;

    putchar('\n');

    return true;
}

bool custc_run(char *source)
{
    if (source == NULL)
        return false;

    char *p = source;

    while (1)
    {
        p = skip_spaces(p);

        if (*p == '\0')
            return true;

        if (!parse_state(&p))
        {
            print("CustC: Syntax error.\n");
            return false;
        }
    }
}
