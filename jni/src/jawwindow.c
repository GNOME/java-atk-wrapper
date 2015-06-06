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
#include "jawwindow.h"
#include "jawutil.h"
#include "jawtoplevel.h"

static void jaw_window_class_init(JawWindowClass *klass);
static void jaw_window_init(JawWindow *object);
static void jaw_window_dispose(GObject *gobject);
static void jaw_window_finalize(GObject *gobject);

static gpointer parent_class = NULL;

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
  TOTAL_SIGNAL
};

static guint     jaw_window_signals[TOTAL_SIGNAL] = { 0, };

G_DEFINE_TYPE(JawWindow, jaw_window, JAW_TYPE_OBJECT)

static void
jaw_window_class_init (JawWindowClass *klass)
{

  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->dispose = jaw_window_dispose;
  gobject_class->finalize = jaw_window_finalize;
  parent_class = g_type_class_peek_parent (klass);

  klass->get_interface_data = NULL;

  jaw_window_signals [ACTIVATE] = g_signal_new ("activate",
                                                G_TYPE_FROM_CLASS (klass),
                                                G_SIGNAL_RUN_LAST,
                                                0, /* default signal handler */
                                                NULL,
                                                NULL,
                                                g_cclosure_marshal_VOID__VOID,
                                                G_TYPE_NONE,
                                                0);
  jaw_window_signals [CREATE] = g_signal_new ("create",
                                              G_TYPE_FROM_CLASS (klass),
                                              G_SIGNAL_RUN_LAST,
                                              0, /* default signal handler */
                                              NULL, NULL,
                                              g_cclosure_marshal_VOID__VOID,
                                              G_TYPE_NONE,
                                              0);
  jaw_window_signals [DEACTIVATE] = g_signal_new ("deactivate",
                                                  G_TYPE_FROM_CLASS (klass),
                                                  G_SIGNAL_RUN_LAST,
                                                  0, /* default signal handler */
                                                  NULL, NULL,
                                                  g_cclosure_marshal_VOID__VOID,
                                                  G_TYPE_NONE, 0);
  jaw_window_signals [DESTROY] = g_signal_new ("destroy",
                                                G_TYPE_FROM_CLASS (klass),
                                                G_SIGNAL_RUN_LAST,
                                                0, /* default signal handler */
                                                NULL,
                                                NULL,
                                                g_cclosure_marshal_VOID__VOID,
                                                G_TYPE_NONE,
                                                0);
  jaw_window_signals [MAXIMIZE] = g_signal_new ("maximize",
                                                G_TYPE_FROM_CLASS (klass),
                                                G_SIGNAL_RUN_LAST,
                                                0, /* default signal handler */
                                                NULL, NULL,
                                                g_cclosure_marshal_VOID__VOID,
                                                G_TYPE_NONE, 0);
  jaw_window_signals [MINIMIZE] = g_signal_new ("minimize",
                                                G_TYPE_FROM_CLASS (klass),
                                                G_SIGNAL_RUN_LAST,
                                                0, /* default signal handler */
                                                NULL, NULL,
                                                g_cclosure_marshal_VOID__VOID,
                                                G_TYPE_NONE, 0);
  jaw_window_signals [MOVE] = g_signal_new ("move",
                                            G_TYPE_FROM_CLASS (klass),
                                            G_SIGNAL_RUN_LAST,
                                            0, /* default signal handler */
                                            NULL, NULL,
                                            g_cclosure_marshal_VOID__VOID,
                                            G_TYPE_NONE, 0);
  jaw_window_signals [RESIZE] = g_signal_new ("resize",
                                              G_TYPE_FROM_CLASS (klass),
                                              G_SIGNAL_RUN_LAST,
                                              0, /* default signal handler */
                                              NULL,
                                              NULL,
                                              g_cclosure_marshal_VOID__VOID,
                                              G_TYPE_NONE,
                                              0);
  jaw_window_signals [RESTORE] = g_signal_new ("restore",
                                                G_TYPE_FROM_CLASS (klass),
                                                G_SIGNAL_RUN_LAST,
                                                0, /* default signal handler */
                                                NULL,
                                                NULL,
                                                g_cclosure_marshal_VOID__VOID,
                                                G_TYPE_NONE,
                                                0);
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
jaw_window_dispose (GObject *gobject)
{
  /* Chain up to parent's dispose method */
  G_OBJECT_CLASS(parent_class)->dispose(gobject);
}

static void
jaw_window_finalize (GObject *gobject)
{
  /* Customized finalize code */
  JawObject *jaw_obj = JAW_OBJECT(gobject);
  AtkObject *atk_obj = ATK_OBJECT(gobject);
  JNIEnv *jniEnv = jaw_util_get_jni_env();

  if (atk_obj->name != NULL)
  {
    (*jniEnv)->ReleaseStringUTFChars(jniEnv, jaw_obj->jstrName, atk_obj->name);
    (*jniEnv)->DeleteGlobalRef(jniEnv, jaw_obj->jstrName);
    jaw_obj->jstrName = NULL;
    atk_obj->name = NULL;
  }

  if (atk_obj->description != NULL)
  {
    (*jniEnv)->ReleaseStringUTFChars(jniEnv,
                                     jaw_obj->jstrDescription,
                                     atk_obj->description);

    (*jniEnv)->DeleteGlobalRef(jniEnv, jaw_obj->jstrDescription);
    jaw_obj->jstrDescription = NULL;
    atk_obj->description = NULL;
  }

  if (G_OBJECT(jaw_obj->state_set) != NULL)
  {
    g_object_unref(G_OBJECT(jaw_obj->state_set));
    /* Chain up to parent's finalize method */
    G_OBJECT_CLASS(parent_class)->finalize(gobject);
  }
}

