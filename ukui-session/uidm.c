/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2005 Raffaele Sandrini
 * Copyright (C) 2005 Red Hat, Inc.
 * Copyright (C) 2002, 2003 George Lebl
 * Copyright (C) 2001 Queen of England,
 * Copyright (C) 2016 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 * Authors:
 *      Raffaele Sandrini <rasa@gmx.ch>
 *      George Lebl <jirka@5z.com>
 *      Mark McLoughlin <mark@skynet.ie>
 */

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <X11/Xauth.h>
#include <gdk/gdk.h>

#include "uidm.h"

#define UIDM_PROTOCOL_UPDATE_INTERVAL 1 /* seconds */

#define UIDM_PROTOCOL_SOCKET_PATH "/var/run/uidm_socket"

#define UIDM_PROTOCOL_MSG_CLOSE "CLOSE"
#define UIDM_PROTOCOL_MSG_VERSION "VERSION"
#define UIDM_PROTOCOL_MSG_AUTHENTICATE "AUTH_LOCAL"
#define UIDM_PROTOCOL_MSG_QUERY_ACTION "QUERY_LOGOUT_ACTION"
#define UIDM_PROTOCOL_MSG_SET_ACTION "SET_SAFE_LOGOUT_ACTION"
#define UIDM_PROTOCOL_MSG_FLEXI_XSERVER "FLEXI_XSERVER"

#define UIDM_ACTION_STR_NONE "NONE"
#define UIDM_ACTION_STR_SHUTDOWN "HALT"
#define UIDM_ACTION_STR_REBOOT "REBOOT"
#define UIDM_ACTION_STR_SUSPEND "SUSPEND"

typedef struct {
	int fd;
	char* auth_cookie;

	UIdmLogoutAction available_actions;
	UIdmLogoutAction current_actions;

	time_t last_update;
} UIdmProtocolData;

static UIdmProtocolData uidm_protocol_data = {
	0,
	NULL,
	UIDM_LOGOUT_ACTION_NONE,
	UIDM_LOGOUT_ACTION_NONE,
	0
};

static char* uidm_send_protocol_msg(UIdmProtocolData* data, const char* msg)
{
	GString* retval;
	char buf[256];
	char* p;
	int len;

	p = g_strconcat(msg, "\n", NULL);

	if (write(data->fd, p, strlen(p)) < 0)
	{
		g_free(p);

		g_warning("Failed to send message to UIDM: %s", g_strerror(errno));

		return NULL;
	}

	g_free(p);

	p = NULL;
	retval = NULL;

	while ((len = read(data->fd, buf, sizeof(buf) - 1)) > 0)
	{
		buf[len] = '\0';

		if (!retval)
		{
			retval = g_string_new(buf);
		}
		else
		{
			retval = g_string_append(retval, buf);
		}

		if ((p = strchr(retval->str, '\n')))
		{
			break;
		}
	}

	if (p)
	{
		*p = '\0';
	}

	return retval ? g_string_free(retval, FALSE) : NULL;
}

static char* get_display_number(void)
{
	const char* display_name;
	char* retval;
	char* p;

	display_name = gdk_display_get_name(gdk_display_get_default());

	p = strchr(display_name, ':');

	if (!p)
	{
		return g_strdup("0");
	}

	while (*p == ':')
	{
		p++;
	}

	retval = g_strdup(p);

	p = strchr(retval, '.');

	if (p != NULL)
	{
		*p = '\0';
	}

	return retval;
}

