## Automake template for fig library.

noinst_LIBRARIES = libfig.a

SRCS = fig-attr.c fig-file.c fig-object.c fig-util.c

HDRS = fig-attr.h fig-file.h fig-object.h fig-util.h fig.h

libfig_a_SOURCES = $(SRCS) $(HDRS)

EXTRA_DIST = fig-format.txt

EXTRA_PROGRAMS = testfig

INCLUDES = -I$(srcdir)/../libvars

testfig_SOURCES = testfig.c
testfig_LDFLAGS = -L$(srcdir)/../libvars
testfig_LDADD = libfig.a -lvars -lm

test: testfig
	./testfig | tee test.fig
	fig2dev -L eps test.fig > test.eps

CLEANFILES = test.fig* test.eps testfig

include $(top_srcdir)/etc/Makefile.common
