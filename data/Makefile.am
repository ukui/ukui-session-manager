SUBDIRS = \
	icons \
	images

bin_SCRIPTS = ukui-wm

uidir = $(pkgdatadir)
ui_DATA = \
	session-properties.ui	\
	gsm-inhibit-dialog.ui

xsessiondir = $(datadir)/xsessions
xsession_in_files = ukui.desktop.in
xsession_DATA = $(xsession_in_files:.desktop.in=.desktop)

desktopdir = $(datadir)/applications
desktop_in_files = ukui-session-properties.desktop.in ukui-wm.desktop.in
desktop_DATA = ukui-session-properties.desktop

if USE_UKUI_WM
desktop_DATA += ukui-wm.desktop
endif

@INTLTOOL_DESKTOP_RULE@

gsettingsschema_in_files = org.ukui.session.gschema.xml.in
gsettings_SCHEMAS = $(gsettingsschema_in_files:.xml.in=.xml)
@INTLTOOL_XML_NOMERGE_RULE@
@GSETTINGS_RULES@

EXTRA_DIST =					\
	ukui-wm						\
	$(xsession_in_files)		\
	$(gsettingsschema_in_files)	\
	$(ui_DATA)					\
	$(desktop_in_files)			\
	$(pixmap_DATA_dist)

CLEANFILES =					\
	$(xsession_DATA)			\
	$(desktop_DATA)				\
	$(gsettings_SCHEMAS)

-include $(top_srcdir)/git.mk
