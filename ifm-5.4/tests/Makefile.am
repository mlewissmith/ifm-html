# Automake template for IFM tests.

TESTS_ENVIRONMENT = SRCDIR=$(top_srcdir) BUILDDIR=$(top_builddir)

SRCS = test-attr1.ifm test-attr2.ifm test-attr3.ifm test-before1.ifm	   \
test-before2.ifm test-cmd.ifm test-do1.ifm test-do2.ifm test-do3.ifm	   \
test-drop1.ifm test-drop2.ifm test-drop3.ifm test-drop4.ifm test-exit.ifm  \
test-finish.ifm test-follow1.ifm test-follow2.ifm test-follow3.ifm	   \
test-follow4.ifm test-give1.ifm test-give2.ifm test-it.ifm test-join1.ifm  \
test-join2.ifm test-keep1.ifm test-keep2.ifm test-leave1.ifm		   \
test-leave2.ifm test-link1.ifm test-link2.ifm test-lose.ifm test-nocmd.ifm \
test-nolink1.ifm test-nolink2.ifm test-nopath.ifm test-noroom.ifm	   \
test-note.ifm test-path.ifm test-simple.ifm test-them.ifm test-unsafe.ifm

IFM		= $(top_builddir)/src/ifm
TKIFM		= $(top_builddir)/progs/tkifm
TESTS		= $(SRCS:.ifm=.test)
EXP		= $(SRCS:.ifm=.exp)

EXTRA_DIST	= README Maketests $(TESTS) $(EXP) $(SRCS)

SUFFIXES	= .ifm .test .task .prof .tk .ps .rec

.ifm.prof:;	time $(IFM) -tasks $<

.ifm.task:;	$(IFM) -tasks -debug $<

.ifm.tk:;	$(TKIFM) $<

.ifm.ps:;	$(IFM) -map -format ps $< > $@

.ifm.rec:;	$(IFM) -tasks -format rec $< > $@

tests:		$(SRCS)
		perl Maketests "$(IFM)" $(SRCS)

CLEANFILES	= *.out

include $(top_srcdir)/etc/Makefile.common
