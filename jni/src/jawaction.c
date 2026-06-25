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

#include "jawimpl.h"
#include "jawutil.h"
#include <atk/atk.h>
#include <glib.h>

/**
 * (From Atk documentation)
 *
 * AtkAction:
 *
 * The ATK interface provided by UI components
 * which the user can activate/interact with.
 *
 * #AtkAction should be implemented by instances of #AtkObject classes
 * with which the user can interact directly, i.e. buttons,
 * checkboxes, scrollbars, e.g. components which are not "passive"
 * providers of UI information.
 *
 * Exceptions: when the user interaction is already covered by another
 * appropriate interface such as #AtkEditableText (insert/delete text,
 * etc.) or #AtkValue (set value) then these actions should not be
 * exposed by #AtkAction as well.
 *
 * Though most UI interactions on components should be invocable via
 * keyboard as well as mouse, there will generally be a close mapping
 * between "mouse actions" that are possible on a component and the
 * AtkActions.  Where mouse and keyboard actions are redundant in
 * effect, #AtkAction should expose only one action rather than
 * exposing redundant actions if possible.  By convention we have been
 * using "mouse centric" terminology for #AtkAction names.
 *
 */

static gboolean jaw_action_do_action (AtkAction *action, gint i);
static gint jaw_action_get_n_actions (AtkAction *action);
static const gchar *jaw_action_get_description (AtkAction *action, gint i);
static const gchar *jaw_action_get_keybinding (AtkAction *action, gint i);
static gboolean jaw_action_set_description (AtkAction *action, gint i, const gchar *description);
static const gchar *jaw_action_get_localized_name (AtkAction *action, gint i);

typedef struct _ActionData
{
  jobject atk_action;
  gchar *localized_name;
  jstring jstrLocalizedName;
  gchar *action_description;
  jstring jstrActionDescription;
  gchar *action_keybinding;
  jstring jstrActionKeybinding;
} ActionData;

#define JAW_GET_ACTION(action, def_ret) \
  JAW_GET_OBJ_IFACE (action, INTERFACE_ACTION, ActionData, atk_action, jniEnv, atk_action, def_ret)

/**
 * AtkActionIface:
 * @do_action:
 * @get_n_actions:
 * @get_description:
 * @get_name:
 * @get_keybinding:
 * @set_description:
 * @get_localized_name:
 **/

void
jaw_action_interface_init (AtkActionIface *iface, gpointer data)
{
  JAW_DEBUG_ALL ("%p, %p", iface, data);
  iface->do_action = jaw_action_do_action;
  iface->get_n_actions = jaw_action_get_n_actions;
  // FIXME: missing java support for distinguishing name and description
  iface->get_description = jaw_action_get_description;
  iface->get_name = jaw_action_get_description;
  iface->get_keybinding = jaw_action_get_keybinding;
  iface->set_description = jaw_action_set_description;
  iface->get_localized_name = jaw_action_get_localized_name;
}

/**
 * jaw_action_data_init:
 * @ac: a Java AccessibleContext object
 *
 * Initializes action interface data for an AccessibleContext.
 *
 * Creates a Java AtkAction wrapper and stores it in an ActionData structure.
 *
 * Explicitly manages a JNI local reference frame using
 * PushLocalFrame/PopLocalFrame; all local references are released
 * before the function returns.
 *
 * Returns: (transfer full): ActionData pointer, or %NULL on error
 */

gpointer
jaw_action_data_init (jobject ac)
{
  JAW_DEBUG_ALL ("%p", ac);
  ActionData *data = g_new0 (ActionData, 1);

  JNIEnv *jniEnv = jaw_util_get_jni_env ();
  jclass classAction = (*jniEnv)->FindClass (jniEnv,
                                             "org/GNOME/Accessibility/AtkAction");
  jmethodID jmid = (*jniEnv)->GetStaticMethodID (jniEnv,
                                                 classAction,
                                                 "createAtkAction",
                                                 "(Ljavax/accessibility/AccessibleContext;)Lorg/GNOME/Accessibility/AtkAction;");
  jobject jatk_action = (*jniEnv)->CallStaticObjectMethod (jniEnv, classAction, jmid, ac);
  data->atk_action = (*jniEnv)->NewGlobalRef (jniEnv, jatk_action);

  return data;
}

