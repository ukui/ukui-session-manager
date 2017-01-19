XSLTPROC_FLAGS = \
	--nonet \
	--stringparam man.output.quietly 1 \
	--stringparam funcsynopsis.style ansi \
	--stringparam man.th.extra1.suppress 1 \
	--stringparam man.authors.section.enabled 0 \
	--stringparam man.copyright.section.enabled 0

.xml.1:
	$(AM_V_GEN) $(XSLTPROC) $(XSLTPROC_FLAGS) http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl $<

man_MANS =				\
	ukui-session.1			\
	ukui-session-properties.1	\
	ukui-session-save.1		\
	ukui-wm.1			\
	ukui-session-inhibit.1

EXTRA_DIST =				\
	ukui-session-inhibit.xml	\
	$(man_MANS)

CLEANFILES =				\
	ukui-session-inhibit.1

-include $(top_srcdir)/git.mk
