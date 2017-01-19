bin_PROGRAMS = ukui-session-properties

AM_CPPFLAGS =						\
	$(SESSION_PROPERTIES_CFLAGS)			\
	-I$(top_srcdir)/ukui-session			\
	-DLOCALE_DIR=\""$(datadir)/locale"\"		\
	-DGTKBUILDER_DIR=\""$(pkgdatadir)"\"		\
	$(DISABLE_DEPRECATED_CFLAGS)

AM_CFLAGS = $(WARN_CFLAGS)

ukui_session_properties_SOURCES =			\
	main.c						\
	gsm-properties-dialog.h				\
	gsm-properties-dialog.c				\
	gsm-app-dialog.h				\
	gsm-app-dialog.c				\
	gsp-app.h					\
	gsp-app.c					\
	gsp-app-manager.h				\
	gsp-app-manager.c				\
	gsp-keyfile.h					\
	gsp-keyfile.c

ukui_session_properties_LDADD =			\
	$(SESSION_PROPERTIES_LIBS)			\
	$(top_builddir)/ukui-session/libgsmutil.la

-include $(top_srcdir)/git.mk
