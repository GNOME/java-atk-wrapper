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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <glib/gprintf.h>
#include "jawutil.h"
#include "jawtoplevel.h"
#include "jawobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/* AtkUtil */
static void jaw_util_class_init(JawUtilClass *klass);

static guint jaw_util_add_key_event_listener(AtkKeySnoopFunc listener,
                                             gpointer data);
static void jaw_util_remove_key_event_listener(guint remove_listener);
static AtkObject* jaw_util_get_root(void);
static const gchar* jaw_util_get_toolkit_name(void);
static const gchar* jaw_util_get_toolkit_version(void);

static GHashTable *key_listener_list = NULL;

JavaVM *cachedJVM;

GType
jaw_util_get_type(void)
{
  static GType type = 0;

  if (!type) {
    static const GTypeInfo tinfo = {
      sizeof(JawUtilClass),
      (GBaseInitFunc) NULL, /*base init*/
      (GBaseFinalizeFunc) NULL, /*base finalize */
      (GClassInitFunc) jaw_util_class_init, /* class init */
      (GClassFinalizeFunc) NULL, /*class finalize */
      NULL, /* class data */
      sizeof(JawUtil), /* instance size */
      0, /* nb preallocs */
      (GInstanceInitFunc) NULL, /* instance init */
      NULL /* value table */
    };

    type = g_type_register_static(ATK_TYPE_UTIL, "JawUtil", &tinfo, 0);
  }

  return type;
}

static void
jaw_util_class_init(JawUtilClass *kclass)
{
  AtkUtilClass *atk_class;
  gpointer data;

  data = g_type_class_peek (ATK_TYPE_UTIL);
  atk_class = ATK_UTIL_CLASS (data);

  atk_class->add_key_event_listener = jaw_util_add_key_event_listener;
  atk_class->remove_key_event_listener = jaw_util_remove_key_event_listener;
  atk_class->get_root = jaw_util_get_root;
  atk_class->get_toolkit_name = jaw_util_get_toolkit_name;
  atk_class->get_toolkit_version = jaw_util_get_toolkit_version;
}

typedef struct _JawKeyListenerInfo{
  AtkKeySnoopFunc listener;
  gpointer data;
}JawKeyListenerInfo;

static gboolean
notify_hf (gpointer key, gpointer value, gpointer data)
{
  JawKeyListenerInfo *info = (JawKeyListenerInfo*)value;
  AtkKeyEventStruct *key_event = (AtkKeyEventStruct*)data;

  AtkKeySnoopFunc func = info->listener;
  gpointer func_data = info->data;

  return (*func)(key_event, func_data) ? TRUE : FALSE;
}

static void
insert_hf (gpointer key, gpointer value, gpointer data)
{
  GHashTable *new_table = (GHashTable *) data;
  g_hash_table_insert (new_table, key, value);
}

gboolean
jaw_util_dispatch_key_event (AtkKeyEventStruct *event)
{
  gint consumed = 0;
  if (key_listener_list) {
    GHashTable *new_hash = g_hash_table_new(NULL, NULL);
    g_hash_table_foreach(key_listener_list, insert_hf, new_hash);
    consumed = g_hash_table_foreach_steal(new_hash, notify_hf, event);
    g_hash_table_destroy(new_hash);
  }

  return (consumed > 0) ? TRUE : FALSE;
}

static guint
jaw_util_add_key_event_listener (AtkKeySnoopFunc listener, gpointer data)
{
  static guint key = 0;

  if (!listener) {
    return 0;
  }

  if (!key_listener_list) {
    key_listener_list = g_hash_table_new(NULL, NULL);
  }

  JawKeyListenerInfo *info = g_new0(JawKeyListenerInfo, 1);
  info->listener = listener;
  info->data = data;

  key++;
  g_hash_table_insert(key_listener_list, GUINT_TO_POINTER(key), info);

  return key;
}

