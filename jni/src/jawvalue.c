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

extern void	jaw_value_interface_init (AtkValueIface*);
extern gpointer	jaw_value_data_init (jobject ac);
extern void	jaw_value_data_finalize (gpointer);

static void			jaw_value_get_current_value		(AtkValue	*obj,
									 GValue		*value);
static void			jaw_value_get_maximum_value		(AtkValue	*obj,
									 GValue		*value);
static void			jaw_value_get_minimum_value		(AtkValue	*obj,
									 GValue		*value);
static gboolean			jaw_value_set_current_value		(AtkValue	*obj,
									 const GValue	*value);
static void			jaw_value_get_minimum_increment		(AtkValue	*obj,
									 GValue		*value);

typedef struct _ValueData {
	jobject atk_value;
} ValueData;

void
jaw_value_interface_init (AtkValueIface *iface)
{
	iface->get_current_value = jaw_value_get_current_value;
	iface->get_maximum_value = jaw_value_get_maximum_value;
	iface->get_minimum_value = jaw_value_get_minimum_value;
	iface->set_current_value = jaw_value_set_current_value;
	iface->get_minimum_increment = jaw_value_get_minimum_increment;
}

gpointer
jaw_value_data_init (jobject ac)
{
	ValueData *data = g_new0(ValueData, 1);

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classValue = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkValue");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classValue, "<init>", "(Ljavax/accessibility/AccessibleContext;)V");
	jobject jatk_value = (*jniEnv)->NewObject(jniEnv, classValue, jmid, ac);
	data->atk_value = (*jniEnv)->NewGlobalRef(jniEnv, jatk_value);

	return data;
}

void
jaw_value_data_finalize (gpointer p)
{
	ValueData *data = (ValueData*)p;
	JNIEnv *jniEnv = jaw_util_get_jni_env();

	if (data && data->atk_value) {
		(*jniEnv)->DeleteGlobalRef(jniEnv, data->atk_value);
		data->atk_value = NULL;
	}
}

static void get_g_value_from_java_number (JNIEnv *jniEnv,
		jobject jnumber, GValue *value)
{
	jclass classByte = (*jniEnv)->FindClass(jniEnv, "java/lang/Byte");
	jclass classDouble = (*jniEnv)->FindClass(jniEnv, "java/lang/Double");
	jclass classFloat = (*jniEnv)->FindClass(jniEnv, "java/lang/Float");
	jclass classInteger = (*jniEnv)->FindClass(jniEnv, "java/lang/Integer");
	jclass classLong = (*jniEnv)->FindClass(jniEnv, "java/lang/Long");
	jclass classShort = (*jniEnv)->FindClass(jniEnv, "java/lang/Short");

	jmethodID jmid;

	if ((*jniEnv)->IsInstanceOf(jniEnv, jnumber, classByte)) {
		jmid = (*jniEnv)->GetMethodID(jniEnv, classByte, "byteValue", "()B");
		g_value_init(value, G_TYPE_CHAR);
		g_value_set_char(value,
				(gchar)(*jniEnv)->CallByteMethod(jniEnv, jnumber, jmid));

		return;
	}

	if ((*jniEnv)->IsInstanceOf(jniEnv, jnumber, classDouble)) {
		jmid = (*jniEnv)->GetMethodID(jniEnv, classDouble, "doubleValue", "()D");
		g_value_init(value, G_TYPE_DOUBLE);
		g_value_set_double(value,
				(gdouble)(*jniEnv)->CallDoubleMethod(jniEnv, jnumber, jmid));

		return;
	}

	if ((*jniEnv)->IsInstanceOf(jniEnv, jnumber, classFloat)) {
		jmid = (*jniEnv)->GetMethodID(jniEnv, classFloat, "floatValue", "()F");
		g_value_init(value, G_TYPE_FLOAT);
		g_value_set_float(value,
				(gfloat)(*jniEnv)->CallFloatMethod(jniEnv, jnumber, jmid));

		return;
	}

	if ((*jniEnv)->IsInstanceOf(jniEnv, jnumber, classInteger)
		|| (*jniEnv)->IsInstanceOf(jniEnv, jnumber, classShort)) {
		jmid = (*jniEnv)->GetMethodID(jniEnv, classInteger, "intValue", "()I");
		g_value_init(value, G_TYPE_INT);
		g_value_set_int(value,
				(gint)(*jniEnv)->CallIntMethod(jniEnv, jnumber, jmid));

		return;
	}

	if ((*jniEnv)->IsInstanceOf(jniEnv, jnumber, classLong)) {
		jmid = (*jniEnv)->GetMethodID(jniEnv, classLong, "longValue", "()J");
		g_value_init(value, G_TYPE_INT64);
		g_value_set_int64(value,
				(gint64)(*jniEnv)->CallLongMethod(jniEnv, jnumber, jmid));

		return;
	}
}

static void
jaw_value_get_current_value (AtkValue *obj, GValue *value)
{
	if (!value) {
		return;
	}

	JawObject *jaw_obj = JAW_OBJECT(obj);
	ValueData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_VALUE);
	jobject atk_value = data->atk_value;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkValue = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkValue");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkValue, "get_current_value", "()Ljava/lang/Number;");
	jobject jnumber = (*jniEnv)->CallObjectMethod(jniEnv, atk_value, jmid);

	if (!jnumber) {
		return;
	}

	get_g_value_from_java_number(jniEnv, jnumber, value);
}

