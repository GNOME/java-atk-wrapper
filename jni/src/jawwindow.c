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
#include "jawwindow.h"

extern void jaw_window_interface_init (AtkWindowIface*);
extern gpointer jaw_window_data_init (jobject ac);
extern void jaw_window_data_finalize (gpointer p);

static void jaw_window_class_init(JawWindowClass *klass);
static void jaw_window_class_dispose(GObject *gobject);
static void jaw_window_class_finalize(GObject *gobject);

G_DEFINE_TYPE_WITH_CODE (JawWindow,
                         jaw_window,
                         JAW_TYPE_WINDOW,
                         G_IMPLEMENT_INTERFACE (ATK_TYPE_WINDOW,
                                                jaw_window_interface_init));

static gpointer parent_class = NULL;

typedef struct _WindowData {
  jobject atk_window;
} WindowData;

void
jaw_window_interface_init (AtkWindowIface *iface)
{
  // Signals
}

static void
jaw_window_initialize (AtkObject *atk_obj, gpointer data)
{
  JawObject *jaw_obj = JAW_OBJECT(atk_obj);
  WindowData *windata = jaw_object_get_interface_data(jaw_obj, INTERFACE_WINDOW);
  jobject atk_widget = windata->atk_window;
  atk_obj->role = jaw_util_get_atk_role_from_jobj (atk_widget);
}

static void jaw_window_class_init(JawWindowClass *klass)
{
  AtkUtilClass *atk_class;
  gpointer data;
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = jaw_window_class_finalize;
  gobject_class->dispose = jaw_window_class_dispose;

  parent_class = g_type_class_peek_parent (klass);
  JawObjectClass *jaw_class = JAW_OBJECT_CLASS (klass);
  klass->get_interface_data = NULL;
}

gpointer
jaw_window_get_interface_data (JawWindow *jaw_win, guint iface)
{
  JawWindowClass *klass = JAW_WINDOW_GET_CLASS(jaw_win);
  if (klass->get_interface_data)
    return klass->get_interface_data(jaw_win, iface);

  return NULL;
}

static void
jaw_window_init (JawWindow *window)
{
  window->state_set = atk_state_set_new();
}

static void
jaw_window_class_dispose (GObject *gobject)
{
  G_OBJECT_CLASS(parent_class)->dispose(gobject);
}

static void
jaw_window_class_finalize (GObject *gobject)
{
  JawWindow *jaw_obj = JAW_WINDOW(gobject);
  AtkObject *atk_obj = ATK_OBJECT(gobject);
  JNIEnv *jniEnv = jaw_util_get_jni_env();

  if (G_OBJECT(jaw_obj->state_set) != NULL)
  {
    g_object_unref(G_OBJECT(jaw_obj->state_set));
    G_OBJECT_CLASS(parent_class)->finalize(gobject);
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

