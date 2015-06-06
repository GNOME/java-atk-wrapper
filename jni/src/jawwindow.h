/*
 * Java ATK Wrapper for GNOME
 *
 * Copyright (C) 2015 Magdalen Berns <m.berns@thismagpie.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
 
#ifndef _JAW_WINDOW_H_
#define _JAW_WINDOW_H_

#include <atk/atk.h>
#include <jni.h>

G_BEGIN_DECLS

#define JAW_TYPE_WINDOW (jaw_window_get_type ())
#define JAW_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), JAW_TYPE_WINDOW, JawWindow))
#define JAW_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), JAW_TYPE_WINDOW, JawWindowClass))
#define JAW_IS_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JAW_TYPE_WINDOW))
#define JAW_IS_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), JAW_TYPE_WINDOW))
#define JAW_WINDOW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), JAW_TYPE_WINDOW, JawWindowClass))

typedef struct _JawWindow JawWindow;
typedef struct _JawWindowClass JawWindowClass;

struct _JawWindow
{
  AtkObject parent;
  jobject acc_context;
  AtkStateSet *state_set;
};

GType jaw_window_get_type (void);

struct _JawWindowClass
{
  AtkObjectClass parent_class;
  gpointer (*get_interface_data) (JawWindow*, guint);
};

gpointer jaw_window_get_interface_data (JawWindow*, guint);

G_END_DECLS

#endif
