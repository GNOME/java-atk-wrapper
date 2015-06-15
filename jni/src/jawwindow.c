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

void
jaw_window_interface_init (AtkWindowIface *iface)
{
  // Signals
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

