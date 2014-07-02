/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/* A Bison parser, made by GNU Bison 1.875c.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     ROOM = 258,
     ITEM = 259,
     LINK = 260,
     FROM = 261,
     TAG = 262,
     TO = 263,
     DIR = 264,
     ONEWAY = 265,
     HIDDEN = 266,
     NODROP = 267,
     NOTE = 268,
     TASK = 269,
     AFTER = 270,
     NEED = 271,
     GET = 272,
     SCORE = 273,
     JOIN = 274,
     GO = 275,
     REQUIRE = 276,
     ANY = 277,
     LAST = 278,
     START = 279,
     GOTO = 280,
     MAP = 281,
     EXIT = 282,
     GIVEN = 283,
     LOST = 284,
     KEEP = 285,
     LENGTH = 286,
     TITLE = 287,
     LOSE = 288,
     SAFE = 289,
     BEFORE = 290,
     FOLLOW = 291,
     CMD = 292,
     LEAVE = 293,
     UNDEF = 294,
     FINISH = 295,
     GIVE = 296,
     DROP = 297,
     ALL = 298,
     EXCEPT = 299,
     IT = 300,
     UNTIL = 301,
     TIMES = 302,
     NOLINK = 303,
     NOPATH = 304,
     NONE = 305,
     STYLE = 306,
     ENDSTYLE = 307,
     WITH = 308,
     IGNORE = 309,
     DO = 310,
     THEM = 311,
     NORTH = 312,
     EAST = 313,
     SOUTH = 314,
     WEST = 315,
     NORTHEAST = 316,
     NORTHWEST = 317,
     SOUTHEAST = 318,
     SOUTHWEST = 319,
     UP = 320,
     DOWN = 321,
     IN = 322,
     OUT = 323,
     INTEGER = 324,
     TRUE = 325,
     FALSE = 326,
     REAL = 327,
     STRING = 328,
     ID = 329
   };
#endif
#define ROOM 258
#define ITEM 259
#define LINK 260
#define FROM 261
#define TAG 262
#define TO 263
#define DIR 264
#define ONEWAY 265
#define HIDDEN 266
#define NODROP 267
#define NOTE 268
#define TASK 269
#define AFTER 270
#define NEED 271
#define GET 272
#define SCORE 273
#define JOIN 274
#define GO 275
#define REQUIRE 276
#define ANY 277
#define LAST 278
#define START 279
#define GOTO 280
#define MAP 281
#define EXIT 282
#define GIVEN 283
#define LOST 284
#define KEEP 285
#define LENGTH 286
#define TITLE 287
#define LOSE 288
#define SAFE 289
#define BEFORE 290
#define FOLLOW 291
#define CMD 292
#define LEAVE 293
#define UNDEF 294
#define FINISH 295
#define GIVE 296
#define DROP 297
#define ALL 298
#define EXCEPT 299
#define IT 300
#define UNTIL 301
#define TIMES 302
#define NOLINK 303
#define NOPATH 304
#define NONE 305
#define STYLE 306
#define ENDSTYLE 307
#define WITH 308
#define IGNORE 309
#define DO 310
#define THEM 311
#define NORTH 312
#define EAST 313
#define SOUTH 314
#define WEST 315
#define NORTHEAST 316
#define NORTHWEST 317
#define SOUTHEAST 318
#define SOUTHWEST 319
#define UP 320
#define DOWN 321
#define IN 322
#define OUT 323
#define INTEGER 324
#define TRUE 325
#define FALSE 326
#define REAL 327
#define STRING 328
#define ID 329




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 78 "ifm-parse.y"
typedef union YYSTYPE {
    int ival;
    double dval;
    char *sval;
    vscalar *vval;
} YYSTYPE;
/* Line 1275 of yacc.c.  */
#line 192 "ifm-parse.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