static void
jaw_util_remove_key_event_listener (guint remove_listener)
{
  gpointer *value = g_hash_table_lookup(key_listener_list,
                                        GUINT_TO_POINTER(remove_listener));
  if (value)
    g_free(value);

  g_hash_table_remove(key_listener_list, GUINT_TO_POINTER(remove_listener));
}

static AtkObject*
jaw_util_get_root (void)
{
  static JawToplevel *root = NULL;

  if (!root) {
    root = g_object_new(JAW_TYPE_TOPLEVEL, NULL);
    atk_object_initialize(ATK_OBJECT(root), NULL);
  }

  return ATK_OBJECT(root);
}

static const gchar*
jaw_util_get_toolkit_name (void)
{
  return "J2SE-access-bridge";
}

static const gchar*
jaw_util_get_toolkit_version (void)
{
  return "1.0";
}

/* static functions */
guint
jaw_util_get_tflag_from_jobj(JNIEnv *jniEnv, jobject jObj)
{
  guint tflag = 0;
  jmethodID jmid;
  jclass classAccessibleContext = (*jniEnv)->FindClass(jniEnv,
                                                       "javax/accessibility/AccessibleContext");
  jclass classAccessible = (*jniEnv)->FindClass(jniEnv,
                                                "javax/accessibility/Accessible");
  jobject ac;
  jobject iface;

  if((*jniEnv)->IsInstanceOf(jniEnv, jObj, classAccessibleContext) )
  {
    ac = jObj;
  } else if((*jniEnv)->IsInstanceOf(jniEnv, jObj, classAccessible))
  {
    jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                            classAccessible,
                                            "getAccessibleContext",
                                            "()Ljavax/accessibility/AccessibleContext;");
    ac = (*jniEnv)->CallObjectMethod(jniEnv, jObj, jmid);
  } else {
    return 0;
  }

  jmid = (*jniEnv)->GetMethodID(jniEnv,
                                classAccessibleContext,
                                "getAccessibleAction",
                                "()Ljavax/accessibility/AccessibleAction;");
  iface = (*jniEnv)->CallObjectMethod(jniEnv, ac, jmid);
  if (iface != NULL)
  {
    tflag |= INTERFACE_ACTION;
  }

  jmid = (*jniEnv)->GetMethodID(jniEnv,
                                classAccessibleContext,
                                "getAccessibleComponent",
                                "()Ljavax/accessibility/AccessibleComponent;");
  iface = (*jniEnv)->CallObjectMethod(jniEnv, ac, jmid);
  if (iface != NULL)
  {
    tflag |= INTERFACE_COMPONENT;
  }

  jmid = (*jniEnv)->GetMethodID(jniEnv,
                                classAccessibleContext,
                                "getAccessibleText",
                                "()Ljavax/accessibility/AccessibleText;");
  iface = (*jniEnv)->CallObjectMethod(jniEnv, ac, jmid);
  if (iface != NULL)
  {
    tflag |= INTERFACE_TEXT;

    jclass classAccessibleHypertext = (*jniEnv)->FindClass(jniEnv, "javax/accessibility/AccessibleHypertext");
    if ( (*jniEnv)->IsInstanceOf(jniEnv, iface, classAccessibleHypertext))
    {
      tflag |= INTERFACE_HYPERTEXT;
    }

    jmid = (*jniEnv)->GetMethodID(jniEnv,
                                  classAccessibleContext,
                                  "getAccessibleEditableText",
                                  "()Ljavax/accessibility/AccessibleEditableText;");
    iface = (*jniEnv)->CallObjectMethod(jniEnv, ac, jmid);
    if (iface != NULL)
    {
      tflag |= INTERFACE_EDITABLE_TEXT;
    }
  }

  jmid = (*jniEnv)->GetMethodID(jniEnv,
                                classAccessibleContext,
                                "getAccessibleIcon",
                                "()[Ljavax/accessibility/AccessibleIcon;");
  iface = (*jniEnv)->CallObjectMethod(jniEnv, ac, jmid);
  if (iface != NULL)
  {
    tflag |= INTERFACE_IMAGE;
  }

  jmid = (*jniEnv)->GetMethodID(jniEnv,
                                classAccessibleContext,
                                "getAccessibleSelection",
                                "()Ljavax/accessibility/AccessibleSelection;");
  iface = (*jniEnv)->CallObjectMethod(jniEnv, ac, jmid);
  if (iface != NULL)
  {
    tflag |= INTERFACE_SELECTION;
  }

  jmid = (*jniEnv)->GetMethodID(jniEnv,
                                classAccessibleContext,
                                "getAccessibleTable",
                                "()Ljavax/accessibility/AccessibleTable;");
  iface = (*jniEnv)->CallObjectMethod(jniEnv, ac, jmid);
  if (iface != NULL)
  {
    tflag |= INTERFACE_TABLE;
    jclass classAccessibleExtendedTable = (*jniEnv)->FindClass(jniEnv, "javax/accessibility/AccessibleExtendedTable");
    if ((*jniEnv)->IsInstanceOf(jniEnv, iface, classAccessibleExtendedTable))
    {
      tflag |= INTERFACE_TABLE_CELL;
    }
  }

  jmid = (*jniEnv)->GetMethodID(jniEnv,
                                classAccessibleContext,
                                "getAccessibleValue",
                                "()Ljavax/accessibility/AccessibleValue;");
  iface = (*jniEnv)->CallObjectMethod(jniEnv, ac, jmid);
  if (iface != NULL)
  {
    tflag |= INTERFACE_VALUE;
  }

  return tflag;
}