static gboolean uidm_authenticate_connection(UIdmProtocolData* data)
{
	#define UIDM_MIT_MAGIC_COOKIE_LEN 16

	const char* xau_path;
	FILE* f;
	Xauth* xau;
	char* display_number;
	gboolean retval;

	if (data->auth_cookie)
	{
		char* msg;
		char* response;

		msg = g_strdup_printf(UIDM_PROTOCOL_MSG_AUTHENTICATE " %s", data->auth_cookie);
		response = uidm_send_protocol_msg(data, msg);
		g_free(msg);

		if (response && !strcmp(response, "OK"))
		{
			g_free(response);
			return TRUE;
		}
		else
		{
			g_free(response);
			g_free(data->auth_cookie);
			data->auth_cookie = NULL;
		}
	}

	if (!(xau_path = XauFileName()))
	{
		return FALSE;
	}

	if (!(f = fopen(xau_path, "r")))
	{
		return FALSE;
	}

	retval = FALSE;
	display_number = get_display_number();

	while ((xau = XauReadAuth(f)))
	{
		char buffer[40]; /* 2*16 == 32, so 40 is enough */
		char* msg;
		char* response;
		int   i;

		if (xau->family != FamilyLocal || strncmp(xau->number, display_number, xau->number_length) || strncmp(xau->name, "MIT-MAGIC-COOKIE-1", xau->name_length) || xau->data_length != UIDM_MIT_MAGIC_COOKIE_LEN)
		{
			XauDisposeAuth(xau);
			continue;
		}

		for (i = 0; i < UIDM_MIT_MAGIC_COOKIE_LEN; i++)
		{
			g_snprintf(buffer + 2 * i, 3, "%02x", (guint)(guchar) xau->data[i]);
		}

		XauDisposeAuth(xau);

		msg = g_strdup_printf(UIDM_PROTOCOL_MSG_AUTHENTICATE " %s", buffer);
		response = uidm_send_protocol_msg(data, msg);
		g_free(msg);

		if (response && !strcmp(response, "OK"))
		{
			data->auth_cookie = g_strdup(buffer);
			g_free(response);
			retval = TRUE;
			break;
		}

		g_free(response);
	}

	g_free(display_number);

	fclose(f);

	return retval;

	#undef UIDM_MIT_MAGIC_COOKIE_LEN
}

static void uidm_shutdown_protocol_connection(UIdmProtocolData *data)
{
	if (data->fd)
	{
		close(data->fd);
	}

	data->fd = 0;
}

static gboolean uidm_init_protocol_connection(UIdmProtocolData* data)
{
	struct sockaddr_un addr;
	char* response;

	g_assert(data->fd <= 0);

	if (g_file_test(UIDM_PROTOCOL_SOCKET_PATH, G_FILE_TEST_EXISTS))
	{
		strcpy(addr.sun_path, UIDM_PROTOCOL_SOCKET_PATH);
	}
	else if (g_file_test("/tmp/.uidm_socket", G_FILE_TEST_EXISTS))
	{
		strcpy(addr.sun_path, "/tmp/.uidm_socket");
	}
	else
	{
		return FALSE;
	}

	data->fd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (data->fd < 0)
	{
		g_warning("Failed to create UIDM socket: %s", g_strerror(errno));

		uidm_shutdown_protocol_connection(data);

		return FALSE;
	}

	addr.sun_family = AF_UNIX;

	if (connect(data->fd, (struct sockaddr*) &addr, sizeof(addr)) < 0)
	{
		g_warning("Failed to establish a connection with UIDM: %s", g_strerror(errno));

		uidm_shutdown_protocol_connection(data);

		return FALSE;
	}

	response = uidm_send_protocol_msg(data, UIDM_PROTOCOL_MSG_VERSION);

	if (!response || strncmp(response, "UIDM ", strlen("UIDM ")) != 0)
	{
		g_free(response);

		g_warning("Failed to get protocol version from UIDM");
		uidm_shutdown_protocol_connection(data);

		return FALSE;
	}

	g_free(response);

	if (!uidm_authenticate_connection(data))
	{
		g_warning("Failed to authenticate with UIDM");
		uidm_shutdown_protocol_connection(data);
		return FALSE;
	}

	return TRUE;
}

