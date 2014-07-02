%{
/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/* Lexical scanner */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <vars.h>

#include "ifm-main.h"
#include "ifm-util.h"
#include "ifm-parse.h"

/* Internal string buffer */
static vbuffer *sbuf = NULL;
%}

%option noyywrap

LETTER		[A-Za-z_]
DIGIT		[0-9]
INTEGER         {DIGIT}+
REAL		{DIGIT}*\.{DIGIT}+
ALNUM		{LETTER}|{DIGIT}
ID		{LETTER}{ALNUM}*
VAR             \${ID}

%%

#.* /* Ignore comments */

[ \t\f\r]+ /* Strip whitespace */

\n {
    line_number++;
}

\" {
    /* Quoted string */

    int c, quote = 0, eatspace = 0;

    /* Initialise buffer */
    if (sbuf == NULL)
        sbuf = vb_create();
    else
        vb_empty(sbuf);

    /* Slurp string in, one char at a time */
    while (1) {
        c = input();

        if        (c == EOF) {
            yyerror("unterminated string");
            break;
        } else if (c == '"' && !quote) {
            break;
        } else if (c == '\\' && !quote) {
            quote = 1;
            continue;
        } else if (c == '\n') {
            vb_putc(sbuf, ' ');
            eatspace = 1;
            line_number++;
        } else if (!eatspace || !isspace(c)) {
            vb_putc(sbuf, c);
        }

        if (!isspace(c))
            eatspace = 0;

        quote = 0;
    }

    yylval.sval = vb_getcopy(sbuf);
    return STRING;
}

"after"		{ return AFTER; }
"all"		{ return ALL; }
"any"		{ return ANY; }
"before"	{ return BEFORE; }
"cmd"		{ return CMD; }
"d"		{ return DOWN; }
"dir"		{ return DIR; }
"do"		{ return DO; }
"down"		{ return DOWN; }
"drop"		{ return DROP; }
"e"		{ return EAST; }
"east"		{ return EAST; }
"endstyle"      { return ENDSTYLE; }
"except"	{ return EXCEPT; }
"exit"		{ return EXIT; }
"false"         { return FALSE; }
"finish"	{ return FINISH; }
"follow"	{ return FOLLOW; }
"from"		{ return FROM; }
"get"		{ return GET; }
"give"		{ return GIVE; }
"given"		{ return GIVEN; }
"go"		{ return GO; }
"goto"		{ return GOTO; }
"hidden"	{ return HIDDEN; }
"ignore"	{ return IGNORE; }
"in"		{ return IN; }
"it"		{ return IT; }
"item"		{ return ITEM; }
"join"		{ return JOIN; }
"keep"		{ return KEEP; }
"last"		{ return LAST; }
"leave"		{ return LEAVE; }
"length"	{ return LENGTH; }
"link"		{ return LINK; }
"lose"		{ return LOSE; }
"lost"		{ return LOST; }
"map"		{ return MAP; }
"n"		{ return NORTH; }
"ne"		{ return NORTHEAST; }
"need"		{ return NEED; }
"nodrop"        { return NODROP; }
"nolink"        { return NOLINK; }
"nopath"        { return NOPATH; }
"none"          { return NONE; }
"north"		{ return NORTH; }
"northeast"	{ return NORTHEAST; }
"northwest"	{ return NORTHWEST; }
"note"		{ return NOTE; }
"nw"		{ return NORTHWEST; }
"oneway"	{ return ONEWAY; }
"out"		{ return OUT; }
"require"       { return REQUIRE; }
"room"		{ return ROOM; }
"s"		{ return SOUTH; }
"safe"		{ return SAFE; }
"score"		{ return SCORE; }
"se"		{ return SOUTHEAST; }
"south"		{ return SOUTH; }
"southeast"	{ return SOUTHEAST; }
"southwest"	{ return SOUTHWEST; }
"start"		{ return START; }
"style"		{ return STYLE; }
"sw"		{ return SOUTHWEST; }
"tag"		{ return TAG; }
"task"		{ return TASK; }
"them"          { return THEM; }
"times"         { return TIMES; }
"title"		{ return TITLE; }
"to"		{ return TO; }
"true"          { return TRUE; }
"u"		{ return UP; }
"undef"		{ return UNDEF; }
"until"		{ return UNTIL; }
"up"		{ return UP; }
"w"		{ return WEST; }
"west"		{ return WEST; }
"with"          { return WITH; }

{ID}\.{ID} {
    yylval.sval = v_strdup(yytext);
    return ID;
}

{ID} {
    yylval.sval = v_strdup(yytext);
    return ID;
}

{REAL} {
    yylval.dval = atof(yytext);
    return REAL;
}

{INTEGER} {
    yylval.ival = atoi(yytext);
    return INTEGER;
}

. { return yytext[0]; }

%%
