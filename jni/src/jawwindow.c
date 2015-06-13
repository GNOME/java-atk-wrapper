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

#include <atk/atk.h>
#include <glib.h>
#include "jawobject.h"
#include "jawimpl.h"
#include "jawutil.h"
#include "jawtoplevel.h"

extern void jaw_window_interface_init (AtkWindowIface*);
extern gpointer jaw_window_data_init (jobject ac);
extern void jaw_window_data_finalize (gpointer p);

typedef struct _WindowData {
  jobject atk_window;
} WindowData;

enum {
  ACTIVATE,
  CREATE,
  DEACTIVATE,
  DESTROY,
  MAXIMIZE,
  MINIMIZE,
  MOVE,
  RESIZE,
  RESTORE,
  LAST_SIGNAL
};

static guint atk_window_signals[LAST_SIGNAL] = { 0 };

static guint
jaw_window_add_signal (const gchar *name)
{
  return g_signal_new (name,
                       JAW_TYPE_OBJECT,
                       G_SIGNAL_RUN_LAST,
                       0,
                       (GSignalAccumulator) NULL, NULL,
                       g_cclosure_marshal_VOID__VOID,
                       G_TYPE_NONE,
                       0);
}

typedef struct _JawWindowIface JawWindowIface;

void
jaw_window_interface_init (AtkWindowIface *iface)
{
 static gboolean initialized = FALSE;

  if (!initialized)
    {
      atk_window_signals[ACTIVATE]    = jaw_window_add_signal ("activate");
      atk_window_signals[CREATE]      = jaw_window_add_signal ("create");
      atk_window_signals[DEACTIVATE]  = jaw_window_add_signal ("deactivate");
      atk_window_signals[DESTROY]     = jaw_window_add_signal ("destroy");
      atk_window_signals[MAXIMIZE]    = jaw_window_add_signal ("maximize");
      atk_window_signals[MINIMIZE]    = jaw_window_add_signal ("minimize");
      atk_window_signals[MOVE]        = jaw_window_add_signal ("move");
      atk_window_signals[RESIZE]      = jaw_window_add_signal ("resize");
      atk_window_signals[RESTORE]     = jaw_window_add_signal ("restore");
      initialized = TRUE;
    }
}

gpointer
jaw_window_data_init (jobject ac)
{
  WindowData *data = g_new0(WindowData, 1);

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classWindow = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkWindow");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classWindow, "<init>", "(Ljavax/accessibility/AccessibleContext;)V");
  jobject jatk_window = (*jniEnv)->NewObject(jniEnv, classWindow, jmid, ac);
  data->atk_window = (*jniEnv)->NewGlobalRef(jniEnv, jatk_window);

  return data;
}

void
jaw_window_data_finalize (gpointer p)
{
  WindowData *data = (WindowData*)p;
  JNIEnv *jniEnv = jaw_util_get_jni_env();

  if (data && data->atk_window)
  {
    (*jniEnv)->DeleteGlobalRef(jniEnv, data->atk_window);
    data->atk_window = NULL;
  }
}

