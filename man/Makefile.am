## Automake template for IFM man page.

HELP2MAN = $(OPTIONAL) help2man -N -n "$(MAN_TITLE)"
OPTIONAL = $(top_srcdir)/etc/missing --run

MAN_TITLE = Interactive Fiction Mapper

IFM = $(top_builddir)/src/ifm

MANPAGE = ifm.1

man_MANS = $(MANPAGE)

EXTRA_DIST = ifm.man ifm.1

TMP = /tmp/ifm.tmp

manpage: $(MANPAGE)

$(MANPAGE): ifm.man
	cp $(srcdir)/ifm.man $(TMP)
	chmod u+w $(TMP)
	echo "[SEE ALSO]" >> $(TMP)
	echo ".IP $(ifmdocdir)/index.html" >> $(TMP)
	echo "IFM manual" >> $(TMP)
	$(HELP2MAN) -o $@ -i $(TMP) $(IFM)
	rm -f $(TMP)

mantest: $(MANPAGE)
	groff -man -Tps $(MANPAGE) > ifm.ps
	gv -a4 -scale -1 -spartan ifm.ps
	rm -f ifm.ps

include $(top_srcdir)/etc/Makefile.common

DISTCLEANFILES = $(MANPAGE)
