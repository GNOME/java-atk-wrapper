/*
 * Java ATK Wrapper for GNOME
 * Copyright (C) 2009 Sun Microsystems Inc.
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
#include "jawimpl.h"
#include "jawutil.h"

static gboolean jaw_action_do_action(AtkAction *action, gint i);
static gint jaw_action_get_n_actions(AtkAction *action);
static const gchar* jaw_action_get_description(AtkAction *action, gint i);
static const gchar* jaw_action_get_name(AtkAction	*action, gint i);
static const gchar* jaw_action_get_keybinding (AtkAction *action, gint i);
static gboolean jaw_action_set_description (AtkAction *action, gint i, const gchar *description);
static const gchar* jaw_action_get_localized_name(AtkAction *action, gint i);

typedef struct _ActionData {
  jobject atk_action;
  jstring jstrLocalizedName;
  gchar* action_description;
  jstring jstrActionDescription;
  gchar* action_keybinding;
  jstring jstrActionKeybinding;
} ActionData;

void
jaw_action_interface_init (AtkActionIface *iface, gpointer data)
{
  iface->do_action = jaw_action_do_action;
  iface->get_n_actions = jaw_action_get_n_actions;
  iface->get_description = jaw_action_get_description;
  iface->get_name = jaw_action_get_name;
  iface->get_keybinding = jaw_action_get_keybinding;
  iface->set_description = jaw_action_set_description;
  iface->get_localized_name = jaw_action_get_localized_name;
}

gpointer
jaw_action_data_init (jobject ac)
{
    JAW_DEBUG("%s(%p)", __func__, ac);
  ActionData *data = g_new0(ActionData, 1);

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classAction = (*jniEnv)->FindClass(jniEnv,
                                            "org/GNOME/Accessibility/AtkAction");
  jmethodID jmid = (*jniEnv)->GetStaticMethodID(jniEnv,
                                          classAction,
                                          "createAtkAction",
                                          "(Ljavax/accessibility/AccessibleContext;)Lorg/GNOME/Accessibility/AtkAction;");
  jobject jatk_action = (*jniEnv)->CallStaticObjectMethod(jniEnv, classAction, jmid, ac);
  data->atk_action = (*jniEnv)->NewWeakGlobalRef(jniEnv, jatk_action);

  return data;
}

void
jaw_action_data_finalize (gpointer p)
{
    JAW_DEBUG("%s(%p)", __func__, p);
  ActionData *data = (ActionData*)p;
  JNIEnv *jniEnv = jaw_util_get_jni_env();

  if (data && data->atk_action) {
    if (data->action_name != NULL) {
      (*jniEnv)->ReleaseStringUTFChars(jniEnv,
                                       data->jstrActionName,
                                       data->action_name);
      (*jniEnv)->ReleaseStringUTFChars(jniEnv,
                                       data->jstrLocalizedName,
                                       data->action_name);

      (*jniEnv)->DeleteGlobalRef(jniEnv, data->jstrActionName);
      (*jniEnv)->DeleteGlobalRef(jniEnv, data->jstrLocalizedName);
      data->jstrActionName = NULL;
      data->jstrLocalizedName = NULL;
      data->action_name = NULL;
    }

    if (data->action_description != NULL) {
      (*jniEnv)->ReleaseStringUTFChars(jniEnv,
                                       data->jstrActionDescription,
                                       data->action_description);
      (*jniEnv)->DeleteGlobalRef(jniEnv, data->jstrActionDescription);
      data->jstrActionDescription = NULL;
      data->action_description = NULL;
    }

    if (data->action_keybinding != NULL) {
      (*jniEnv)->ReleaseStringUTFChars(jniEnv,
                                       data->jstrActionKeybinding,
                                       data->action_keybinding);
      (*jniEnv)->DeleteGlobalRef(jniEnv, data->jstrActionKeybinding);
      data->jstrActionKeybinding = NULL;
      data->action_keybinding = NULL;
    }

    (*jniEnv)->DeleteWeakGlobalRef(jniEnv, data->atk_action);
    data->atk_action = NULL;
  }
}

static gboolean
jaw_action_do_action (AtkAction *action, gint i)
{
    JAW_DEBUG("%s(%p, %d)", __func__, action, i);
  JawObject *jaw_obj = JAW_OBJECT(action);
  ActionData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_ACTION);
  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jobject atk_action = (*jniEnv)->NewGlobalRef(jniEnv, data->atk_action);
  if (!atk_action) {
    return FALSE;
  }

  jclass classAtkAction = (*jniEnv)->FindClass(jniEnv,
                                               "org/GNOME/Accessibility/AtkAction");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkAction,
                                          "do_action",
                                          "(I)Z");
  jboolean jresult = (*jniEnv)->CallBooleanMethod(jniEnv,
                                                  atk_action,
                                                  jmid,
                                                  (jint)i);
  (*jniEnv)->DeleteGlobalRef(jniEnv, atk_action);

  if (jresult == JNI_TRUE)
    return TRUE;

  return FALSE;
}

static gint
jaw_action_get_n_actions (AtkAction *action)
{
    JAW_DEBUG("%s(%p)", __func__, action);
  JawObject *jaw_obj = JAW_OBJECT(action);
  ActionData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_ACTION);
  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jobject atk_action = (*jniEnv)->NewGlobalRef(jniEnv, data->atk_action);
  if (!atk_action) {
    return 0;
  }

  jclass classAtkAction = (*jniEnv)->FindClass(jniEnv,
                                               "org/GNOME/Accessibility/AtkAction");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkAction,
                                          "get_n_actions", "()I");

  gint ret = (gint)(*jniEnv)->CallIntMethod(jniEnv, atk_action, jmid);
  (*jniEnv)->DeleteGlobalRef(jniEnv, atk_action);
  return ret;
}

static const gchar*
jaw_action_get_description (AtkAction *action, gint i)
{
    JAW_DEBUG("%s(%p, %d)", __func__, action, i);
  JawObject *jaw_obj = JAW_OBJECT(action);
  ActionData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_ACTION);
  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jobject atk_action = (*jniEnv)->NewGlobalRef(jniEnv, data->atk_action);
  if (!atk_action) {
    return NULL;
  }

  jclass classAtkAction = (*jniEnv)->FindClass(jniEnv,
                                               "org/GNOME/Accessibility/AtkAction");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkAction,
                                          "get_description",
                                          "(I)Ljava/lang/String;");
  jstring jstr = (*jniEnv)->CallObjectMethod(jniEnv,
                                             atk_action,
                                             jmid,
                                             (jint)i);
  (*jniEnv)->DeleteGlobalRef(jniEnv, atk_action);

  if (data->action_description != NULL)
  {
    (*jniEnv)->ReleaseStringUTFChars(jniEnv,
                                     data->jstrActionDescription,
                                     data->action_description);
    (*jniEnv)->DeleteGlobalRef(jniEnv, data->jstrActionDescription);
  }

  data->jstrActionDescription = (*jniEnv)->NewGlobalRef(jniEnv, jstr);
  data->action_description = (gchar*)(*jniEnv)->GetStringUTFChars(jniEnv,
                                                                  data->jstrActionDescription,
                                                                  NULL);

  return data->action_description;
}

static gboolean
jaw_action_set_description (AtkAction *action, gint i, const gchar *description)
{
    JAW_DEBUG("%s(%p, %d, %s)", __func__, action, i, description);
    g_warning("it is impossible to set the Accessible description on this Java object");
    return FALSE;
}

static const gchar*
jaw_action_get_name (AtkAction *action, gint i)
{
    JAW_DEBUG("%s(%p, %d)", __func__, action, i);
    return "";
}

static const gchar*
jaw_action_get_localized_name (AtkAction *action, gint i)
{
    JAW_DEBUG("%s(%p, %d)", __func__, action, i);
  JawObject *jaw_obj = JAW_OBJECT(action);
  ActionData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_ACTION);
  JNIEnv *env = jaw_util_get_jni_env();
  jobject atk_action = (*env)->NewGlobalRef(env, data->atk_action);
  if (!atk_action) {
    return NULL;
  }

  jclass classAtkAction = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkAction");
  jmethodID jmid = (*env)->GetMethodID(env,
                                       classAtkAction,
                                       "getLocalizedName",
                                       "(I)Ljava/lang/String;");
  jstring jstr = (*env)->CallObjectMethod(env, atk_action, jmid, (jint)i);
  (*env)->DeleteGlobalRef(env, atk_action);
  if (data->action_name != NULL)
  {
    (*env)->ReleaseStringUTFChars(env, data->jstrLocalizedName, data->action_name);
    (*env)->DeleteGlobalRef(env, data->jstrLocalizedName);
  }
  data->jstrLocalizedName = (*env)->NewGlobalRef(env, jstr);
  data->action_name = (gchar*)(*env)->GetStringUTFChars(env, data->jstrLocalizedName, NULL);
  return data->action_name;
}

static const gchar*
jaw_action_get_keybinding (AtkAction *action, gint i)
{
    JAW_DEBUG("%s(%p, %d)", __func__, action, i);
  JawObject *jaw_obj = JAW_OBJECT(action);
  if (jaw_obj == NULL)
    return NULL;

  ActionData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_ACTION);
  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jobject atk_action = (*jniEnv)->NewGlobalRef(jniEnv, data->atk_action);
  if (!atk_action) {
    return NULL;
  }

  jclass classAtkAction = (*jniEnv)->FindClass(jniEnv,
                                               "org/GNOME/Accessibility/AtkAction");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkAction,
                                          "get_keybinding",
                                          "(I)Ljava/lang/String;");
  jstring jstr = (*jniEnv)->CallObjectMethod(jniEnv, atk_action, jmid, (jint)i);
  (*jniEnv)->DeleteGlobalRef(jniEnv, atk_action);

  if (data->action_keybinding != NULL)
  {
    (*jniEnv)->ReleaseStringUTFChars(jniEnv,
                                     data->jstrActionKeybinding,
                                     data->action_keybinding);

    (*jniEnv)->DeleteGlobalRef(jniEnv, data->jstrActionKeybinding);
  }

  data->jstrActionKeybinding = (*jniEnv)->NewGlobalRef(jniEnv, jstr);
  data->action_keybinding = (gchar*)(*jniEnv)->GetStringUTFChars(jniEnv,
                                                                 data->jstrActionKeybinding,
                                                                 NULL);
  return data->action_keybinding;
}
