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

extern void	jaw_table_interface_init (AtkTableIface*);
extern gpointer	jaw_table_data_init (jobject ac);
extern void	jaw_table_data_finalize (gpointer);

static AtkObject*		jaw_table_ref_at			(AtkTable	*table,
									 gint		row,
									 gint		column);
static gint			jaw_table_get_index_at			(AtkTable	*table,
									 gint		row,
									 gint		column);
static gint			jaw_table_get_column_at_index		(AtkTable	*table,
									 gint		index);
static gint			jaw_table_get_row_at_index		(AtkTable	*table,
									 gint		index);
static gint			jaw_table_get_n_columns			(AtkTable	*table);
static gint			jaw_table_get_n_rows			(AtkTable	*table);
static gint			jaw_table_get_column_extent_at		(AtkTable	*table,
									 gint		row,
									 gint		column);
static gint			jaw_table_get_row_extent_at		(AtkTable	*table,
									 gint		row,
									 gint		column);
static AtkObject*		jaw_table_get_caption			(AtkTable	*table);
static const gchar*		jaw_table_get_column_description	(AtkTable	*table,
									 gint		column);
static const gchar*		jaw_table_get_row_description		(AtkTable	*table,
									 gint		row);
static AtkObject*		jaw_table_get_column_header		(AtkTable	*table,
									 gint		column);
static AtkObject*		jaw_table_get_row_header		(AtkTable	*table,
									 gint		row);
static AtkObject*		jaw_table_get_summary			(AtkTable	*table);
static gint			jaw_table_get_selected_columns		(AtkTable	*table,
									 gint		**selected);
static gint			jaw_table_get_selected_rows		(AtkTable	*table,
									 gint		**selected);
static gboolean			jaw_table_is_column_selected		(AtkTable	*table,
									 gint		column);
static gboolean			jaw_table_is_row_selected		(AtkTable	*table,
									 gint		row);
static gboolean			jaw_table_is_selected			(AtkTable	*table,
									 gint		row,
									 gint		column);

typedef struct _TableData {
	jobject atk_table;
	gchar* description;
	jstring jstrDescription;
} TableData;

void
jaw_table_interface_init (AtkTableIface *iface)
{
	iface->ref_at = jaw_table_ref_at;
	iface->get_index_at = jaw_table_get_index_at;
	iface->get_column_at_index = jaw_table_get_column_at_index;
	iface->get_row_at_index = jaw_table_get_row_at_index;
	iface->get_n_columns = jaw_table_get_n_columns;
	iface->get_n_rows = jaw_table_get_n_rows;
	iface->get_column_extent_at = jaw_table_get_column_extent_at;
	iface->get_row_extent_at = jaw_table_get_row_extent_at;
	iface->get_caption = jaw_table_get_caption;
	iface->get_column_description = jaw_table_get_column_description;
	iface->get_row_description = jaw_table_get_row_description;
	iface->get_column_header = jaw_table_get_column_header;
	iface->get_row_header = jaw_table_get_row_header;
	iface->get_summary = jaw_table_get_summary;
	iface->get_selected_columns = jaw_table_get_selected_columns;
	iface->get_selected_rows = jaw_table_get_selected_rows;
	iface->is_column_selected = jaw_table_is_column_selected;
	iface->is_row_selected = jaw_table_is_row_selected;
	iface->is_selected = jaw_table_is_selected;
}

gpointer
jaw_table_data_init (jobject ac)
{
	TableData *data = g_new0(TableData, 1);

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classTable = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkTable");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classTable, "<init>", "(Ljavax/accessibility/AccessibleContext;)V");
	jobject jatk_table = (*jniEnv)->NewObject(jniEnv, classTable, jmid, ac);
	data->atk_table = (*jniEnv)->NewGlobalRef(jniEnv, jatk_table);

	return data;
}