/**
 * jaw_action_data_finalize:
 * @p: ActionData pointer to finalize
 *
 * Cleans up ActionData when the parent GObject is finalized.
 * Called from jaw_impl_finalize() when the object's reference count reaches
 * zero.
 */

void
jaw_action_data_finalize (gpointer p)
{
  JAW_DEBUG_ALL ("%p", p);
  ActionData *data = (ActionData *) p;
  JNIEnv *jniEnv = jaw_util_get_jni_env ();

  if (data && data->atk_action)
    {
      if (data->localized_name != NULL)
        {
          (*jniEnv)->ReleaseStringUTFChars (jniEnv, data->jstrLocalizedName, data->localized_name);
          (*jniEnv)->DeleteGlobalRef (jniEnv, data->jstrLocalizedName);
          data->jstrLocalizedName = NULL;
          data->localized_name = NULL;
        }

      if (data->action_description != NULL)
        {
          (*jniEnv)->ReleaseStringUTFChars (jniEnv,
                                            data->jstrActionDescription,
                                            data->action_description);
          (*jniEnv)->DeleteGlobalRef (jniEnv, data->jstrActionDescription);
          data->jstrActionDescription = NULL;
          data->action_description = NULL;
        }

      if (data->action_keybinding != NULL)
        {
          (*jniEnv)->ReleaseStringUTFChars (jniEnv,
                                            data->jstrActionKeybinding,
                                            data->action_keybinding);
          (*jniEnv)->DeleteGlobalRef (jniEnv, data->jstrActionKeybinding);
          data->jstrActionKeybinding = NULL;
          data->action_keybinding = NULL;
        }

      (*jniEnv)->DeleteGlobalRef (jniEnv, data->atk_action);
      data->atk_action = NULL;
    }
}

/**
 * jaw_action_do_action:
 * @action: a #GObject instance that implements AtkActionIface
 * @i: the action index corresponding to the action to be performed
 *
 * Perform the specified action on the object.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 *
 * Returns: %TRUE if success, %FALSE otherwise
 **/

static gboolean
jaw_action_do_action (AtkAction *action, gint i)
{
  JAW_DEBUG_C ("%p, %d", action, i);
  JAW_GET_ACTION (action, FALSE);

  jclass classAtkAction = (*jniEnv)->FindClass (jniEnv,
                                                "org/GNOME/Accessibility/AtkAction");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkAction,
                                           "do_action",
                                           "(I)Z");
  jboolean jresult = (*jniEnv)->CallBooleanMethod (jniEnv,
                                                   atk_action,
                                                   jmid,
                                                   (jint) i);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_action);
  return jresult;
}

/**
 * jaw_action_get_n_actions:
 * @action: a #GObject instance that implements AtkActionIface
 *
 * Gets the number of accessible actions available on the object.
 * If there are more than one, the first one is considered the
 * "default" action of the object.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 *
 * Returns: the number of actions, or 0 if @action does not
 * implement this interface.
 **/

static gint
jaw_action_get_n_actions (AtkAction *action)
{
  JAW_DEBUG_C ("%p", action);
  JAW_GET_ACTION (action, 0);

  jclass classAtkAction = (*jniEnv)->FindClass (jniEnv,
                                                "org/GNOME/Accessibility/AtkAction");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkAction,
                                           "get_n_actions", "()I");

  gint ret = (gint) (*jniEnv)->CallIntMethod (jniEnv, atk_action, jmid);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_action);
  return ret;
}

/**
 * jaw_action_get_description:
 * @action: a #GObject instance that implements AtkActionIface
 * @i: the action index corresponding to the action to be performed
 *
 * Returns a description of the specified action of the object.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * Returns: (nullable): a description string for action @i, or %NULL if
 * @action does not implement this interface or if an error occurs.
 **/

