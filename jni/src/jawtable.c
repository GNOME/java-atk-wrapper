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

extern void jaw_table_interface_init (AtkTableIface*);
extern gpointer jaw_table_data_init (jobject ac);
extern void jaw_table_data_finalize (gpointer);

static AtkObject*   jaw_table_ref_at(AtkTable *table, gint row, gint column);
static gint         jaw_table_get_column_at_index(AtkTable *table, gint index);
static gint         jaw_table_get_row_at_index(AtkTable *table, gint index);
static gint         jaw_table_get_n_columns(AtkTable *table);
static gint         jaw_table_get_n_rows(AtkTable *table);
static gint         jaw_table_get_column_extent_at(AtkTable	*table, gint row, gint column);
static gint         jaw_table_get_row_extent_at(AtkTable *table, gint row, gint column);
static AtkObject*   jaw_table_get_caption(AtkTable *table);
static const gchar* jaw_table_get_column_description(AtkTable *table, gint column);
static const gchar* jaw_table_get_row_description(AtkTable *table, gint row);
static AtkObject*   jaw_table_get_column_header(AtkTable *table, gint column);
static AtkObject*   jaw_table_get_row_header(AtkTable *table, gint row);
static AtkObject*   jaw_table_get_summary(AtkTable *table);
static gint         jaw_table_get_selected_columns(AtkTable *table, gint **selected);
static gint         jaw_table_get_selected_rows(AtkTable *table, gint **selected);
static gboolean     jaw_table_is_column_selected(AtkTable *table, gint column);
static gboolean     jaw_table_is_row_selected(AtkTable *table, gint row);
static gboolean     jaw_table_is_selected(AtkTable *table,gint row, gint column);
static gboolean     jaw_table_add_row_selection(AtkTable *table, gint row);
static gboolean     jaw_table_add_column_selection(AtkTable *table, gint column);
static void         jaw_table_set_row_description(AtkTable *table,
                                                  gint row,
                                                  const gchar *description);
static void         jaw_table_set_column_description(AtkTable *table,
                                                     gint column,
                                                     const gchar *description);
static void         jaw_table_set_row_header(AtkTable *table, gint row, AtkObject *header);
static void         jaw_table_set_column_header(AtkTable *table, gint column, AtkObject *header);
static void         jaw_table_set_caption(AtkTable *table, AtkObject *caption);
static void         jaw_table_set_summary(AtkTable *table, AtkObject *summary);

typedef struct _TableData {
  jobject atk_table;
  gchar* description;
  jstring jstrDescription;
} TableData;

void
jaw_table_interface_init (AtkTableIface *iface)
{
  iface->ref_at = jaw_table_ref_at;
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
  iface->add_row_selection = jaw_table_add_row_selection;
  iface->add_column_selection = jaw_table_add_column_selection;
  iface->set_row_description = jaw_table_set_row_description;
  iface->set_column_description = jaw_table_set_column_description;
  iface->set_row_header = jaw_table_set_row_header;
  iface->set_column_header = jaw_table_set_column_header;
  iface->set_caption = jaw_table_set_caption;
  iface->set_summary = jaw_table_set_summary;
}

gpointer
jaw_table_data_init (jobject ac)
{
  TableData *data = g_new0(TableData, 1);

  JNIEnv *env = jaw_util_get_jni_env();
  jclass classTable = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID(env,
                                       classTable,
                                       "<init>",
                                       "(Ljavax/accessibility/AccessibleContext;)V");

  jobject jatk_table = (*env)->NewObject(env, classTable, jmid, ac);
  data->atk_table = (*env)->NewGlobalRef(env, jatk_table);

  return data;
}

void
jaw_table_data_finalize (gpointer p)
{
  TableData *data = (TableData*)p;
  JNIEnv *env = jaw_util_get_jni_env();

  if (data && data->atk_table)
  {
    if (data->description != NULL)
    {
      (*env)->ReleaseStringUTFChars(env, data->jstrDescription, data->description);
      (*env)->DeleteGlobalRef(env, data->jstrDescription);
      data->jstrDescription = NULL;
      data->description = NULL;
    }

    (*env)->DeleteGlobalRef(env, data->atk_table);
    data->atk_table = NULL;
  }
}