gboolean
jaw_util_is_same_jobject(gconstpointer a, gconstpointer b)
{
  JNIEnv *jniEnv = jaw_util_get_jni_env();
  if ( (*jniEnv)->IsSameObject(jniEnv, (jobject)a, (jobject)b) ) {
    return TRUE;
  } else {
    return FALSE;
  }
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserve)
{
  if (jvm == NULL)
  {
    g_error("JavaVM pointer was NULL when initializing library");
    return JNI_ERR;
  }
  cachedJVM = jvm;
  return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *jvm, void *reserve) {
}

JNIEnv*
jaw_util_get_jni_env(void)
{
  JNIEnv *env;
  env  = NULL;
  static int i;

  i = 0;
  void* ptr;
  ptr = NULL;
  JavaVMAttachArgs args = { 0, };
  jint res;

  #ifdef JNI_VERSION_1_6
  res = (*cachedJVM)->GetEnv(cachedJVM, &ptr, JNI_VERSION_1_6);
  #endif
  env = (JNIEnv*) ptr;

  if (env != NULL)
    return env;

    switch (res)
    {
      case JNI_EDETACHED:
        args.version = JNI_VERSION_1_6;
        args.name = g_strdup_printf("NativeThread %d", i++);
        res = (*cachedJVM)->AttachCurrentThread(cachedJVM, &ptr, NULL);
        env = (JNIEnv*) ptr;
        if ((res == JNI_OK) && (env != NULL))
        {
          g_free(args.name);
          return env;
        }
        g_printerr("\n *** Attach failed. *** JNIEnv thread is detached.\n");
        break;
      case JNI_EVERSION:
        g_printerr(" *** Version error *** \n");
        break;
    }
    fflush(stderr);
    exit(2);
  return NULL;
}

void
jaw_util_detach(void)
{
  JavaVM* jvm;
  jvm = cachedJVM;
  (*jvm)->DetachCurrentThread(jvm);
}

static jobject
jaw_util_get_java_acc_role (JNIEnv *jniEnv, const gchar* roleName)
{
  jclass classAccessibleRole = (*jniEnv)->FindClass(jniEnv,
                                                    "javax/accessibility/AccessibleRole");
  jfieldID jfid = (*jniEnv)->GetStaticFieldID(jniEnv,
                                              classAccessibleRole,
                                              roleName,
                                              "Ljavax/accessibility/AccessibleRole;");
  jobject jrole = (*jniEnv)->GetStaticObjectField(jniEnv, classAccessibleRole, jfid);

  return jrole;
}