void
jaw_table_data_finalize (gpointer p)
{
	TableData *data = (TableData*)p;
	JNIEnv *jniEnv = jaw_util_get_jni_env();

	if (data && data->atk_table) {
		if (data->description != NULL) {
			(*jniEnv)->ReleaseStringUTFChars(jniEnv, data->jstrDescription, data->description);
			(*jniEnv)->DeleteGlobalRef(jniEnv, data->jstrDescription);
			data->jstrDescription = NULL;
			data->description = NULL;
		}

		(*jniEnv)->DeleteGlobalRef(jniEnv, data->atk_table);
		data->atk_table = NULL;
	}
}

static AtkObject*
jaw_table_ref_at (AtkTable *table, gint	row, gint column)
{
	JawObject *jaw_obj = JAW_OBJECT(table);
	TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
	jobject atk_table = data->atk_table;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkTable = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkTable");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkTable, "ref_at", "(II)Ljavax/accessibility/AccessibleContext;");
	jobject jac = (*jniEnv)->CallObjectMethod(jniEnv, atk_table, jmid, (jint)row, (jint)column);

	if (!jac) {
		return NULL;
	}

	JawImpl* jaw_impl = jaw_impl_get_instance( jniEnv, jac );

	g_object_ref( G_OBJECT(jaw_impl) );

	return ATK_OBJECT(jaw_impl);
}

static gint
jaw_table_get_index_at (AtkTable *table, gint row, gint column)
{
	JawObject *jaw_obj = JAW_OBJECT(table);
	TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
	jobject atk_table = data->atk_table;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkTable = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkTable");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkTable, "get_index_at", "(II)I");
	jint index = (*jniEnv)->CallIntMethod(jniEnv, atk_table, jmid, (jint)row, (jint)column);

	return (gint)index;
}

static gint
jaw_table_get_column_at_index (AtkTable *table, gint index)
{
	JawObject *jaw_obj = JAW_OBJECT(table);
	TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
	jobject atk_table = data->atk_table;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkTable = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkTable");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkTable, "get_column_at_index", "(I)I");
	jint jcolumn = (*jniEnv)->CallIntMethod(jniEnv, atk_table, jmid, (jint)index);

	return (gint)jcolumn;
}

static gint
jaw_table_get_row_at_index (AtkTable *table, gint index)
{
	JawObject *jaw_obj = JAW_OBJECT(table);
	TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
	jobject atk_table = data->atk_table;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkTable = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkTable");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkTable, "get_row_at_index", "(I)I");
	jint jrow = (*jniEnv)->CallIntMethod(jniEnv, atk_table, jmid, (jint)index);

	return (gint)jrow;
}

static gint
jaw_table_get_n_columns	(AtkTable *table)
{
	JawObject *jaw_obj = JAW_OBJECT(table);
	TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
	jobject atk_table = data->atk_table;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkTable = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkTable");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkTable, "get_n_columns", "()I");
	jint jcolumns = (*jniEnv)->CallIntMethod(jniEnv, atk_table, jmid);

	return (gint)jcolumns;
}

static gint
jaw_table_get_n_rows (AtkTable *table)
{
	JawObject *jaw_obj = JAW_OBJECT(table);
	TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
	jobject atk_table = data->atk_table;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkTable = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkTable");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkTable, "get_n_rows", "()I");
	jint jrows = (*jniEnv)->CallIntMethod(jniEnv, atk_table, jmid);

	return (gint)jrows;
}

static gint
jaw_table_get_column_extent_at (AtkTable *table, gint row, gint	column)
{
	JawObject *jaw_obj = JAW_OBJECT(table);
	TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
	jobject atk_table = data->atk_table;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkTable = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkTable");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkTable, "get_column_extent_at", "(II)I");
	jint jextent = (*jniEnv)->CallIntMethod(jniEnv, atk_table, jmid, (jint)row, (jint)column);

	return (gint)jextent;
}

static gint
jaw_table_get_row_extent_at (AtkTable *table, gint row, gint column)
{
	JawObject *jaw_obj = JAW_OBJECT(table);
	TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
	jobject atk_table = data->atk_table;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkTable = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkTable");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkTable, "get_row_extent_at", "(II)I");
	jint jextent = (*jniEnv)->CallIntMethod(jniEnv, atk_table, jmid, (jint)row, (jint)column);

	return (gint)jextent;
}