static AtkObject*
jaw_table_ref_at (AtkTable *table, gint	row, gint column)
{
  JawObject *jaw_obj = JAW_OBJECT(table);
  TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
  jobject atk_table = data->atk_table;

  JNIEnv *env = jaw_util_get_jni_env();
  jclass classAtkTable = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID(env,
                                       classAtkTable,
                                       "ref_at",
                                       "(II)Ljavax/accessibility/AccessibleContext;");
  jobject jac = (*env)->CallObjectMethod(env, atk_table, jmid, (jint)row, (jint)column);

  if (!jac)
    return NULL;

  JawImpl* jaw_impl = jaw_impl_get_instance( env, jac );

  if (G_OBJECT(jaw_impl) != NULL)
    g_object_ref(G_OBJECT(jaw_impl));

  return ATK_OBJECT(jaw_impl);
}

static gint
jaw_table_get_column_at_index (AtkTable *table, gint index)
{
  JawObject *jaw_obj = JAW_OBJECT(table);
  TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
  jobject atk_table = data->atk_table;

  JNIEnv *env = jaw_util_get_jni_env();
  jclass classAtkTable = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID(env, classAtkTable, "get_column_at_index", "(I)I");
  jint jcolumn = (*env)->CallIntMethod(env, atk_table, jmid, (jint)index);

  return (gint)jcolumn;
}

static gint
jaw_table_get_row_at_index (AtkTable *table, gint index)
{
  JawObject *jaw_obj = JAW_OBJECT(table);
  TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
  jobject atk_table = data->atk_table;

  JNIEnv *env = jaw_util_get_jni_env();
  jclass classAtkTable = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID(env, classAtkTable, "get_row_at_index", "(I)I");
  jint jrow = (*env)->CallIntMethod(env, atk_table, jmid, (jint)index);

  return (gint)jrow;
}

static gint
jaw_table_get_n_columns	(AtkTable *table)
{
  JawObject *jaw_obj = JAW_OBJECT(table);
  TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
  jobject atk_table = data->atk_table;

  JNIEnv *env = jaw_util_get_jni_env();
  jclass classAtkTable = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID(env, classAtkTable, "get_n_columns", "()I");
  jint jcolumns = (*env)->CallIntMethod(env, atk_table, jmid);

  return (gint)jcolumns;
}

static gint
jaw_table_get_n_rows (AtkTable *table)
{
  JawObject *jaw_obj = JAW_OBJECT(table);
  TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
  jobject atk_table = data->atk_table;

  JNIEnv *env = jaw_util_get_jni_env();
  jclass classAtkTable = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID(env, classAtkTable, "get_n_rows", "()I");
  jint jrows = (*env)->CallIntMethod(env, atk_table, jmid);

  return (gint)jrows;
}

static gint
jaw_table_get_column_extent_at (AtkTable *table, gint row, gint	column)
{
  JawObject *jaw_obj = JAW_OBJECT(table);
  TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
  jobject atk_table = data->atk_table;

  JNIEnv *env = jaw_util_get_jni_env();
  jclass classAtkTable = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID(env, classAtkTable, "get_column_extent_at", "(II)I");
  jint jextent = (*env)->CallIntMethod(env, atk_table, jmid, (jint)row, (jint)column);

  return (gint)jextent;
}

static gint
jaw_table_get_row_extent_at (AtkTable *table, gint row, gint column)
{
  JawObject *jaw_obj = JAW_OBJECT(table);
  TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
  jobject atk_table = data->atk_table;

  JNIEnv *env = jaw_util_get_jni_env();
  jclass classAtkTable = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID(env, classAtkTable, "get_row_extent_at", "(II)I");
  jint jextent = (*env)->CallIntMethod(env, atk_table, jmid, (jint)row, (jint)column);

  return (gint)jextent;
}

