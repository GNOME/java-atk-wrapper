/*
 * Java ATK Wrapper for GNOME
 * Copyright (C) 2009 Sun Microsystems Inc.
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

#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <atk-bridge.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <X11/Xlib.h>
#include "jawutil.h"
#include "jawimpl.h"
#include "jawtoplevel.h"
#include "AtkWrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

#define JNI_FALSE 0
#define JNI_TRUE 1

#define KEY_DISPATCH_NOT_DISPATCHED 0
#define KEY_DISPATCH_CONSUMED 1
#define KEY_DISPATCH_NOT_CONSUMED 2

#define GDK_SHIFT_MASK (1 << 0)
#define GDK_CONTROL_MASK (1 << 2)
#define GDK_MOD1_MASK (1 << 3)
#define GDK_META_MASK (1 << 28)

typedef enum _SignalType SignalType;
gboolean jaw_accessibility_init (void);
void jaw_accessibility_shutdown (void);

gboolean jaw_debug = FALSE;

static gint key_dispatch_result;
static GMainLoop* jni_main_loop;

static gboolean jaw_initialized = FALSE;

gboolean jaw_accessibility_init (void)
{
  atk_bridge_adaptor_init (NULL, NULL);
  if (jaw_debug)
    printf("Atk Bridge Initialized\n");
  return TRUE;
}

void
jaw_accessibility_shutdown (void)
{
  atk_bridge_adaptor_cleanup();
}

static gpointer jni_loop_callback(void *data)
{
  if (!g_main_loop_is_running((GMainLoop *)data))
    g_main_loop_run((GMainLoop *)data);
  else
  {
    if (jaw_debug)
      printf("Running JNI already\n");
  }
  return 0;
}

JNIEXPORT jboolean
JNICALL Java_org_GNOME_Accessibility_AtkWrapper_initNativeLibrary()
{
  const gchar* debug_env = g_getenv("JAW_DEBUG");
  if (g_strcmp0(debug_env, "1") == 0) {
    jaw_debug = TRUE;
  }

  if (jaw_initialized)
    return JNI_TRUE;
  // Java app with GTK Look And Feel will load gail
  // Set NO_GAIL to "1" to prevent gail from executing

  g_setenv("NO_GAIL", "1", TRUE);

  // Disable ATK Bridge temporarily to aoid the loading
  // of ATK Bridge by GTK look and feel
  g_setenv("NO_AT_BRIDGE", "1", TRUE);

  g_type_class_unref(g_type_class_ref(JAW_TYPE_UTIL));
  // Force to invoke base initialization function of each ATK interfaces
  g_type_class_unref(g_type_class_ref(ATK_TYPE_NO_OP_OBJECT));

  return JNI_TRUE;
}

JNIEXPORT void
JNICALL Java_org_GNOME_Accessibility_AtkWrapper_loadAtkBridge()
{
  // Enable ATK Bridge so we can load it now
  g_unsetenv ("NO_AT_BRIDGE");

  GThread *thread;
  GError *err;
  char * message;
  message = "JNI main loop";
  err = NULL;

  jaw_initialized = jaw_accessibility_init();
  if (jaw_debug)
    printf("Jaw Initialization STATUS in loadAtkBridge: %d\n", jaw_initialized);

  jni_main_loop = g_main_loop_new (NULL, FALSE); /*main loop NOT running*/
  thread = g_thread_new(message, jni_loop_callback, (void *) jni_main_loop);
  if(thread == NULL)
  {
    if (jaw_debug)
    {
      printf("Thread create failed: %s!!\n", err->message );
      g_error_free (err);
    }
  }
}

enum _SignalType {
  Sig_Text_Caret_Moved = 0,
  Sig_Text_Property_Changed_Insert = 1,
  Sig_Text_Property_Changed_Delete = 2,
  Sig_Text_Property_Changed_Replace = 3,
  Sig_Object_Children_Changed_Add = 4,
  Sig_Object_Children_Changed_Remove = 5,
  Sig_Object_Active_Descendant_Changed = 6,
  Sig_Object_Selection_Changed = 7,
  Sig_Object_Visible_Data_Changed = 8,
  Sig_Object_Property_Change_Accessible_Actions = 9,
  Sig_Object_Property_Change_Accessible_Value = 10,
  Sig_Object_Property_Change_Accessible_Description = 11,
  Sig_Object_Property_Change_Accessible_Name = 12,
  Sig_Object_Property_Change_Accessible_Hypertext_Offset = 13,
  Sig_Object_Property_Change_Accessible_Table_Caption = 14,
  Sig_Object_Property_Change_Accessible_Table_Summary = 15,
  Sig_Object_Property_Change_Accessible_Table_Column_Header = 16,
  Sig_Object_Property_Change_Accessible_Table_Column_Description = 17,
  Sig_Object_Property_Change_Accessible_Table_Row_Header = 18,
  Sig_Object_Property_Change_Accessible_Table_Row_Description = 19,
  Sig_Table_Model_Changed = 20,
  Sig_Text_Property_Changed = 21
};

typedef struct _CallbackPara {
  jobject global_ac;
  gboolean is_toplevel;
  SignalType signal_id;
  jobjectArray args;
  AtkStateType atk_state;
  gboolean state_value;
} CallbackPara;

static CallbackPara*
alloc_callback_para (jobject ac)
{
  if (ac == NULL)
    return NULL;
  CallbackPara *para = g_new(CallbackPara, 1);
  para->global_ac = ac;
  para->args = NULL;

  return para;
}