static AtkObject*
jaw_table_get_caption (AtkTable	*table)
{
	JawObject *jaw_obj = JAW_OBJECT(table);
	TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
	jobject atk_table = data->atk_table;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkTable = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkTable");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkTable, "get_caption", "()Ljavax/accessibility/AccessibleContext;");
	jobject jac = (*jniEnv)->CallObjectMethod(jniEnv, atk_table, jmid);

	if (!jac) {
		return NULL;
	}

	JawImpl* jaw_impl = jaw_impl_get_instance( jniEnv, jac );

	return ATK_OBJECT(jaw_impl);
}

static const gchar*
jaw_table_get_column_description (AtkTable *table, gint	column)
{
	JawObject *jaw_obj = JAW_OBJECT(table);
	TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
	jobject atk_table = data->atk_table;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkTable = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkTable");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkTable, "get_column_description", "(I)Ljava/lang/String;");
	jstring jstr = (*jniEnv)->CallObjectMethod(jniEnv, atk_table, jmid, (jint)column);

	if (data->description != NULL) {
		(*jniEnv)->ReleaseStringUTFChars(jniEnv, data->jstrDescription, data->description);
		(*jniEnv)->DeleteGlobalRef(jniEnv, data->jstrDescription);
	}
	
	data->jstrDescription = (*jniEnv)->NewGlobalRef(jniEnv, jstr);
	data->description = (gchar*)(*jniEnv)->GetStringUTFChars(jniEnv, data->jstrDescription, NULL);

	return data->description;
}

static const gchar*
jaw_table_get_row_description (AtkTable *table, gint row)
{
	JawObject *jaw_obj = JAW_OBJECT(table);
	TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
	jobject atk_table = data->atk_table;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkTable = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkTable");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkTable, "get_row_description", "(I)Ljava/lang/String;");
	jstring jstr = (*jniEnv)->CallObjectMethod(jniEnv, atk_table, jmid, (jint)row);

	if (data->description != NULL) {
		(*jniEnv)->ReleaseStringUTFChars(jniEnv, data->jstrDescription, data->description);
		(*jniEnv)->DeleteGlobalRef(jniEnv, data->jstrDescription);
	}
	
	data->jstrDescription = (*jniEnv)->NewGlobalRef(jniEnv, jstr);
	data->description = (gchar*)(*jniEnv)->GetStringUTFChars(jniEnv, data->jstrDescription, NULL);

	return data->description;
}

static AtkObject*
jaw_table_get_column_header (AtkTable *table, gint column)
{
	JawObject *jaw_obj = JAW_OBJECT(table);
	TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
	jobject atk_table = data->atk_table;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkTable = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkTable");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkTable, "get_column_header", "(I)Ljavax/accessibility/AccessibleContext;");
	jobject jac = (*jniEnv)->CallObjectMethod(jniEnv, atk_table, jmid, (jint)column);

	if (!jac) {
		return NULL;
	}

	JawImpl* jaw_impl = jaw_impl_get_instance( jniEnv, jac );

	return ATK_OBJECT(jaw_impl);
}

static AtkObject*
jaw_table_get_row_header (AtkTable *table, gint row)
{
	JawObject *jaw_obj = JAW_OBJECT(table);
	TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
	jobject atk_table = data->atk_table;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkTable = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkTable");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkTable, "get_row_header", "(I)Ljavax/accessibility/AccessibleContext;");
	jobject jac = (*jniEnv)->CallObjectMethod(jniEnv, atk_table, jmid, (jint)row);

	if (!jac) {
		return NULL;
	}

	JawImpl* jaw_impl = jaw_impl_get_instance( jniEnv, jac );

	return ATK_OBJECT(jaw_impl);
}