static AtkObject*
jaw_table_get_caption (AtkTable	*table)
{
  JawObject *jaw_obj = JAW_OBJECT(table);
  TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
  jobject atk_table = data->atk_table;

  JNIEnv *env = jaw_util_get_jni_env();
  jclass classAtkTable = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID(env,
                                       classAtkTable,
                                       "get_caption",
                                       "()Ljavax/accessibility/AccessibleContext;");

  jobject jac = (*env)->CallObjectMethod(env, atk_table, jmid);

  if (!jac)
    return NULL;

  JawImpl* jaw_impl = jaw_impl_get_instance( env, jac );

  return ATK_OBJECT(jaw_impl);
}

static const gchar*
jaw_table_get_column_description (AtkTable *table, gint	column)
{
  JawObject *jaw_obj = JAW_OBJECT(table);
  TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
  jobject atk_table = data->atk_table;

  JNIEnv *env = jaw_util_get_jni_env();
  jclass classAtkTable = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID(env, classAtkTable, "get_column_description", "(I)Ljava/lang/String;");
  jstring jstr = (*env)->CallObjectMethod(env, atk_table, jmid, (jint)column);

  if (data->description != NULL)
  {
    (*env)->ReleaseStringUTFChars(env, data->jstrDescription, data->description);
    (*env)->DeleteGlobalRef(env, data->jstrDescription);
  }

  data->jstrDescription = (*env)->NewGlobalRef(env, jstr);
  data->description = (gchar*)(*env)->GetStringUTFChars(env, data->jstrDescription, NULL);

  return data->description;
}

static const gchar*
jaw_table_get_row_description (AtkTable *table, gint row)
{
  JawObject *jaw_obj = JAW_OBJECT(table);
  TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
  jobject atk_table = data->atk_table;

  JNIEnv *env = jaw_util_get_jni_env();
  jclass classAtkTable = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID(env, classAtkTable, "get_row_description", "(I)Ljava/lang/String;");
  jstring jstr = (*env)->CallObjectMethod(env, atk_table, jmid, (jint)row);

  if (data->description != NULL)
  {
    (*env)->ReleaseStringUTFChars(env, data->jstrDescription, data->description);
    (*env)->DeleteGlobalRef(env, data->jstrDescription);
  }

  data->jstrDescription = (*env)->NewGlobalRef(env, jstr);
  data->description = (gchar*)(*env)->GetStringUTFChars(env, data->jstrDescription, NULL);

  return data->description;
}

static AtkObject*
jaw_table_get_column_header (AtkTable *table, gint column)
{
  JawObject *jaw_obj = JAW_OBJECT(table);
  TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
  jobject atk_table = data->atk_table;

  JNIEnv *env = jaw_util_get_jni_env();
  jclass classAtkTable = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID(env, classAtkTable, "get_column_header", "(I)Ljavax/accessibility/AccessibleContext;");
  jobject jac = (*env)->CallObjectMethod(env, atk_table, jmid, (jint)column);

  if (!jac)
    return NULL;

  JawImpl* jaw_impl = jaw_impl_get_instance( env, jac );

  return ATK_OBJECT(jaw_impl);
}

static AtkObject*
jaw_table_get_row_header (AtkTable *table, gint row)
{
  JawObject *jaw_obj = JAW_OBJECT(table);
  TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
  jobject atk_table = data->atk_table;

  JNIEnv *env = jaw_util_get_jni_env();
  jclass classAtkTable = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID(env, classAtkTable, "get_row_header", "(I)Ljavax/accessibility/AccessibleContext;");
  jobject jac = (*env)->CallObjectMethod(env, atk_table, jmid, (jint)row);

  if (!jac)
    return NULL;

  JawImpl* jaw_impl = jaw_impl_get_instance( env, jac );

  return ATK_OBJECT(jaw_impl);
}

static AtkObject*
jaw_table_get_summary (AtkTable *table)
{
  JawObject *jaw_obj = JAW_OBJECT(table);
  TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
  jobject atk_table = data->atk_table;

  JNIEnv *env = jaw_util_get_jni_env();
  jclass classAtkTable = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID(env, classAtkTable, "get_summary", "()Ljavax/accessibility/AccessibleContext;");
  jobject jac = (*env)->CallObjectMethod(env, atk_table, jmid);

  if (!jac)
    return NULL;

  JawImpl* jaw_impl = jaw_impl_get_instance( env, jac );

  return ATK_OBJECT(jaw_impl);
}

