%{
/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/* Freeze/thaw scanner */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>

#include "vars-config.h"
#include "vars-buffer.h"
#include "vars-freeze.h"
#include "vars-freeze-lex.h"
#include "vars-hash.h"
#include "vars-macros.h"
#include "vars-system.h"
#include "vars-utils.h"

int start_line = 1;             /* Starting line number */
int last_line_number;           /* Last line number */
int line_number;                /* Current line number */

static vbuffer *sbuf = NULL;    /* String buffer */
%}

%option noyywrap

SIGN            [-+]
DIGIT		[0-9]
DECIMAL         \.{DIGIT}*
EXP		[Ee]{SIGN}?{DIGIT}*
LETTER		[A-Za-z_]
ALNUM		{LETTER}|{DIGIT}

%%

\n {
    /* Newline */
    line_number++;
}

\#.* {
    /* Comment */
}

[ \t\f\r]+ {
    /* Whitespace */
}

{SIGN}?{DIGIT}*{DECIMAL}?{EXP} {
    /* Real number with exponent */
    if (sscanf(yytext, "%lg", &v_thaw_dvalue) == 1) {
        v_thaw_ivalue = (int) v_thaw_dvalue;
        return V_TOKEN_REAL;
    } else {
        return V_TOKEN_ERROR;
    }
}

{SIGN}?{DIGIT}*{DECIMAL} {
    /* Real number with decimal point and no exponent */
    if (sscanf(yytext, "%lg", &v_thaw_dvalue) == 1) {
        v_thaw_ivalue = (int) v_thaw_dvalue;
        return V_TOKEN_REAL;
    } else {
        return V_TOKEN_ERROR;
    }
}

{SIGN}?{DIGIT}+ {
    /* Integer */
    if (sscanf(yytext, "%d", &v_thaw_ivalue) == 1) {
        v_thaw_dvalue = (double) v_thaw_ivalue;
        return V_TOKEN_INT;
    } else {
        return V_TOKEN_ERROR;
    }
}

\" {
    /* String */
    int c;

    vb_init(sbuf);
    while ((c = input()) != EOF) {
        switch (c) {
        case '"':
            v_thaw_svalue = vb_get(sbuf);
            return V_TOKEN_STRING;
        case '\\':
            switch (c = input()) {
            case 'n':
                vb_putc(sbuf, '\n');
                break;
            case EOF:
                return V_TOKEN_ERROR;
            default:
                vb_putc(sbuf, c);
                break;
            }
            break;
        case '\n':
            line_number++;
            vb_putc(sbuf, c);
            break;
        default:
            vb_putc(sbuf, c);
            break;
        }
    }

    return V_TOKEN_ERROR;
}

{LETTER}{ALNUM}* {
    /* Identifier */
    vtype *type;

    v_thaw_svalue = yytext;

    if ((type = v_find_name(yytext)) != NULL) {
        v_thaw_pvalue = type;
        return V_TOKEN_VTYPE;
    }

    if      (V_STREQ(yytext, V_TOKEN_REF_STRING))
        return V_TOKEN_REF;
    else if (V_STREQ(yytext, V_TOKEN_UNDEF_STRING))
        return V_TOKEN_UNDEF;
    else if (V_STREQ(yytext, V_TOKEN_NULL_STRING))
        return V_TOKEN_NULL;

    return V_TOKEN_ID;
}

. {
    /* Single character */
    return yytext[0];
}

%%

char *
thaw_text(void)
{
    return yytext;
}

int
thaw_token(FILE *fp)
{
    yyin = fp;
    return yylex();
}

void
thaw_restart(void)
{
    yyrestart(yyin);
    last_line_number = line_number;
    line_number = 0;
    start_line = 1;
}