static AtkObject*
jaw_table_get_summary (AtkTable *table)
{
	JawObject *jaw_obj = JAW_OBJECT(table);
	TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
	jobject atk_table = data->atk_table;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkTable = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkTable");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkTable, "get_summary", "()Ljavax/accessibility/AccessibleContext;");
	jobject jac = (*jniEnv)->CallObjectMethod(jniEnv, atk_table, jmid);

	if (!jac) {
		return NULL;
	}

	JawImpl* jaw_impl = jaw_impl_get_instance( jniEnv, jac );

	return ATK_OBJECT(jaw_impl);
}

static gint
jaw_table_get_selected_columns (AtkTable *table, gint **selected)
{
	JawObject *jaw_obj = JAW_OBJECT(table);
	TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
	jobject atk_table = data->atk_table;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkTable = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkTable");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkTable, "get_selected_columns", "()[I");
	jintArray jcolumnArray = (*jniEnv)->CallObjectMethod(jniEnv, atk_table, jmid);

	if (!jcolumnArray) {
		return 0;
	}

	jsize length = (*jniEnv)->GetArrayLength(jniEnv, jcolumnArray);
	jint *jcolumns = (*jniEnv)->GetIntArrayElements(jniEnv, jcolumnArray, NULL);
	gint *columns = g_new(gint, length);

	gint i;
	for (i = 0; i < length; i++) {
		columns[i] = (gint)jcolumns[i];
	}

	(*jniEnv)->ReleaseIntArrayElements(jniEnv, jcolumnArray, jcolumns, JNI_ABORT);

	return (gint)length;
}

static gint
jaw_table_get_selected_rows (AtkTable *table, gint **selected)
{
	JawObject *jaw_obj = JAW_OBJECT(table);
	TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
	jobject atk_table = data->atk_table;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkTable = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkTable");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkTable, "get_selected_rows", "()[I");
	jintArray jrowArray = (*jniEnv)->CallObjectMethod(jniEnv, atk_table, jmid);

	if (!jrowArray) {
		return 0;
	}

	jsize length = (*jniEnv)->GetArrayLength(jniEnv, jrowArray);
	jint *jrows = (*jniEnv)->GetIntArrayElements(jniEnv, jrowArray, NULL);
	gint *rows = g_new(gint, length);

	gint i;
	for (i = 0; i < length; i++) {
		rows[i] = (gint)jrows[i];
	}

	(*jniEnv)->ReleaseIntArrayElements(jniEnv, jrowArray, jrows, JNI_ABORT);

	return (gint)length;
}

static gboolean
jaw_table_is_column_selected (AtkTable *table, gint column)
{
	JawObject *jaw_obj = JAW_OBJECT(table);
	TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
	jobject atk_table = data->atk_table;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkTable = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkTable");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkTable, "is_column_selected", "(I)Z");
	jboolean jselected = (*jniEnv)->CallBooleanMethod(jniEnv, atk_table, jmid, (jint)column);

	if (jselected == JNI_TRUE) {
		return TRUE;
	}

	return FALSE;
}

static gboolean
jaw_table_is_row_selected (AtkTable *table, gint row)
{
	JawObject *jaw_obj = JAW_OBJECT(table);
	TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
	jobject atk_table = data->atk_table;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkTable = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkTable");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkTable, "is_row_selected", "(I)Z");
	jboolean jselected = (*jniEnv)->CallBooleanMethod(jniEnv, atk_table, jmid, (jint)row);

	if (jselected == JNI_TRUE) {
		return TRUE;
	}

	return FALSE;
}

static gboolean
jaw_table_is_selected (AtkTable *table, gint row, gint column)
{
	JawObject *jaw_obj = JAW_OBJECT(table);
	TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
	jobject atk_table = data->atk_table;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkTable = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkTable");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkTable, "is_selected", "(II)Z");
	jboolean jselected = (*jniEnv)->CallBooleanMethod(jniEnv, atk_table, jmid, (jint)row, (jint)column);

	if (jselected == JNI_TRUE) {
		return TRUE;
	}

	return FALSE;
}

