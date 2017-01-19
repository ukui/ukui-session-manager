bin_PROGRAMS = ukui-session-save ukui-session-inhibit

AM_CPPFLAGS =					\
	$(UKUI_SESSION_CFLAGS)			\
	$(DBUS_GLIB_CFLAGS)			\
	-DLOCALE_DIR=\""$(datadir)/locale"\"	\
	$(DISABLE_DEPRECATED_CFLAGS)

AM_CFLAGS = $(WARN_CFLAGS)

ukui_session_save_SOURCES =			\
	ukui-session-save.c

ukui_session_save_LDADD =			\
	$(SM_LIBS)				\
	$(ICE_LIBS)				\
	$(UKUI_SESSION_LIBS)			\
	$(DBUS_GLIB_LIBS)

ukui_session_inhibit_SOURCES =			\
	ukui-session-inhibit.c

ukui_session_inhibit_CPPFLAGS =			\
	$(AM_CPPFLAGS)				\
	$(UKUI_SESSION_CFLAGS)			\
	-DLOCALE_DIR=\""$(datadir)/locale"\"	\
	$(DISABLE_DEPRECATED_CFLAGS)

ukui_session_inhibit_LDADD =			\
	$(UKUI_SESSION_LIBS)

-include $(top_srcdir)/git.mk