static void
free_callback_para (CallbackPara *para)
{
  JNIEnv *jniEnv = jaw_util_get_jni_env();
  if (jniEnv == NULL)
  {
    free_callback_para(para);
    return;
  }

  if (para->global_ac == NULL)
  {
    if (jaw_debug)
      g_warning("free_callback_para: para->global_ac == NULL");
    free_callback_para(para);
    return;
  }

  (*jniEnv)->DeleteGlobalRef(jniEnv, para->global_ac);

  if (para->args) {
    (*jniEnv)->DeleteGlobalRef(jniEnv, para->args);
  }

  g_free(para);
}

static gboolean
focus_notify_handler (gpointer p)
{
  CallbackPara *para = (CallbackPara*)p;
  jobject global_ac = para->global_ac;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  if (jniEnv == NULL)
  {
    if (jaw_debug)
      g_warning("\nfocus_notify_handler: env == NULL\n");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }
  if (global_ac == NULL)
  {
    if (jaw_debug)
      g_warning("\nfocus_notify_handler: global_ac == NULL\n");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }
  JawImpl* jaw_impl = jaw_impl_get_instance(jniEnv, global_ac);
  if (jaw_impl == NULL)
  {
    if (jaw_debug)
      g_warning("\nfocus_notify_handler: jaw_impl == NULL\n");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  AtkObject* atk_obj = ATK_OBJECT(jaw_impl);
  atk_object_notify_state_change(atk_obj,
                                 ATK_STATE_SHOWING,
                                 1);

  free_callback_para(para);

  return G_SOURCE_REMOVE;
}

JNIEXPORT void
JNICALL Java_org_GNOME_Accessibility_AtkWrapper_focusNotify(JNIEnv *jniEnv,
                                                            jclass jClass,
                                                            jobject jAccContext)
{
  jobject global_ac = (*jniEnv)->NewGlobalRef(jniEnv, jAccContext);
  CallbackPara *para = alloc_callback_para(global_ac);
  gdk_threads_add_idle(focus_notify_handler, para);
}

static gboolean
window_open_handler (gpointer p)
{
  CallbackPara *para = (CallbackPara*)p;
  jobject global_ac = para->global_ac;
  gboolean is_toplevel = para->is_toplevel;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  if (jniEnv == NULL)
  {
    if (jaw_debug)
      fprintf(stderr,"\n *** window_open_handler: jniEnv == NULL *** \n");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  if (global_ac == NULL)
  {
    if (jaw_debug)
      fprintf(stderr,"\n *** window_open_handler: global_ac == NULL *** \n");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  JawImpl* jaw_impl = jaw_impl_get_instance(jniEnv, global_ac);
  if (jaw_impl == NULL)
  {
    if (jaw_debug)
      g_warning("window_open_handler: jaw_impl == NULL");
  }
  AtkObject* atk_obj = ATK_OBJECT(jaw_impl);

  if (!g_strcmp0(atk_role_get_name(atk_object_get_role(atk_obj)),
                 "redundant object"))
  {
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  if (atk_object_get_role(atk_obj) == ATK_ROLE_TOOL_TIP)
  {
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  if (is_toplevel)
  {
    gint n = jaw_toplevel_add_window(JAW_TOPLEVEL(atk_get_root()), atk_obj);

    g_object_notify(G_OBJECT(atk_get_root()), "accessible-name");

    g_signal_emit_by_name(ATK_OBJECT(atk_get_root()),
                          "children-changed::add",
                          n,
                          atk_obj,
                          NULL);

    g_signal_emit_by_name(atk_obj, "create", 0);
  }

  free_callback_para(para);

  return G_SOURCE_REMOVE;
}

JNIEXPORT void
JNICALL Java_org_GNOME_Accessibility_AtkWrapper_windowOpen(JNIEnv *jniEnv,
                                                           jclass jClass,
                                                           jobject jAccContext,
                                                           jboolean jIsToplevel)
{

  jobject global_ac = (*jniEnv)->NewGlobalRef(jniEnv, jAccContext);
  CallbackPara *para = alloc_callback_para(global_ac);
  para->is_toplevel = (jIsToplevel == JNI_TRUE) ? TRUE : FALSE;
  gdk_threads_add_idle(window_open_handler, para);
}

static gboolean
window_close_handler (gpointer p)
{
  CallbackPara *para = (CallbackPara*)p;
  jobject global_ac = para->global_ac;
  gboolean is_toplevel = para->is_toplevel;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  if (jniEnv == NULL)
  {
    if (jaw_debug)
      g_warning("window_close_handler: env == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  if (global_ac == NULL)
  {
    if (jaw_debug)
      g_warning("window_close_handler: global_ac == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }
  JawImpl *jaw_impl = jaw_impl_find_instance(jniEnv, global_ac);
  if (jaw_impl == NULL)
  {
    if (jaw_debug)
      g_warning("window_close_handler: jaw_impl == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  AtkObject* atk_obj = ATK_OBJECT(jaw_impl);

  if (!g_strcmp0(atk_role_get_name(atk_object_get_role(atk_obj)), "redundant object"))
  {
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  if (atk_object_get_role(atk_obj) == ATK_ROLE_TOOL_TIP)
  {
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  if (is_toplevel) {
    gint n = jaw_toplevel_remove_window(JAW_TOPLEVEL(atk_get_root()), atk_obj);

    g_object_notify(G_OBJECT(atk_get_root()), "accessible-name");

    g_signal_emit_by_name(ATK_OBJECT(atk_get_root()),
                          "children-changed::remove",
                          n,
                          atk_obj,
                          NULL);

    g_signal_emit_by_name(atk_obj, "destroy", 0);
  }

  free_callback_para(para);

  return G_SOURCE_REMOVE;
}

JNIEXPORT void
JNICALL Java_org_GNOME_Accessibility_AtkWrapper_windowClose(JNIEnv *jniEnv,
                                                            jclass jClass,
                                                            jobject jAccContext,
                                                            jboolean jIsToplevel)
{
  jobject global_ac = (*jniEnv)->NewGlobalRef(jniEnv, jAccContext);
  CallbackPara *para = alloc_callback_para(global_ac);
  para->is_toplevel = (jIsToplevel == JNI_TRUE) ? TRUE : FALSE;
  gdk_threads_add_idle(window_close_handler, para);
}

static gboolean
window_minimize_handler (gpointer p)
{
  CallbackPara *para = (CallbackPara*)p;
  jobject global_ac = para->global_ac;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  if (jniEnv == NULL)
  {
    if (jaw_debug)
      g_warning("window_minimize_handler: env == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  if (global_ac == NULL)
  {
    if (jaw_debug)
      g_warning("window_minimize_handler: global_ac == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }
  JawImpl* jaw_impl = jaw_impl_find_instance(jniEnv, global_ac);
  if (jaw_impl == NULL)
  {
    if (jaw_debug)
      g_warning("window_minimize_handler: jaw_impl == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  AtkObject* atk_obj = ATK_OBJECT(jaw_impl);
  g_signal_emit_by_name(atk_obj, "minimize", 0);

  free_callback_para(para);

  return G_SOURCE_REMOVE;
}

JNIEXPORT void
JNICALL Java_org_GNOME_Accessibility_AtkWrapper_windowMinimize(JNIEnv *jniEnv,
                                                               jclass jClass,
                                                               jobject jAccContext)
{
  jobject global_ac = (*jniEnv)->NewGlobalRef(jniEnv, jAccContext);
  CallbackPara *para = alloc_callback_para(global_ac);
  gdk_threads_add_idle(window_minimize_handler, para);
}

static gboolean
window_maximize_handler (gpointer p)
{
  CallbackPara *para = (CallbackPara*)p;
  jobject global_ac = para->global_ac;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  if (jniEnv == NULL)
  {
    if (jaw_debug)
      g_warning("window_maximize_handler: env == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  if (global_ac == NULL)
  {
    if (jaw_debug)
      g_warning("window_maximize_handler: global_ac == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  JawImpl* jaw_impl = jaw_impl_find_instance(jniEnv, global_ac);
  if (jaw_impl == NULL)
  {
    if (jaw_debug)
      g_warning("window_maximize_handler: jaw_impl == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  AtkObject* atk_obj = ATK_OBJECT(jaw_impl);
  g_signal_emit_by_name(atk_obj, "maximize", 0);

  free_callback_para(para);

  return G_SOURCE_REMOVE;
}

JNIEXPORT void JNICALL Java_org_GNOME_Accessibility_AtkWrapper_windowMaximize(JNIEnv *jniEnv,
                                                                              jclass jClass,
                                                                              jobject jAccContext)
{
  jobject global_ac = (*jniEnv)->NewGlobalRef(jniEnv, jAccContext);
  CallbackPara *para = alloc_callback_para(global_ac );
  gdk_threads_add_idle(window_maximize_handler, para);
}

static gboolean
window_restore_handler (gpointer p)
{
  CallbackPara *para = (CallbackPara*)p;
  jobject global_ac = para->global_ac;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  if (jniEnv == NULL)
  {
    if (jaw_debug)
      g_warning("window_restore_handler: env == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  if (global_ac == NULL)
  {
    if (jaw_debug)
      g_warning("window_restore_handler: global_ac == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  JawImpl* jaw_impl = jaw_impl_find_instance(jniEnv, global_ac);
  if (jaw_impl == NULL)
  {
    if (jaw_debug)
      g_warning("window_restore_handler: jaw_impl == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  AtkObject* atk_obj = ATK_OBJECT(jaw_impl);
  g_signal_emit_by_name(atk_obj, "restore", 0);

  free_callback_para(para);

  return G_SOURCE_REMOVE;
}

JNIEXPORT void JNICALL Java_org_GNOME_Accessibility_AtkWrapper_windowRestore(JNIEnv *jniEnv,
                                                                             jclass jClass,
                                                                             jobject jAccContext)
{

  jobject global_ac = (*jniEnv)->NewGlobalRef(jniEnv, jAccContext);
  CallbackPara *para = alloc_callback_para(global_ac);
  gdk_threads_add_idle(window_restore_handler, para);
}

static gboolean
window_activate_handler (gpointer p)
{
  CallbackPara *para = (CallbackPara*)p;
  jobject global_ac = para->global_ac;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  if (jniEnv == NULL)
  {
    if (jaw_debug)
      g_warning("window_activate_handler: env == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  if (global_ac == NULL)
  {
    if (jaw_debug)
      g_warning("window_activate_handler: global_ac == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }
  JawImpl* jaw_impl = jaw_impl_get_instance(jniEnv, global_ac);
  if (jaw_impl == NULL)
  {
    if (jaw_debug)
      g_warning("window_activate_handler: jaw_impl == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  AtkObject* atk_obj = ATK_OBJECT(jaw_impl);
  g_signal_emit_by_name(atk_obj, "activate", 0);

  free_callback_para(para);

  return G_SOURCE_REMOVE;
}

JNIEXPORT void JNICALL Java_org_GNOME_Accessibility_AtkWrapper_windowActivate(JNIEnv *jniEnv,
                                                                              jclass jClass,
                                                                              jobject jAccContext) {

  jobject global_ac = (*jniEnv)->NewGlobalRef(jniEnv, jAccContext);
  CallbackPara *para = alloc_callback_para(global_ac);
  gdk_threads_add_idle(window_activate_handler, para);
}

static gboolean
window_deactivate_handler (gpointer p)
{
  CallbackPara *para = (CallbackPara*)p;
  jobject global_ac = para->global_ac;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  if (jniEnv == NULL)
  {
    if (jaw_debug)
      g_warning("window_deactivate_handler: env == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  if (global_ac == NULL)
  {
    if (jaw_debug)
      g_warning("window_deactivate_handler: global_ac == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }
  JawImpl* jaw_impl = jaw_impl_get_instance(jniEnv, global_ac);
  if (jaw_impl == NULL)
  {
    if (jaw_debug)
      g_warning("window_deactivate_handler: jaw_impl == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  AtkObject* atk_obj = ATK_OBJECT(jaw_impl);
  g_signal_emit_by_name(atk_obj, "deactivate", 0);

  free_callback_para(para);

  return G_SOURCE_REMOVE;
}

JNIEXPORT void
JNICALL Java_org_GNOME_Accessibility_AtkWrapper_windowDeactivate(JNIEnv *jniEnv,
                                                                 jclass jClass,
                                                                 jobject jAccContext)
{

  jobject global_ac = (*jniEnv)->NewGlobalRef(jniEnv, jAccContext);
  CallbackPara *para = alloc_callback_para(global_ac);
  gdk_threads_add_idle(window_deactivate_handler, para);
}

static gboolean
window_state_change_handler (gpointer p)
{
  CallbackPara *para = (CallbackPara*)p;
  jobject global_ac = para->global_ac;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  if (jniEnv == NULL)
  {
    if (jaw_debug)
      g_warning("window_state_change_handler: env == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  if (global_ac == NULL)
  {
    if (jaw_debug)
      g_warning("window_state_change_handler: global_ac == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  JawImpl* jaw_impl = jaw_impl_get_instance(jniEnv, global_ac);
  if (jaw_impl == NULL)
  {
    if (jaw_debug)
      g_warning("window_state_change_handler: jaw_impl == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  AtkObject* atk_obj = ATK_OBJECT(jaw_impl);
  g_signal_emit_by_name(atk_obj, "state-change", 0);

  free_callback_para(para);

  return G_SOURCE_REMOVE;
}

JNIEXPORT void
JNICALL Java_org_GNOME_Accessibility_AtkWrapper_windowStateChange(JNIEnv *jniEnv,
                                                                  jclass jClass,
                                                                  jobject jAccContext)
{

  jobject global_ac = (*jniEnv)->NewGlobalRef(jniEnv, jAccContext);
  CallbackPara *para = alloc_callback_para(global_ac);
  gdk_threads_add_idle(window_state_change_handler, para);
}

static gchar
get_char_value (JNIEnv *jniEnv, jobject o)
{
  jclass classByte = (*jniEnv)->FindClass(jniEnv, "java/lang/Byte");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classByte, "byteValue", "()B");
  return (gchar)(*jniEnv)->CallByteMethod(jniEnv, o, jmid);
}

static gdouble
get_double_value (JNIEnv *jniEnv, jobject o)
{
  jclass classDouble = (*jniEnv)->FindClass(jniEnv, "java/lang/Double");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classDouble, "doubleValue", "()D");
  return (gdouble)(*jniEnv)->CallDoubleMethod(jniEnv, o, jmid);
}

static gfloat
get_float_value (JNIEnv *jniEnv, jobject o)
{
  jclass classFloat = (*jniEnv)->FindClass(jniEnv, "java/lang/Float");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classFloat, "floatValue", "()F");
  return (gfloat)(*jniEnv)->CallFloatMethod(jniEnv, o, jmid);
}

static gint
get_int_value (JNIEnv *jniEnv, jobject o)
{
  jclass classInteger = (*jniEnv)->FindClass(jniEnv, "java/lang/Integer");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classInteger, "intValue", "()I");
  return (gint)(*jniEnv)->CallIntMethod(jniEnv, o, jmid);
}
static gint64
get_int64_value (JNIEnv *jniEnv, jobject o)
{
  jclass classLong = (*jniEnv)->FindClass(jniEnv, "java/lang/Long");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classLong, "longValue", "()J");
  return (gint64)(*jniEnv)->CallLongMethod(jniEnv, o, jmid);
}

static gboolean
signal_emit_handler (gpointer p)
{
  CallbackPara *para = (CallbackPara*)p;
  jobject global_ac = para->global_ac;
  JNIEnv *jniEnv = jaw_util_get_jni_env();
  if (jniEnv == NULL)
  {
    if (jaw_debug)
      g_warning("signal_emit_handler: jniEnv == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  jobjectArray args = para->args;

  if (global_ac == NULL)
  {
    if (jaw_debug)
      g_warning("signal_emit_handler: global_ac == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  JawImpl* jaw_impl = jaw_impl_find_instance(jniEnv, global_ac);

  if (jaw_impl == NULL)
  {
    if (jaw_debug)
      g_warning("signal_emit_handler: jaw_impl == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  AtkObject* atk_obj = ATK_OBJECT(jaw_impl);

  switch (para->signal_id)
  {
    case Sig_Text_Caret_Moved:
    {
      gint cursor_pos = get_int_value(jniEnv,
                                      (*jniEnv)->GetObjectArrayElement(jniEnv, args, 0));
      g_signal_emit_by_name(atk_obj, "text_caret_moved", cursor_pos);
      break;
    }
    case Sig_Text_Property_Changed_Insert:
    {
      gint insert_position = get_int_value(jniEnv,
                                           (*jniEnv)->GetObjectArrayElement(jniEnv, args, 0));
      gint insert_length = get_int_value(jniEnv,
                                         (*jniEnv)->GetObjectArrayElement(jniEnv, args, 1));
      g_signal_emit_by_name(atk_obj,
                            "text_changed::insert",
                            insert_position,
                            insert_length);
      break;
    }
    case Sig_Text_Property_Changed_Delete:
    {
      gint delete_position = get_int_value(jniEnv,
                                           (*jniEnv)->GetObjectArrayElement(jniEnv, args, 0));
      gint delete_length = get_int_value(jniEnv,
                                         (*jniEnv)->GetObjectArrayElement(jniEnv, args, 1));
      g_signal_emit_by_name(atk_obj,
                            "text_changed::delete",
                            delete_position,
                            delete_length);
      break;
    }
    case Sig_Object_Children_Changed_Add:
    {
      gint child_index = get_int_value(jniEnv,
                                       (*jniEnv)->GetObjectArrayElement(jniEnv, args, 0));
      jobject child_ac = (*jniEnv)->GetObjectArrayElement(jniEnv, args, 1);
      JawImpl *child_impl = jaw_impl_get_instance(jniEnv, child_ac);
      if (child_impl == NULL)
      {
        if (jaw_debug)
          g_warning("signal_emit_handler: child_impl == NULL");
        free_callback_para(para);
        return G_SOURCE_REMOVE;
      }
      if (!child_impl)
      {
        break;
      }

      g_signal_emit_by_name(atk_obj,
                            "children_changed::add",
                            child_index,
                            child_impl);
        break;
      }
      case Sig_Object_Children_Changed_Remove:
      {
        gint child_index = get_int_value(jniEnv,
                                         (*jniEnv)->GetObjectArrayElement(jniEnv, args, 0));
        jobject child_ac = (*jniEnv)->GetObjectArrayElement(jniEnv, args, 1);
        JawImpl *child_impl = jaw_impl_find_instance(jniEnv, child_ac);
        if (!child_impl)
        {
          break;
        }

        g_signal_emit_by_name(atk_obj,
                              "children_changed::remove",
                              child_index,
                              child_impl);
        if (G_OBJECT(atk_obj) != NULL)
          g_object_unref(G_OBJECT(atk_obj));
        break;
      }
      case Sig_Object_Active_Descendant_Changed:
      {
      jobject child_ac = (*jniEnv)->GetObjectArrayElement(jniEnv, args, 0);
      JawImpl *child_impl = jaw_impl_get_instance(jniEnv, child_ac);
      if (child_impl == NULL)
      {
        if (jaw_debug)
          g_warning("signal_emit_handler: child_impl == NULL");
        free_callback_para(para);
        return G_SOURCE_REMOVE;
      }
      if (!child_impl)
      {
        break;
      }

      g_signal_emit_by_name(atk_obj,
                            "active_descendant_changed",
                            child_impl);
      break;
    }
    case Sig_Object_Selection_Changed:
    {
      g_signal_emit_by_name(atk_obj,
                            "selection_changed");
      break;
    }
    case Sig_Object_Visible_Data_Changed:
    {
      g_signal_emit_by_name(atk_obj,
                            "visible_data_changed");
      break;
    }
    case Sig_Object_Property_Change_Accessible_Actions:
    {
      gint oldValue = get_int_value(jniEnv,
                                    (*jniEnv)->GetObjectArrayElement(jniEnv, args, 0));
      gint newValue = get_int_value(jniEnv,
                                    (*jniEnv)->GetObjectArrayElement(jniEnv, args, 1));
      AtkPropertyValues values = { NULL };

      // GValues must be initialized
      g_assert (!G_VALUE_HOLDS_INT (&values.old_value));
      g_value_init(&values.old_value, G_TYPE_INT);
      g_assert (G_VALUE_HOLDS_INT (&values.old_value));
      g_value_set_int(&values.old_value, oldValue);
      if (jaw_debug)
        printf ("%d\n", g_value_get_int(&values.old_value));

      g_assert (!G_VALUE_HOLDS_INT (&values.new_value));
      g_value_init(&values.new_value, G_TYPE_INT);
      g_assert (G_VALUE_HOLDS_INT (&values.new_value));
      g_value_set_int(&values.new_value, newValue);
      if (jaw_debug)
        printf ("%d\n", g_value_get_int(&values.new_value));

      values.property_name = "accessible-actions";

      g_signal_emit_by_name(atk_obj,
                            "property_change::accessible-actions",
                            &values);
      break;
    }
    case Sig_Object_Property_Change_Accessible_Value:
    {
      g_object_notify(G_OBJECT(atk_obj), "accessible-value");
      break;
    }
    case Sig_Object_Property_Change_Accessible_Description:
    {
      g_object_notify(G_OBJECT(atk_obj), "accessible-description");
      break;
    }
    case Sig_Object_Property_Change_Accessible_Name:
    {
      g_object_notify(G_OBJECT(atk_obj), "accessible-name");
      break;
     }
    case Sig_Object_Property_Change_Accessible_Hypertext_Offset:
    {
      g_signal_emit_by_name(atk_obj,
                            "property_change::accessible-hypertext-offset",
                            NULL);
      break;
    }
    case Sig_Object_Property_Change_Accessible_Table_Caption:
    {
      g_signal_emit_by_name(atk_obj,
                            "property_change::accessible-table-caption",
                            NULL);
      break;
    }
    case Sig_Object_Property_Change_Accessible_Table_Summary:
    {
      g_signal_emit_by_name(atk_obj,
                            "property_change::accessible-table-summary",
                            NULL);
      break;
    }
    case Sig_Object_Property_Change_Accessible_Table_Column_Header:
    {
      g_signal_emit_by_name(atk_obj,
                            "property_change::accessible-table-column-header",
                            NULL);
      break;
    }
    case Sig_Object_Property_Change_Accessible_Table_Column_Description:
    {
      g_signal_emit_by_name(atk_obj,
                            "property_change::accessible-table-column-description",
                            NULL);
      break;
    }
    case Sig_Object_Property_Change_Accessible_Table_Row_Header:
    {
      g_signal_emit_by_name(atk_obj,
      "property_change::accessible-table-row-header",
      NULL);
      break;
    }
    case Sig_Object_Property_Change_Accessible_Table_Row_Description:
    {
      g_signal_emit_by_name(atk_obj,
                            "property_change::accessible-table-row-description",
                            NULL);
      break;
    }
    case Sig_Table_Model_Changed:
    {
      g_signal_emit_by_name(atk_obj,
                            "model_changed");
      break;
    }
    case Sig_Text_Property_Changed:
    {
      JawObject * jaw_obj = JAW_OBJECT(atk_obj);

      gint newValue = get_int_value(jniEnv,
                                    (*jniEnv)->GetObjectArrayElement(jniEnv, args, 0));

      gint prevCount = GPOINTER_TO_INT(g_hash_table_lookup(jaw_obj->storedData,
                                                           "Previous_Count"));
      gint curCount = atk_text_get_character_count(ATK_TEXT(jaw_obj));

      g_hash_table_insert(jaw_obj->storedData,
                          "Previous_Count",
                          GINT_TO_POINTER(curCount));

      if (curCount > prevCount)
      {
        g_signal_emit_by_name(atk_obj,
                              "text_changed::insert",
                              newValue,
                              curCount - prevCount);
      }
      else if (curCount < prevCount)
      {
        g_signal_emit_by_name(atk_obj,
                              "text_changed::delete",
                              newValue,
                              prevCount - curCount);
      }
      break;
    }
    default:
      break;
  }
  free_callback_para(para);
  return G_SOURCE_REMOVE;
}

JNIEXPORT void
JNICALL Java_org_GNOME_Accessibility_AtkWrapper_emitSignal(JNIEnv *jniEnv,
                                                           jclass jClass,
                                                           jobject jAccContext,
                                                           jint id,
                                                           jobjectArray args)
{
  jobject global_ac = (*jniEnv)->NewGlobalRef(jniEnv, jAccContext);
  jobjectArray global_args = (jobjectArray)(*jniEnv)->NewGlobalRef(jniEnv, args);
  CallbackPara *para = alloc_callback_para(global_ac);
  para->signal_id = (gint)id;
  para->args = global_args;
  gdk_threads_add_idle(signal_emit_handler, para);
}

static gboolean
object_state_change_handler (gpointer p)
{
  CallbackPara *para = (CallbackPara*)p;
  jobject global_ac = para->global_ac;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  if (jniEnv == NULL)
  {
    if (jaw_debug)
      g_warning("object_state_change_handler: env == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  if (global_ac == NULL)
  {
    if (jaw_debug)
      g_warning("object_state_change_handler: global_ac");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  JawImpl* jaw_impl = jaw_impl_get_instance(jniEnv, global_ac);
  if (jaw_impl == NULL)
  {
    if (jaw_debug)
      g_warning("object_state_change_handler: jaw_impl == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  atk_object_notify_state_change(ATK_OBJECT(jaw_impl),
                                 para->atk_state,
                                 para->state_value);

  free_callback_para(para);
  return G_SOURCE_REMOVE;
}

JNIEXPORT void
JNICALL Java_org_GNOME_Accessibility_AtkWrapper_objectStateChange(JNIEnv *jniEnv,
                                                                  jclass jClass,
                                                                  jobject jAccContext,
                                                                  jobject state,
                                                                  jboolean value)
{
  jobject global_ac = (*jniEnv)->NewGlobalRef(jniEnv, jAccContext);
  CallbackPara *para = alloc_callback_para(global_ac);
  AtkStateType state_type = jaw_util_get_atk_state_type_from_java_state( jniEnv, state );
  para->atk_state = state_type;
  if (value == JNI_TRUE) {
    para->state_value = TRUE;
  } else {
    para->state_value = FALSE;
  }
  gdk_threads_add_idle(object_state_change_handler, para);
}

static gboolean
component_added_handler (gpointer p)
{
  JNIEnv *jniEnv;

  CallbackPara *para = (CallbackPara*)p;
  jobject global_ac = para->global_ac;

  jniEnv = jaw_util_get_jni_env();
  if (jniEnv == NULL)
  {
    if (jaw_debug)
      g_warning("component_added_handler: env == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  if (global_ac == NULL)
  {
    if (jaw_debug)
      g_warning("component_added_handler: global_ac == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  JawImpl* jaw_impl = jaw_impl_get_instance(jniEnv, global_ac);
  if (jaw_impl == NULL)
  {
    if (jaw_debug)
      g_warning("component_added_handler: jaw_impl == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }


  AtkObject* atk_obj = ATK_OBJECT(jaw_impl);
  if (atk_object_get_role(atk_obj) == ATK_ROLE_TOOL_TIP)
  {
    atk_object_notify_state_change(atk_obj,
                                   ATK_STATE_SHOWING,
                                   1);
  }

  free_callback_para(para);
  return G_SOURCE_REMOVE;
}

JNIEXPORT void
JNICALL Java_org_GNOME_Accessibility_AtkWrapper_componentAdded(JNIEnv *jniEnv,
                                                               jclass jClass,
                                                               jobject jAccContext)
{
  jobject global_ac = (*jniEnv)->NewGlobalRef(jniEnv, jAccContext);
  CallbackPara *para = alloc_callback_para(global_ac);
  gdk_threads_add_idle(component_added_handler, para);
}

static gboolean
component_removed_handler (gpointer p)
{
  JNIEnv *jniEnv;

  CallbackPara *para = (CallbackPara*)p;
  jobject global_ac = para->global_ac;
  jniEnv = jaw_util_get_jni_env();
  if (jniEnv == NULL)
  {
    if (jaw_debug)
      g_warning("component_removed_handler: env == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  if (global_ac == NULL)
  {
    if (jaw_debug)
      g_warning("component_removed_handler: global_ac == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  JawImpl* jaw_impl = jaw_impl_get_instance(jniEnv, global_ac);
  if (jaw_impl == NULL)
  {
    if (jaw_debug)
      g_warning("component_removed_handler: jaw_impl == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  AtkObject* atk_obj = ATK_OBJECT(jaw_impl);

  if (atk_obj == NULL)
  {
    if (jaw_debug)
      g_warning("component_removed_handler: atk_obj == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }
  if (atk_object_get_role(atk_obj) == ATK_ROLE_TOOL_TIP)
    atk_object_notify_state_change(atk_obj, ATK_STATE_SHOWING, FALSE);
  free_callback_para(para);

  return G_SOURCE_REMOVE;
}

JNIEXPORT void
JNICALL Java_org_GNOME_Accessibility_AtkWrapper_componentRemoved(JNIEnv *jniEnv,
                                                                 jclass jClass,
                                                                 jobject jAccContext)
{
  jobject global_ac = (*jniEnv)->NewGlobalRef(jniEnv, jAccContext);
  CallbackPara *para = alloc_callback_para(global_ac);
  gdk_threads_add_idle(component_removed_handler, para);
}

/**
 * Signal is emitted when the position or size of the component changes.
 */
static gboolean
bounds_changed_handler (gpointer p)
{
  JNIEnv *jniEnv;

  CallbackPara *para = (CallbackPara*)p;
  jobject global_ac = para->global_ac;
  jniEnv = jaw_util_get_jni_env();
  if (jniEnv == NULL)
  {
    if (jaw_debug)
      g_warning("bounds_changed_handler: env == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  if (global_ac == NULL)
  {
    if (jaw_debug)
      g_warning("bounds_changed_handler: global_ac == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  JawImpl* jaw_impl = jaw_impl_get_instance(jniEnv, global_ac);
  if (jaw_impl == NULL)
  {
    if (jaw_debug)
      g_warning("bounds_changed_handler: jaw_impl == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }

  AtkObject* atk_obj = ATK_OBJECT(jaw_impl);

  if (atk_obj == NULL)
  {
    if (jaw_debug)
      g_warning("bounds_changed_handler: atk_obj == NULL");
    free_callback_para(para);
    return G_SOURCE_REMOVE;
  }
  g_signal_emit_by_name(atk_obj, "bounds_changed");
  free_callback_para(para);

  return G_SOURCE_REMOVE;
}

JNIEXPORT void
JNICALL Java_org_GNOME_Accessibility_AtkWrapper_boundsChanged(JNIEnv *jniEnv,
                                                              jclass jClass,
                                                              jobject jAccContext)
{
  jobject global_ac = (*jniEnv)->NewGlobalRef(jniEnv, jAccContext);
  CallbackPara *para = alloc_callback_para(global_ac);
  gdk_threads_add_idle(bounds_changed_handler, para);
}

static gboolean
key_dispatch_handler (gpointer p)
{
  key_dispatch_result = 0;
  jobject jAtkKeyEvent = (jobject)p;
  AtkKeyEventStruct *event = g_new0(AtkKeyEventStruct, 1);

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  if (jniEnv == NULL)
  {
    if (jaw_debug)
      g_warning("key_dispatch_handler: env == NULL");
    return G_SOURCE_REMOVE;
  }

  jclass classAtkKeyEvent = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkKeyEvent");

  // type
  jfieldID jfidType = (*jniEnv)->GetFieldID(jniEnv, classAtkKeyEvent, "type", "I");
  jint type = (*jniEnv)->GetIntField(jniEnv, jAtkKeyEvent, jfidType);

  jfieldID jfidTypePressed = (*jniEnv)->GetStaticFieldID(jniEnv,
                                                         classAtkKeyEvent,
                                                         "ATK_KEY_EVENT_PRESSED",
                                                         "I");
  jfieldID jfidTypeReleased = (*jniEnv)->GetStaticFieldID(jniEnv,
                                                          classAtkKeyEvent,
                                                          "ATK_KEY_EVENT_RELEASED",
                                                          "I");

  jint type_pressed = (*jniEnv)->GetStaticIntField(jniEnv,
                                                   classAtkKeyEvent,
                                                   jfidTypePressed);
  jint type_released = (*jniEnv)->GetStaticIntField(jniEnv,
                                                    classAtkKeyEvent,
                                                    jfidTypeReleased);

  if (type == type_pressed)
  {
    event->type = ATK_KEY_EVENT_PRESS;
  } else if (type == type_released)
  {
    event->type = ATK_KEY_EVENT_RELEASE;
  } else {
    g_assert_not_reached();
  }

  // state
  jfieldID jfidShift = (*jniEnv)->GetFieldID(jniEnv, classAtkKeyEvent, "isShiftKeyDown", "Z");
  jboolean jShiftKeyDown = (*jniEnv)->GetBooleanField(jniEnv, jAtkKeyEvent, jfidShift);
  if (jShiftKeyDown == JNI_TRUE) {
    event->state |= GDK_SHIFT_MASK;
  }

  jfieldID jfidCtrl = (*jniEnv)->GetFieldID(jniEnv, classAtkKeyEvent, "isCtrlKeyDown", "Z");
  jboolean jCtrlKeyDown = (*jniEnv)->GetBooleanField(jniEnv, jAtkKeyEvent, jfidCtrl);
  if (jCtrlKeyDown == JNI_TRUE) {
    event->state |= GDK_CONTROL_MASK;
  }

  jfieldID jfidAlt = (*jniEnv)->GetFieldID(jniEnv, classAtkKeyEvent, "isAltKeyDown", "Z");
  jboolean jAltKeyDown = (*jniEnv)->GetBooleanField(jniEnv, jAtkKeyEvent, jfidAlt);
  if (jAltKeyDown == JNI_TRUE) {
    event->state |= GDK_MOD1_MASK;
  }

  jfieldID jfidMeta = (*jniEnv)->GetFieldID(jniEnv, classAtkKeyEvent, "isMetaKeyDown", "Z");
  jboolean jMetaKeyDown = (*jniEnv)->GetBooleanField(jniEnv, jAtkKeyEvent, jfidMeta);
  if (jMetaKeyDown == JNI_TRUE)
  {
    event->state |= GDK_META_MASK;
  }

  // keyval
  jfieldID jfidKeyval = (*jniEnv)->GetFieldID(jniEnv, classAtkKeyEvent, "keyval", "I");
  jint jkeyval = (*jniEnv)->GetIntField(jniEnv, jAtkKeyEvent, jfidKeyval);
  event->keyval = (guint)jkeyval;

  // string
  jfieldID jfidString = (*jniEnv)->GetFieldID(jniEnv, classAtkKeyEvent, "string", "Ljava/lang/String;");
  jstring jstr = (jstring)(*jniEnv)->GetObjectField(jniEnv, jAtkKeyEvent, jfidString);
  event->length = (gint)(*jniEnv)->GetStringLength(jniEnv, jstr);
  event->string = (gchar*)(*jniEnv)->GetStringUTFChars(jniEnv, jstr, 0);

  // keycode
  jfieldID jfidKeycode = (*jniEnv)->GetFieldID(jniEnv, classAtkKeyEvent, "keycode", "I");
  event->keycode = (gint)(*jniEnv)->GetIntField(jniEnv, jAtkKeyEvent, jfidKeycode);

  // timestamp
  jfieldID jfidTimestamp = (*jniEnv)->GetFieldID(jniEnv, classAtkKeyEvent, "timestamp", "I");
  event->timestamp = (guint32)(*jniEnv)->GetIntField(jniEnv, jAtkKeyEvent, jfidTimestamp);

  gboolean b = jaw_util_dispatch_key_event (event);
  if(jaw_debug)
    printf("key_dispatch_result b = %d\n ", b);
  if (b) {
    key_dispatch_result = KEY_DISPATCH_CONSUMED;
  } else {
    key_dispatch_result = KEY_DISPATCH_NOT_CONSUMED;
  }

  (*jniEnv)->ReleaseStringUTFChars(jniEnv, jstr, event->string);
  g_free(event);

  (*jniEnv)->DeleteGlobalRef(jniEnv, jAtkKeyEvent);
  return G_SOURCE_REMOVE;
}

JNIEXPORT jboolean
JNICALL Java_org_GNOME_Accessibility_AtkWrapper_dispatchKeyEvent(JNIEnv *jniEnv,
                                                                 jclass jClass,
                                                                 jobject jAtkKeyEvent)
{
  jboolean key_consumed;
  jobject global_key_event = (*jniEnv)->NewGlobalRef(jniEnv, jAtkKeyEvent);
  gdk_threads_add_idle(key_dispatch_handler, (gpointer)global_key_event);

  if(jaw_debug)
    printf("key_dispatch_result saved = %d\n ", key_dispatch_result);
  if (key_dispatch_result == KEY_DISPATCH_CONSUMED)
  {
    key_consumed = JNI_TRUE;
  } else
  {
    key_consumed = JNI_FALSE;
  }

  key_dispatch_result = KEY_DISPATCH_NOT_DISPATCHED;

  return key_consumed;
}

#ifdef __cplusplus
}
#endif

