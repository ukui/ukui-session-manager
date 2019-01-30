bin_PROGRAMS = ukui-session-save ukui-session-inhibit ukui-session-shortcuts

AM_CPPFLAGS =					\
	$(UKUI_SESSION_CFLAGS)			\
	-DLOCALE_DIR=\""$(datadir)/locale"\"	\
	$(DISABLE_DEPRECATED_CFLAGS)

AM_CFLAGS = $(WARN_CFLAGS)

ukui_session_save_SOURCES =			\
	ukui-session-save.c

ukui_session_save_LDADD =			\
	$(SM_LIBS)				\
	$(ICE_LIBS)				\
	$(UKUI_SESSION_LIBS)

ukui_session_inhibit_SOURCES =			\
	ukui-session-inhibit.c

ukui_session_inhibit_CPPFLAGS =			\
	$(AM_CPPFLAGS)				\
	$(UKUI_SESSION_CFLAGS)			\
	-DLOCALE_DIR=\""$(datadir)/locale"\"	\
	$(DISABLE_DEPRECATED_CFLAGS)

ukui_session_inhibit_LDADD =			\
	$(UKUI_SESSION_LIBS)

ukui_session_shortcuts_SOURCES =		\
	ukui-session-shortcuts.c			\
    uksm-shortcuts-dialog.c				\
	uksm-shortcuts-dialog.h


ukui_session_shortcuts_CPPFLAGS =		\
	$(UKUI_SESSION_CFLAGS)				\
	-I$(top_srcdir)/ukui-session		\
	-DLOCALE_DIR=\""$(datadir)/locale"\"	\
	-DDATA_DIR=\""$(datadir)/ukui-session-manager"\"

ukui_session_shortcuts_LDADD =			\
	$(UKUI_SESSION_LIBS)				\
	$(top_builddir)/ukui-session/libgsmblur.la

-include $(top_srcdir)/git.mk
