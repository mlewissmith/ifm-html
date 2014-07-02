## Automake template for IFM.

AUTOMAKE_OPTIONS = check-news

ACLOCAL_AMFLAGS = -I etc

SUBDIRS = $(subdirs) src lib doc man progs tests

EXTRA_DIST = COPYING.DOC TO-DO

EXTRA_DIRS = etc contrib

dist-hook: dist-extra dist-config dist-header dist-touch dist-cleanup

dist-config:
	rm -f $(distdir)/etc/*header*

dist-extra:
	for dir in $(EXTRA_DIRS); do					\
	   cp -frp $(top_srcdir)/$$dir $(distdir);			\
	done

dist-header:
	addheader=$(top_srcdir)/etc/add-header;				\
	for dir in fig src lib progs; do				\
	   if test -f $$addheader; then					\
	      $$addheader $(top_srcdir)/etc $(distdir)/$$dir;		\
	   fi;								\
	done

dist-touch:
	for file in src/ifm-scan.c src/ifm-parse.c src/ifm-parse.h; do	\
	   touch $(distdir)/$$file;					\
	done

dist-cleanup:
	find $(distdir) -type f -name '*~' | xargs rm -f
	rm -f $(distdir)/etc/config.h
	rm -f $(distdir)/etc/stamp-h*

include $(top_srcdir)/etc/Makefile.common