static void uidm_parse_query_response(UIdmProtocolData* data, const char* response)
{
	char** actions;
	int i;

	data->available_actions = UIDM_LOGOUT_ACTION_NONE;
	data->current_actions = UIDM_LOGOUT_ACTION_NONE;

	if (strncmp(response, "OK ", 3) != 0)
	{
		return;
	}

	response += 3;

	actions = g_strsplit(response, ";", -1);

	for (i = 0; actions[i]; i++)
	{
		UIdmLogoutAction action = UIDM_LOGOUT_ACTION_NONE;
		gboolean selected = FALSE;
		char* str = actions [i];
		int len;

		len = strlen(str);

		if (!len)
		{
			continue;
		}

		if (str[len - 1] == '!')
		{
			selected = TRUE;
			str[len - 1] = '\0';
		}

		if (!strcmp(str, UIDM_ACTION_STR_SHUTDOWN))
		{
				action = UIDM_LOGOUT_ACTION_SHUTDOWN;
		}
		else if (!strcmp(str, UIDM_ACTION_STR_REBOOT))
		{
				action = UIDM_LOGOUT_ACTION_REBOOT;
		}
		else if (!strcmp(str, UIDM_ACTION_STR_SUSPEND))
		{
				action = UIDM_LOGOUT_ACTION_SUSPEND;
		}

		data->available_actions |= action;

		if (selected)
		{
			data->current_actions |= action;
		}
	}

	g_strfreev(actions);
}

static void uidm_update_logout_actions(UIdmProtocolData* data)
{
	time_t current_time;
	char* response;

	current_time = time(NULL);

	if (current_time <= (data->last_update + UIDM_PROTOCOL_UPDATE_INTERVAL))
	{
		return;
	}

	data->last_update = current_time;

	if (!uidm_init_protocol_connection(data))
	{
		return;
	}

	if ((response = uidm_send_protocol_msg(data, UIDM_PROTOCOL_MSG_QUERY_ACTION)))
	{
		uidm_parse_query_response(data, response);
		g_free(response);
	}

	uidm_shutdown_protocol_connection(data);
}

gboolean uidm_is_available(void)
{
	if (!uidm_init_protocol_connection(&uidm_protocol_data))
	{
		return FALSE;
	}

	uidm_shutdown_protocol_connection(&uidm_protocol_data);

	return TRUE;
}

gboolean uidm_supports_logout_action(UIdmLogoutAction action)
{
	uidm_update_logout_actions(&uidm_protocol_data);

	return (uidm_protocol_data.available_actions & action) != 0;
}

UIdmLogoutAction uidm_get_logout_action(void)
{
	uidm_update_logout_actions(&uidm_protocol_data);

	return uidm_protocol_data.current_actions;
}

void uidm_set_logout_action(UIdmLogoutAction action)
{
	char* action_str = NULL;
	char* msg;
	char* response;

	if (!uidm_init_protocol_connection(&uidm_protocol_data))
	{
		return;
	}

	switch (action)
	{
		case UIDM_LOGOUT_ACTION_NONE:
			action_str = UIDM_ACTION_STR_NONE;
			break;
		case UIDM_LOGOUT_ACTION_SHUTDOWN:
			action_str = UIDM_ACTION_STR_SHUTDOWN;
			break;
		case UIDM_LOGOUT_ACTION_REBOOT:
			action_str = UIDM_ACTION_STR_REBOOT;
			break;
		case UIDM_LOGOUT_ACTION_SUSPEND:
			action_str = UIDM_ACTION_STR_SUSPEND;
			break;
	}

	msg = g_strdup_printf(UIDM_PROTOCOL_MSG_SET_ACTION " %s", action_str);

	response = uidm_send_protocol_msg(&uidm_protocol_data, msg);

	g_free(msg);
	g_free(response);

	uidm_protocol_data.last_update = 0;

	uidm_shutdown_protocol_connection(&uidm_protocol_data);
}

void uidm_new_login(void)
{
    char* response;

    if (!uidm_init_protocol_connection(&uidm_protocol_data))
    {
        return;
    }

    response = uidm_send_protocol_msg(&uidm_protocol_data, UIDM_PROTOCOL_MSG_FLEXI_XSERVER);

    g_free(response);

    uidm_protocol_data.last_update = 0;

    uidm_shutdown_protocol_connection(&uidm_protocol_data);
}