static gint
jaw_table_get_selected_columns (AtkTable *table, gint **selected)
{
  JawObject *jaw_obj = JAW_OBJECT(table);
  TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
  jobject atk_table = data->atk_table;

  JNIEnv *env = jaw_util_get_jni_env();
  jclass classAtkTable = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID(env, classAtkTable, "get_selected_columns", "()[I");
  jintArray jcolumnArray = (*env)->CallObjectMethod(env, atk_table, jmid);

  if (!jcolumnArray)
    return 0;

  jsize length = (*env)->GetArrayLength(env, jcolumnArray);
  jint *jcolumns = (*env)->GetIntArrayElements(env, jcolumnArray, NULL);
  gint *columns = g_new(gint, length);

  gint i;
  for (i = 0; i < length; i++) {
    columns[i] = (gint)jcolumns[i];
  }

  (*env)->ReleaseIntArrayElements(env, jcolumnArray, jcolumns, JNI_ABORT);

  return (gint)length;
}

static gint
jaw_table_get_selected_rows (AtkTable *table, gint **selected)
{
  JawObject *jaw_obj = JAW_OBJECT(table);
  TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
  jobject atk_table = data->atk_table;

  JNIEnv *env = jaw_util_get_jni_env();
  jclass classAtkTable = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID(env, classAtkTable, "get_selected_rows", "()[I");
  jintArray jrowArray = (*env)->CallObjectMethod(env, atk_table, jmid);

  if (!jrowArray)
    return 0;

  jsize length = (*env)->GetArrayLength(env, jrowArray);
  jint *jrows = (*env)->GetIntArrayElements(env, jrowArray, NULL);
  gint *rows = g_new(gint, length);

  gint i;
  for (i = 0; i < length; i++) {
    rows[i] = (gint)jrows[i];
  }

  (*env)->ReleaseIntArrayElements(env, jrowArray, jrows, JNI_ABORT);

  return (gint)length;
}

static gboolean
jaw_table_is_column_selected (AtkTable *table, gint column)
{
  JawObject *jaw_obj = JAW_OBJECT(table);
  TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
  jobject atk_table = data->atk_table;

  JNIEnv *env = jaw_util_get_jni_env();
  jclass classAtkTable = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID(env, classAtkTable, "is_column_selected", "(I)Z");
  jboolean jselected = (*env)->CallBooleanMethod(env, atk_table, jmid, (jint)column);

  if (jselected == JNI_TRUE)
    return TRUE;

  return FALSE;
}

static gboolean
jaw_table_is_row_selected (AtkTable *table, gint row)
{
  JawObject *jaw_obj = JAW_OBJECT(table);
  TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
  jobject atk_table = data->atk_table;

  JNIEnv *env = jaw_util_get_jni_env();
  jclass classAtkTable = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID(env, classAtkTable, "is_row_selected", "(I)Z");
  jboolean jselected = (*env)->CallBooleanMethod(env, atk_table, jmid, (jint)row);

  if (jselected == JNI_TRUE)
    return TRUE;

  return FALSE;
}

static gboolean
jaw_table_is_selected (AtkTable *table, gint row, gint column)
{
  JawObject *jaw_obj = JAW_OBJECT(table);
  TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
  jobject atk_table = data->atk_table;

  JNIEnv *env = jaw_util_get_jni_env();
  jclass classAtkTable = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID(env, classAtkTable, "is_selected", "(II)Z");
  jboolean jselected = (*env)->CallBooleanMethod(env, atk_table, jmid, (jint)row, (jint)column);

  if (jselected == JNI_TRUE)
    return TRUE;

  return FALSE;
}

static gboolean
jaw_table_add_row_selection(AtkTable *table, gint row)
{
  JawObject *jaw_obj = JAW_OBJECT(table);
  TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
  jobject atk_table = data->atk_table;

  JNIEnv *env = jaw_util_get_jni_env();
  jclass classAtkTable = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID(env, classAtkTable, "addRowSelection", "(I)Z");
  jboolean jselected = (*env)->CallBooleanMethod(env, atk_table, jmid, (jint)row);

  if (jselected == JNI_TRUE)
    return TRUE;

  return FALSE;
}