static void
jaw_value_get_maximum_value (AtkValue *obj, GValue *value)
{
	if (!value) {
		return;
	}

	JawObject *jaw_obj = JAW_OBJECT(obj);
	ValueData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_VALUE);
	jobject atk_value = data->atk_value;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkValue = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkValue");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkValue, "get_maximum_value", "()Ljava/lang/Number;");
	jobject jnumber = (*jniEnv)->CallObjectMethod(jniEnv, atk_value, jmid);

	if (!jnumber) {
		return;
	}

	get_g_value_from_java_number(jniEnv, jnumber, value);
}

static void
jaw_value_get_minimum_value (AtkValue *obj, GValue *value)
{
	if (!value) {
		return;
	}

	JawObject *jaw_obj = JAW_OBJECT(obj);
	ValueData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_VALUE);
	jobject atk_value = data->atk_value;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkValue = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkValue");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkValue, "get_minimum_value", "()Ljava/lang/Number;");
	jobject jnumber = (*jniEnv)->CallObjectMethod(jniEnv, atk_value, jmid);

	if (!jnumber) {
		return;
	}

	get_g_value_from_java_number(jniEnv, jnumber, value);
}

static gboolean
jaw_value_set_current_value (AtkValue *obj, const GValue *value)
{
	if (!value) {
		return FALSE;
	}

	if (!G_TYPE_IS_FUNDAMENTAL (G_VALUE_TYPE (value))) {
		return FALSE;
	}

	JawObject *jaw_obj = JAW_OBJECT(obj);
	ValueData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_VALUE);
	jobject atk_value = data->atk_value;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkValue = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkValue");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkValue, "set_current_value", "(Ljava/lang/Number;)Z");

	jboolean jbool;
	jclass klass;
	jmethodID jmidInit;
	jobject o;
	switch (value->g_type) {
		case G_TYPE_CHAR:
		{
			gchar c = g_value_get_char(value);
			klass = (*jniEnv)->FindClass(jniEnv, "java/lang/Byte");
			jmidInit = (*jniEnv)->GetMethodID(jniEnv, klass, "<init>", "(B)V");
			o = (*jniEnv)->NewObject(jniEnv, klass, jmidInit, (jbyte)c);
			jbool = (*jniEnv)->CallBooleanMethod(jniEnv, atk_value, jmid, o);
			break;
		}
		case G_TYPE_DOUBLE:
		{
			gdouble d = g_value_get_double(value);
			klass = (*jniEnv)->FindClass(jniEnv, "java/lang/Double");
			jmidInit = (*jniEnv)->GetMethodID(jniEnv, klass, "<init>", "(D)V");
			o = (*jniEnv)->NewObject(jniEnv, klass, jmidInit, (jdouble)d);
			jbool = (*jniEnv)->CallBooleanMethod(jniEnv, atk_value, jmid, o);
			break;
		}
		case G_TYPE_FLOAT:
		{
			gfloat f = g_value_get_float(value);
			klass = (*jniEnv)->FindClass(jniEnv, "java/lang/Float");
			jmidInit = (*jniEnv)->GetMethodID(jniEnv, klass, "<init>", "(F)V");
			o = (*jniEnv)->NewObject(jniEnv, klass, jmidInit, (jfloat)f);
			jbool = (*jniEnv)->CallBooleanMethod(jniEnv, atk_value, jmid, o);
			break;
		}
		case G_TYPE_INT:
		{
			gint i = g_value_get_int(value);
			klass = (*jniEnv)->FindClass(jniEnv, "java/lang/Integer");
			jmidInit = (*jniEnv)->GetMethodID(jniEnv, klass, "<init>", "(I)V");
			o = (*jniEnv)->NewObject(jniEnv, klass, jmidInit, (jint)i);
			jbool = (*jniEnv)->CallBooleanMethod(jniEnv, atk_value, jmid, o);
			break;
		}
		case G_TYPE_INT64:
		{
			gint64 i64 = g_value_get_int64(value);
			klass = (*jniEnv)->FindClass(jniEnv, "java/lang/Long");
			jmidInit = (*jniEnv)->GetMethodID(jniEnv, klass, "<init>", "(J)V");
			o = (*jniEnv)->NewObject(jniEnv, klass, jmidInit, (jlong)i64);
			jbool = (*jniEnv)->CallBooleanMethod(jniEnv, atk_value, jmid, o);
			break;
		}
		default:
			return FALSE;
	}

	return (jbool == JNI_TRUE) ? TRUE : FALSE;
}

static void
jaw_value_get_minimum_increment (AtkValue *obj, GValue *value)
{
	if (!value) {
		return;
	}

	GValue curValue = {0,};
	atk_value_get_current_value(obj, &curValue);

	if (G_TYPE_IS_FUNDAMENTAL (G_VALUE_TYPE (&curValue))) {
		switch (curValue.g_type) {
			case G_TYPE_CHAR:
			case G_TYPE_INT:
			case G_TYPE_INT64:
			{
				g_value_init(value, G_TYPE_INT);
				g_value_set_int(value, 1);
				return;
			}
			default:
				break;
		}
	}

	g_value_init(value, G_TYPE_DOUBLE);
	g_value_set_double(value, 0.0);
}