static gboolean
jaw_util_is_java_acc_role (JNIEnv *jniEnv, jobject acc_role, const gchar* roleName)
{
  jobject jrole = jaw_util_get_java_acc_role (jniEnv, roleName);

  if ((*jniEnv)->IsSameObject(jniEnv, acc_role, jrole))
  {
    return TRUE;
  } else {
    return FALSE;
  }
}

AtkRole
jaw_util_get_atk_role_from_jobj (jobject jobj)
{
  jobject ac;
  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classAccessibleContext = (*jniEnv)->FindClass(jniEnv,
                                                       "javax/accessibility/AccessibleContext");
  jclass classAccessible = (*jniEnv)->FindClass(jniEnv,
                                                "javax/accessibility/Accessible");
  jmethodID jmidGetContext;
  jmidGetContext = (*jniEnv)->GetMethodID(jniEnv,
                                          classAccessible,
                                          "getAccessibleContext",
                                          "()Ljavax/accessibility/AccessibleContext;");
  if( (*jniEnv)->IsInstanceOf(jniEnv, jobj, classAccessibleContext))
  {
    ac = jobj;
  } else if ((*jniEnv)->IsInstanceOf(jniEnv, jobj, classAccessible))
  {
    ac = (*jniEnv)->CallObjectMethod(jniEnv, jobj, jmidGetContext);
  } else {
    return ATK_ROLE_INVALID;
  }

  jmethodID jmidGetRole = (*jniEnv)->GetMethodID(jniEnv,
                                                 classAccessibleContext,
                                                 "getAccessibleRole",
                                                 "()Ljavax/accessibility/AccessibleRole;");
  jobject ac_role = (*jniEnv)->CallObjectMethod(jniEnv, ac, jmidGetRole);

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "ALERT"))
    return ATK_ROLE_ALERT;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "AWT_COMPONENT"))
    return ATK_ROLE_UNKNOWN;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "CANVAS"))
    return ATK_ROLE_CANVAS;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "CHECK_BOX"))
    return ATK_ROLE_CHECK_BOX;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "COLOR_CHOOSER"))
    return ATK_ROLE_COLOR_CHOOSER;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "COLUMN_HEADER"))
    return ATK_ROLE_COLUMN_HEADER;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "COMBO_BOX"))
    return ATK_ROLE_COMBO_BOX;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "DATE_EDITOR"))
    return ATK_ROLE_DATE_EDITOR;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "DESKTOP_ICON"))
    return ATK_ROLE_DESKTOP_ICON;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "DESKTOP_PANE"))
    return ATK_ROLE_DESKTOP_FRAME;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "DIALOG"))
    return ATK_ROLE_DIALOG;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "DIRECTORY_PANE"))
    return ATK_ROLE_DIRECTORY_PANE;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "EDITBAR"))
    return ATK_ROLE_EDITBAR;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "FILE_CHOOSER"))
    return ATK_ROLE_FILE_CHOOSER;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "FILLER"))
    return ATK_ROLE_FILLER;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "FONT_CHOOSER"))
    return ATK_ROLE_FONT_CHOOSER;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "FOOTER"))
    return ATK_ROLE_FOOTER;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "FRAME"))
    return ATK_ROLE_FRAME;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "GLASS_PANE"))
    return ATK_ROLE_GLASS_PANE;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "GROUP_BOX"))
    return ATK_ROLE_PANEL;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "HEADER"))
    return ATK_ROLE_HEADER;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "HTML_CONTAINER"))
    return ATK_ROLE_HTML_CONTAINER;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "HYPERLINK"))
    return ATK_ROLE_UNKNOWN;

  if ( jaw_util_is_java_acc_role(jniEnv, ac_role, "ICON"))
    return ATK_ROLE_ICON;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "INTERNAL_FRAME"))
    return ATK_ROLE_INTERNAL_FRAME;


  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "LABEL"))
    return ATK_ROLE_LABEL;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "LAYERED_PANE"))
    return ATK_ROLE_LAYERED_PANE;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "LIST"))
    return ATK_ROLE_LIST;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "LIST_ITEM"))
    return ATK_ROLE_LIST_ITEM;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "MENU"))
    return ATK_ROLE_MENU;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "MENU_BAR"))
    return ATK_ROLE_MENU_BAR;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "MENU_ITEM"))
    return ATK_ROLE_MENU_ITEM;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "OPTION_PANE"))
    return ATK_ROLE_OPTION_PANE;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "PAGE_TAB"))
    return ATK_ROLE_PAGE_TAB;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "PAGE_TAB_LIST"))
    return ATK_ROLE_PAGE_TAB_LIST;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "PANEL"))
    return ATK_ROLE_PANEL;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "PARAGRAPH"))
    return ATK_ROLE_PARAGRAPH;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "PASSWORD_TEXT"))
    return ATK_ROLE_PASSWORD_TEXT;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "POPUP_MENU"))
    return ATK_ROLE_POPUP_MENU;

  if  (jaw_util_is_java_acc_role(jniEnv, ac_role, "PROGRESS_BAR"))
    return ATK_ROLE_PROGRESS_BAR;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "PUSH_BUTTON"))
    return ATK_ROLE_PUSH_BUTTON;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "RADIO_BUTTON"))
  {
    jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                            classAccessibleContext,
                                            "getAccessibleParent",
                                            "()Ljavax/accessibility/Accessible;");

    jobject parent_obj = (*jniEnv)->CallObjectMethod(jniEnv, ac, jmid);
    if (!parent_obj)
      return ATK_ROLE_RADIO_BUTTON;

    jobject parent_ac = (*jniEnv)->CallObjectMethod(jniEnv, parent_obj, jmidGetContext);
    jobject parent_role = (*jniEnv)->CallObjectMethod(jniEnv, parent_ac, jmidGetRole);

    if (jaw_util_is_java_acc_role(jniEnv, parent_role, "MENU"))
      return ATK_ROLE_RADIO_MENU_ITEM;

    return ATK_ROLE_RADIO_BUTTON;
  }

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "ROOT_PANE"))
    return ATK_ROLE_ROOT_PANE;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "ROW_HEADER"))
    return ATK_ROLE_ROW_HEADER;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "RULER"))
    return ATK_ROLE_RULER;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "SCROLL_BAR"))
    return ATK_ROLE_SCROLL_BAR;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "SCROLL_PANE"))
    return ATK_ROLE_SCROLL_PANE;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "SEPARATOR"))
    return ATK_ROLE_SEPARATOR;

  if ( jaw_util_is_java_acc_role(jniEnv, ac_role, "SLIDER"))
    return ATK_ROLE_SLIDER;

  if ( jaw_util_is_java_acc_role(jniEnv, ac_role, "SPIN_BOX"))
    return ATK_ROLE_SPIN_BUTTON;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "SPLIT_PANE"))
    return ATK_ROLE_SPLIT_PANE;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "STATUS_BAR"))
    return ATK_ROLE_STATUSBAR;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "SWING_COMPONENT"))
    return ATK_ROLE_UNKNOWN;

  if ( jaw_util_is_java_acc_role(jniEnv, ac_role, "TABLE"))
    return ATK_ROLE_TABLE;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "TEXT"))
    return ATK_ROLE_TEXT;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "TOGGLE_BUTTON"))
    return ATK_ROLE_TOGGLE_BUTTON;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "TOOL_BAR"))
    return ATK_ROLE_TOOL_BAR;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "TOOL_TIP"))
    return ATK_ROLE_TOOL_TIP;

  if (jaw_util_is_java_acc_role(jniEnv, ac_role, "TREE"))
    return ATK_ROLE_TREE;

  if ( jaw_util_is_java_acc_role(jniEnv, ac_role, "UNKNOWN"))
  {
    jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                            classAccessibleContext,
                                            "getAccessibleParent",
                                            "()Ljavax/accessibility/Accessible;");
    jobject parent_obj = (*jniEnv)->CallObjectMethod(jniEnv, ac, jmid);

    if (parent_obj == NULL)
      return ATK_ROLE_APPLICATION;

    return ATK_ROLE_UNKNOWN;
  }

  if ( jaw_util_is_java_acc_role(jniEnv, ac_role, "VIEWPORT"))
    return ATK_ROLE_VIEWPORT;

  if ( jaw_util_is_java_acc_role(jniEnv, ac_role, "WINDOW"))
    return ATK_ROLE_WINDOW;

  jclass classAccessibleRole = (*jniEnv)->FindClass(jniEnv,
                                                    "javax/accessibility/AccessibleRole");
  jmethodID jmidToDisplayString = (*jniEnv)->GetMethodID(jniEnv,
                                                         classAccessibleRole,
                                                         "toDisplayString",
                                                         "(Ljava/util/Locale;)Ljava/lang/String;");

  jclass classLocale = (*jniEnv)->FindClass(jniEnv, "java/util/Locale");
  jfieldID jfidUS = (*jniEnv)->GetStaticFieldID(jniEnv,
                                                classLocale,
                                                "US",
                                                "Ljava/util/Locale;");
  jobject jobjUS = (*jniEnv)->GetStaticObjectField(jniEnv, classLocale, jfidUS);
  jobject jobjString = (*jniEnv)->CallObjectMethod(jniEnv,
                                                   ac_role,
                                                   jmidToDisplayString,
                                                   jobjUS);

  jclass classString = (*jniEnv)->FindClass(jniEnv, "java/lang/String");
  jmethodID jmidEqualsIgnoreCase = (*jniEnv)->GetMethodID(jniEnv,
                                                          classString,
                                                          "equalsIgnoreCase",
                                                          "(Ljava/lang/String;)Z");

  jstring jstr = (*jniEnv)->NewStringUTF(jniEnv, "paragraph");
  if ((*jniEnv)->CallBooleanMethod(jniEnv, jobjString, jmidEqualsIgnoreCase, jstr))
    return ATK_ROLE_PARAGRAPH;

  return ATK_ROLE_UNKNOWN; /* ROLE_EXTENDED */
}

