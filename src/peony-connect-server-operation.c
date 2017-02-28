/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
 * Peony
 *
 * Copyright (C) 2010 Cosimo Cecchi <cosimoc@gnome.org>
 *
 * Peony is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * Peony is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; see the file COPYING.  If not,
 * write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 * Author: Cosimo Cecchi <cosimoc@gnome.org>
 */

#include <config.h>

#include "peony-connect-server-operation.h"

#include "peony-connect-server-dialog.h"

G_DEFINE_TYPE (PeonyConnectServerOperation,
	       peony_connect_server_operation, GTK_TYPE_MOUNT_OPERATION);

enum {
	PROP_DIALOG = 1,
	NUM_PROPERTIES
};

struct _PeonyConnectServerOperationDetails {
	PeonyConnectServerDialog *dialog;
};

static void
fill_details_async_cb (GObject *source,
		       GAsyncResult *result,
		       gpointer user_data)
{
	PeonyConnectServerDialog *dialog;
	PeonyConnectServerOperation *self;
	gboolean res;

	self = user_data;
	dialog = PEONY_CONNECT_SERVER_DIALOG (source);

	res = peony_connect_server_dialog_fill_details_finish (dialog, result);

	if (!res) {
		g_mount_operation_reply (G_MOUNT_OPERATION (self), G_MOUNT_OPERATION_ABORTED);
	} else {
		g_mount_operation_reply (G_MOUNT_OPERATION (self), G_MOUNT_OPERATION_HANDLED);
	}
}

static void
peony_connect_server_operation_ask_password (GMountOperation *op,
						const gchar *message,
						const gchar *default_user,
						const gchar *default_domain,
						GAskPasswordFlags flags)
{
	PeonyConnectServerOperation *self;

	self = PEONY_CONNECT_SERVER_OPERATION (op);

	peony_connect_server_dialog_fill_details_async (self->details->dialog,
							   G_MOUNT_OPERATION (self),
							   default_user,
							   default_domain,
							   flags,
							   fill_details_async_cb,
							   self);
}

static void
peony_connect_server_operation_set_property (GObject *object,
						guint property_id,
						const GValue *value,
						GParamSpec *pspec)
{
	PeonyConnectServerOperation *self;

	self = PEONY_CONNECT_SERVER_OPERATION (object);

	switch (property_id) {
	case PROP_DIALOG:
		self->details->dialog = g_value_get_object (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}

static void
peony_connect_server_operation_class_init (PeonyConnectServerOperationClass *klass)
{
	GMountOperationClass *mount_op_class;
	GObjectClass *object_class;
	GParamSpec *pspec;

	object_class = G_OBJECT_CLASS (klass);
	object_class->set_property = peony_connect_server_operation_set_property;

	mount_op_class = G_MOUNT_OPERATION_CLASS (klass);
	mount_op_class->ask_password = peony_connect_server_operation_ask_password;

	pspec = g_param_spec_object ("dialog", "The connect dialog",
				     "The connect to server dialog",
				     PEONY_TYPE_CONNECT_SERVER_DIALOG,
				     G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS);
	g_object_class_install_property (object_class, PROP_DIALOG, pspec);

	g_type_class_add_private (klass, sizeof (PeonyConnectServerOperationDetails));
}

static void
peony_connect_server_operation_init (PeonyConnectServerOperation *self)
{
	self->details = G_TYPE_INSTANCE_GET_PRIVATE (self,
						     PEONY_TYPE_CONNECT_SERVER_OPERATION,
						     PeonyConnectServerOperationDetails);
}

GMountOperation *
peony_connect_server_operation_new (PeonyConnectServerDialog *dialog)
{
	return g_object_new (PEONY_TYPE_CONNECT_SERVER_OPERATION,
			     "dialog", dialog,
			     NULL);
}
