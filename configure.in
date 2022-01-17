dnl Process this file with autoconf to produce a configure script.

dnl Initialize autoconf.
AC_INIT(ifm, 5.4, zondo42@googlemail.com)
AC_CONFIG_SRCDIR(src/ifm-main.c)
AC_CONFIG_AUX_DIR(etc)

dnl Initialize automake.
AM_INIT_AUTOMAKE(foreign)

dnl Checks for programs.
AC_PROG_CC
AM_PROG_LEX
AC_PROG_YACC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_RANLIB

dnl Checks for header files.
AC_HEADER_STDC

dnl Check for cygwin.
AC_CANONICAL_HOST

case $host_os in
     *cygwin*) CYGWIN=yes;;
     *)        CYGWIN=no;;
esac 

if test "$IFM_CYGWIN" != ""; then
   AC_MSG_NOTICE(overriding Cygwin detection)
   CYGWIN=yes
fi

if test $CYGWIN = yes; then
   AC_DEFINE(CYGWIN, 1, Whether building using Cygwin on Windows)
fi

AM_CONDITIONAL(CYGWIN, test $CYGWIN = yes)

dnl Check for wish.
AC_PATH_PROGS(WISH, wish8.0 wish80 wish, no)

tkifm=tkifm
AC_SUBST(tkifm)

if test $WISH = no; then
   AC_MSG_WARN(wish not found -- not building wish scripts)
   tkifm=
fi

dnl Check for perl.
AC_PATH_PROGS(PERL, perl5.002 perl5 perl, no)

ifm2dev=ifm2dev
AC_SUBST(ifm2dev)

ifm2tex=ifm2tex
AC_SUBST(ifm2tex)

ifm2web=ifm2web
AC_SUBST(ifm2web)

scr2ifm=scr2ifm
AC_SUBST(scr2ifm)

if test $PERL = no; then
   AC_MSG_WARN(perl not found -- not building perl scripts)
   ifm2dev=
   ifm2tex=
   ifm2web=
   scr2ifm=
fi

dnl Write Makefiles.
AC_OUTPUT(
Makefile
doc/Makefile
doc/conf.py
lib/Makefile
man/Makefile
progs/Makefile
src/Makefile
src/libfig/Makefile
src/libvars/Makefile
tests/Makefile
)