static gboolean
is_same_java_state (JNIEnv *jniEnv, jobject jobj, const gchar* strState)
{
  jclass classAccessibleState = (*jniEnv)->FindClass(jniEnv,
                                                     "javax/accessibility/AccessibleState");
  jfieldID jfid = (*jniEnv)->GetStaticFieldID(jniEnv,
                                              classAccessibleState,
                                              strState,
                                              "Ljavax/accessibility/AccessibleState;");
  jobject jstate = (*jniEnv)->GetStaticObjectField(jniEnv, classAccessibleState, jfid);

  if ((*jniEnv)->IsSameObject( jniEnv, jobj, jstate )) {
    return TRUE;
  }

  return FALSE;
}

AtkStateType
jaw_util_get_atk_state_type_from_java_state (JNIEnv *jniEnv, jobject jobj)
{
  if (is_same_java_state( jniEnv, jobj, "ACTIVE" ))
    return ATK_STATE_ACTIVE;

  if (is_same_java_state( jniEnv, jobj, "ARMED" ))
    return ATK_STATE_ARMED;

  if (is_same_java_state( jniEnv, jobj, "BUSY" ))
    return ATK_STATE_BUSY;

  if (is_same_java_state( jniEnv, jobj, "CHECKED" ))
    return ATK_STATE_CHECKED;

  if (is_same_java_state( jniEnv, jobj, "COLLAPSED" ))
    return ATK_STATE_INVALID;

  if (is_same_java_state( jniEnv, jobj, "EDITABLE" ))
    return ATK_STATE_EDITABLE;

  if (is_same_java_state( jniEnv, jobj, "ENABLED" ))
    return ATK_STATE_ENABLED;

  if (is_same_java_state( jniEnv, jobj, "EXPANDABLE" ))
    return ATK_STATE_EXPANDABLE;

  if (is_same_java_state( jniEnv, jobj, "EXPANDED" ))
    return ATK_STATE_EXPANDED;

  if (is_same_java_state( jniEnv, jobj, "FOCUSABLE" ))
    return ATK_STATE_FOCUSABLE;

  if (is_same_java_state( jniEnv, jobj, "FOCUSED" ))
    return ATK_STATE_FOCUSED;

  if (is_same_java_state( jniEnv, jobj, "HORIZONTAL" ))
    return ATK_STATE_HORIZONTAL;

  if (is_same_java_state( jniEnv, jobj, "ICONIFIED" ))
    return ATK_STATE_ICONIFIED;

  if (is_same_java_state( jniEnv, jobj, "INDETERMINATE" ))
    return ATK_STATE_INDETERMINATE;

  if (is_same_java_state( jniEnv, jobj, "MANAGES_DESCENDANTS" ))
    return ATK_STATE_MANAGES_DESCENDANTS;

  if (is_same_java_state( jniEnv, jobj, "MODAL" ))
    return ATK_STATE_MODAL;

  if (is_same_java_state( jniEnv, jobj, "MULTI_LINE" ))
    return ATK_STATE_MULTI_LINE;

  if (is_same_java_state( jniEnv, jobj, "MULTISELECTABLE" ))
    return ATK_STATE_MULTISELECTABLE;

  if (is_same_java_state( jniEnv, jobj, "OPAQUE" ))
    return ATK_STATE_OPAQUE;

  if (is_same_java_state( jniEnv, jobj, "PRESSED" ))
    return ATK_STATE_PRESSED;

  if (is_same_java_state( jniEnv, jobj, "RESIZABLE" ))
    return ATK_STATE_RESIZABLE;

  if (is_same_java_state( jniEnv, jobj, "SELECTABLE" ))
    return ATK_STATE_SELECTABLE;

  if (is_same_java_state( jniEnv, jobj, "SELECTED" ))
    return ATK_STATE_SELECTED;

  if (is_same_java_state( jniEnv, jobj, "SHOWING" ))
    return ATK_STATE_SHOWING;

  if (is_same_java_state( jniEnv, jobj, "SINGLE_LINE" ))
    return ATK_STATE_SINGLE_LINE;

  if (is_same_java_state( jniEnv, jobj, "TRANSIENT" ))
    return ATK_STATE_TRANSIENT;

  if (is_same_java_state( jniEnv, jobj, "TRUNCATED" ))
    return ATK_STATE_TRUNCATED;

  if (is_same_java_state( jniEnv, jobj, "VERTICAL" ))
    return ATK_STATE_VERTICAL;

  if (is_same_java_state( jniEnv, jobj, "VISIBLE" ))
    return ATK_STATE_VISIBLE;

  return ATK_STATE_INVALID;
}

void
jaw_util_get_rect_info (JNIEnv *jniEnv,
                        jobject jrect,
                        gint *x,
                        gint *y,
                        gint *width,
                        gint *height)
{
  jclass classRectangle = (*jniEnv)->FindClass(jniEnv, "java/awt/Rectangle");
  jfieldID jfidX = (*jniEnv)->GetFieldID(jniEnv, classRectangle, "x", "I");
  jfieldID jfidY = (*jniEnv)->GetFieldID(jniEnv, classRectangle, "y", "I");
  jfieldID jfidWidth = (*jniEnv)->GetFieldID(jniEnv, classRectangle, "width", "I");
  jfieldID jfidHeight = (*jniEnv)->GetFieldID(jniEnv, classRectangle, "height", "I");

  (*x) = (gint)(*jniEnv)->GetIntField(jniEnv, jrect, jfidX);
  (*y) = (gint)(*jniEnv)->GetIntField(jniEnv, jrect, jfidY);
  (*width) = (gint)(*jniEnv)->GetIntField(jniEnv, jrect, jfidWidth);
  (*height) = (gint)(*jniEnv)->GetIntField(jniEnv, jrect, jfidHeight);
}

#ifdef __cplusplus
}
#endif