static const gchar *
jaw_action_get_description (AtkAction *action, gint i)
{
  JAW_DEBUG_C ("%p, %d", action, i);
  JAW_GET_ACTION (action, NULL);

  jclass classAtkAction = (*jniEnv)->FindClass (jniEnv,
                                                "org/GNOME/Accessibility/AtkAction");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkAction,
                                           "get_description",
                                           "(I)Ljava/lang/String;");
  jstring jstr = (*jniEnv)->CallObjectMethod (jniEnv,
                                              atk_action,
                                              jmid,
                                              (jint) i);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_action);

  if (data->action_description != NULL)
    {
      (*jniEnv)->ReleaseStringUTFChars (jniEnv,
                                        data->jstrActionDescription,
                                        data->action_description);
      (*jniEnv)->DeleteGlobalRef (jniEnv, data->jstrActionDescription);
      data->jstrActionDescription = NULL;
      data->action_description = NULL;
    }

  if (jstr)
    {
      data->jstrActionDescription = (*jniEnv)->NewGlobalRef (jniEnv, jstr);
      data->action_description = (gchar *) (*jniEnv)->GetStringUTFChars (jniEnv,
                                                                         data->jstrActionDescription,
                                                                         NULL);
    }

  return data->action_description;
}

/**
 * jaw_action_set_description:
 * @action: a #GObject instance that implements AtkActionIface
 * @i: the action index corresponding to the action to be performed
 * @description: the description to be assigned to this action
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * Returns: %TRUE if the description was successfully set, %FALSE otherwise.
 **/

static gboolean
jaw_action_set_description (AtkAction *action, gint i, const gchar *description)
{
  JAW_DEBUG_C ("%p, %d, %s", action, i, description);
  JAW_GET_ACTION (action, FALSE);

  jclass classAtkAction = (*jniEnv)->FindClass (jniEnv,
                                                "org/GNOME/Accessibility/AtkAction");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkAction,
                                           "setDescription",
                                           "(ILjava/lang/String;)Z");
  jboolean jisset = (*jniEnv)->CallBooleanMethod (jniEnv,
                                                  atk_action,
                                                  jmid,
                                                  (jint) i,
                                                  (jstring) description);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_action);
  return jisset;
}

/**
 * jaw_action_get_localized_name:
 * @action: a #GObject instance that implements AtkActionIface
 * @i: the action index corresponding to the action to be performed
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * Returns: (nullable): a localized name string for action @i, or %NULL
 *   if @action does not implement this interface or if an error occurs.
 **/

static const gchar *
jaw_action_get_localized_name (AtkAction *action, gint i)
{
  JAW_DEBUG_C ("%p, %d", action, i);
  JAW_GET_ACTION (action, NULL);

  jclass classAtkAction = (*jniEnv)->FindClass (jniEnv, "org/GNOME/Accessibility/AtkAction");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkAction,
                                           "getLocalizedName",
                                           "(I)Ljava/lang/String;");
  jstring jstr = (*jniEnv)->CallObjectMethod (jniEnv, atk_action, jmid, (jint) i);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_action);
  if (data->localized_name != NULL)
    {
      (*jniEnv)->ReleaseStringUTFChars (jniEnv, data->jstrLocalizedName, data->localized_name);
      (*jniEnv)->DeleteGlobalRef (jniEnv, data->jstrLocalizedName);
    }
  data->jstrLocalizedName = (*jniEnv)->NewGlobalRef (jniEnv, jstr);
  data->localized_name = (gchar *) (*jniEnv)->GetStringUTFChars (jniEnv, data->jstrLocalizedName, NULL);
  return data->localized_name;
}

static const gchar *
jaw_action_get_keybinding (AtkAction *action, gint i)
{
  JAW_DEBUG_C ("%p, %d", action, i);
  JAW_GET_ACTION (action, NULL);

  jclass classAtkAction = (*jniEnv)->FindClass (jniEnv,
                                                "org/GNOME/Accessibility/AtkAction");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkAction,
                                           "get_keybinding",
                                           "(I)Ljava/lang/String;");
  jstring jstr = (*jniEnv)->CallObjectMethod (jniEnv, atk_action, jmid, (jint) i);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_action);

  if (data->action_keybinding != NULL)
    {
      (*jniEnv)->ReleaseStringUTFChars (jniEnv,
                                        data->jstrActionKeybinding,
                                        data->action_keybinding);

      (*jniEnv)->DeleteGlobalRef (jniEnv, data->jstrActionKeybinding);
    }

  data->jstrActionKeybinding = (*jniEnv)->NewGlobalRef (jniEnv, jstr);
  data->action_keybinding = (gchar *) (*jniEnv)->GetStringUTFChars (jniEnv,
                                                                    data->jstrActionKeybinding,
                                                                    NULL);
  return data->action_keybinding;
}
