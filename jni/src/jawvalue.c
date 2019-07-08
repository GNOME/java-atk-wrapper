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

#include <atk/atk.h>
#include <glib.h>
#include "jawimpl.h"
#include "jawutil.h"

#ifdef __cplusplus
extern "C" {
#endif

static void jaw_value_get_current_value(AtkValue *obj, GValue *value);
static void    jaw_value_set_value(AtkValue *obj, const gdouble value);
static gdouble jaw_value_get_increment (AtkValue *obj);
static AtkRange* jaw_value_get_range(AtkValue *obj);

typedef struct _ValueData {
  jobject atk_value;
} ValueData;

void
jaw_value_interface_init (AtkValueIface *iface, gpointer data)
{
  iface->get_current_value = jaw_value_get_current_value;
  iface->set_value = jaw_value_set_value;
  iface->get_increment = jaw_value_get_increment;
  iface->get_range = jaw_value_get_range;
}

gpointer
jaw_value_data_init (jobject ac)
{
    JAW_DEBUG("%s(%p)", __func__, ac);
  ValueData *data = g_new0(ValueData, 1);

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classValue = (*jniEnv)->FindClass(jniEnv,
                                           "org/GNOME/Accessibility/AtkValue");
  jmethodID jmid = (*jniEnv)->GetStaticMethodID(jniEnv,
                                          classValue,
                                          "createAtkValue",
                                          "(Ljavax/accessibility/AccessibleContext;)Lorg/GNOME/Accessibility/AtkValue;");
  jobject jatk_value = (*jniEnv)->CallStaticObjectMethod(jniEnv, classValue, jmid, ac);
  data->atk_value = (*jniEnv)->NewWeakGlobalRef(jniEnv, jatk_value);

  return data;
}

void
jaw_value_data_finalize (gpointer p)
{
    JAW_DEBUG("%s(%p)", __func__, p);
  ValueData *data = (ValueData*)p;
  JNIEnv *jniEnv = jaw_util_get_jni_env();

  if (data && data->atk_value)
  {
    (*jniEnv)->DeleteWeakGlobalRef(jniEnv, data->atk_value);
    data->atk_value = NULL;
  }
}

//FIXME getMaximumValue(), getIncrement() and getMinimumValue() are return double it is better to cast everithing
static void
jaw_value_get_current_value (AtkValue *obj, GValue *value)
{
    JAW_DEBUG("%s(%p, %p)", __func__, obj, value);
    if (!value)
        return;
    JawObject *jaw_obj = JAW_OBJECT(obj);
    if(!jaw_obj)
        return;
    ValueData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_VALUE);
    JNIEnv *jniEnv = jaw_util_get_jni_env();
    jobject atk_value = (*jniEnv)->NewGlobalRef(jniEnv, data->atk_value);
    if (!atk_value)
        return;
    jclass classAtkValue = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkValue");
    jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkValue, "get_current_value", "()D");
    jdouble jnumber = (*jniEnv)->CallDoubleMethod(jniEnv, atk_value, jmid);
    (*jniEnv)->DeleteGlobalRef(jniEnv, atk_value);
    if (!jnumber)
        return;
    g_value_init(value, G_TYPE_DOUBLE);
    g_value_set_double(value, (gdouble)jnumber);
}

static void
jaw_value_set_value(AtkValue *obj, const gdouble value)
{
    JAW_DEBUG("%s(%p, %lf)", __func__, obj, value);
  if (!value)
    return;

  JawObject *jaw_obj = JAW_OBJECT(obj);
  ValueData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_VALUE);
  JNIEnv *env = jaw_util_get_jni_env();
  jobject atk_value = (*env)->NewGlobalRef(env, data->atk_value);
  if (!atk_value) {
    return;
  }

  jclass classAtkValue = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkValue");
  jmethodID jmid = (*env)->GetMethodID(env,
                                       classAtkValue,
                                          "setValue",
                                          "(D)V");
  (*env)->CallVoidMethod(env, atk_value, jmid,(jdouble)value);
  (*env)->DeleteGlobalRef(env, atk_value);
}

static AtkRange*
jaw_value_get_range(AtkValue *obj)
{
    JAW_DEBUG("%s(%p)", __func__, obj);
  JawObject *jaw_obj = JAW_OBJECT(obj);
  ValueData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_VALUE);
  JNIEnv *env = jaw_util_get_jni_env();
  jobject atk_value = (*env)->NewGlobalRef(env, data->atk_value);
  if (!atk_value) {
    return NULL;
  }

  jclass classAtkValue = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkValue");
  jmethodID jmidMin = (*env)->GetMethodID(env, classAtkValue, "getMinimumValue", "()D");
  jmethodID jmidMax = (*env)->GetMethodID(env, classAtkValue, "getMaximumValue", "()D");
  AtkRange *ret = atk_range_new((gdouble)(*env)->CallDoubleMethod(env, atk_value, jmidMin),
                       (gdouble)(*env)->CallDoubleMethod(env, atk_value, jmidMax),
                       NULL); // NULL description
  (*env)->DeleteGlobalRef(env, atk_value);
  return ret;
}

static gdouble
jaw_value_get_increment (AtkValue *obj)
{
    JAW_DEBUG("%s(%p)", __func__, obj);
  JawObject *jaw_obj = JAW_OBJECT(obj);
  ValueData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_VALUE);
  JNIEnv *env = jaw_util_get_jni_env();
  jobject atk_value = (*env)->NewGlobalRef(env, data->atk_value);
  if (!atk_value) {
    return 0.;
  }
  jclass classAtkValue = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkValue");
  jmethodID jmid = (*env)->GetMethodID(env, classAtkValue, "getIncrement", "()D");
  gdouble ret = (*env)->CallDoubleMethod(env, atk_value, jmid);
  (*env)->DeleteGlobalRef(env, atk_value);
  return ret;
}

#ifdef __cplusplus
}
#endif