static gboolean
jaw_table_add_column_selection(AtkTable *table, gint column)
{
  JawObject *jaw_obj = JAW_OBJECT(table);
  TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
  jobject atk_table = data->atk_table;

  JNIEnv *env = jaw_util_get_jni_env();
  jclass classAtkTable = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID(env, classAtkTable, "addColumnSelection", "(I)Z");
  jboolean jselected = (*env)->CallBooleanMethod(env, atk_table, jmid, (jint)column);

  if (jselected == JNI_TRUE)
    return TRUE;

  return FALSE;
}

static void
jaw_table_set_row_description(AtkTable *table, gint row, const gchar *description)
{
  JawObject *jaw_obj = JAW_OBJECT(table);
  TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
  jobject atk_table = data->atk_table;

  JNIEnv *env = jaw_util_get_jni_env();
  jclass classAtkTable = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID(env,
                                       classAtkTable,
                                       "setRowDescription",
                                       "(ILjava/lang/String;)V");
  jstring jstr = (*env)->NewStringUTF(env, description);
  (*env)->CallVoidMethod(env, atk_table, jmid, (jint)row, jstr);
}

static void
jaw_table_set_column_description(AtkTable *table, gint column, const gchar *description)
{
  JawObject *jaw_obj = JAW_OBJECT(table);
  TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
  jobject atk_table = data->atk_table;

  JNIEnv *env = jaw_util_get_jni_env();
  jclass classAtkTable = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID(env,
                                       classAtkTable,
                                       "setColumnDescription",
                                       "(ILjava/lang/String;)V");
  jstring jstr = (*env)->NewStringUTF(env, description);
  (*env)->CallVoidMethod(env, atk_table, jmid, (jint)column, jstr);
}

static void
jaw_table_set_row_header(AtkTable *table, gint row, AtkObject *header)
{
  JawObject *jaw_obj = JAW_OBJECT(table);
  TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
  jobject atk_table = data->atk_table;

  JNIEnv *env = jaw_util_get_jni_env();
  jclass classAtkTable = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID(env,
                                       classAtkTable,
                                       "setRowHeader",
                                       "(ILjavax/accessibility/AccessibleTable;)V");
  (*env)->CallVoidMethod(env, atk_table, jmid, (jint)row, (jobject)header);
}

static void
jaw_table_set_column_header(AtkTable *table, gint column, AtkObject *header)
{
  JawObject *jaw_obj = JAW_OBJECT(table);
  TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
  jobject atk_table = data->atk_table;

  JNIEnv *env = jaw_util_get_jni_env();
  jclass classAtkTable = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID(env,
                                       classAtkTable,
                                       "setColumnHeader",
                                       "(ILjavax/accessibility/AccessibleTable;)V");
  (*env)->CallVoidMethod(env, atk_table, jmid, (jint)column, (jobject)header);
}

static void
jaw_table_set_caption(AtkTable *table, AtkObject *caption)
{
  JawObject *jaw_obj = JAW_OBJECT(table);
  TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
  jobject atk_table = data->atk_table;

  JNIEnv *env = jaw_util_get_jni_env();
  jclass classAtkTable = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID(env,
                                       classAtkTable,
                                       "setCaption",
                                       "(Ljavax/accessibility/Accessible;)V");
  (*env)->CallVoidMethod(env, atk_table, jmid, (jobject)caption);
}

static void
jaw_table_set_summary(AtkTable *table, AtkObject *summary)
{
  JawObject *jaw_obj = JAW_OBJECT(table);
  TableData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE);
  jobject atk_table = data->atk_table;

  JNIEnv *env = jaw_util_get_jni_env();
  jclass classAtkTable = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID(env,
                                       classAtkTable,
                                       "setSummary",
                                       "(Ljavax/accessibility/Accessible;)V");
  (*env)->CallVoidMethod(env, atk_table, jmid, (jobject)summary);
}
