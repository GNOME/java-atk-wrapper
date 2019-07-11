/*
 * Java ATK Wrapper for GNOME
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  021101301  USA
 */

#include <atk/atk.h>
#include <glib.h>
#include "jawimpl.h"
#include "jawutil.h"

static AtkObject *jaw_table_cell_get_table (AtkTableCell *cell);
static gboolean jaw_table_cell_get_position(AtkTableCell *cell, gint *row, gint *column);
static gboolean jaw_table_cell_get_row_column_span(AtkTableCell *cell,
                                                   gint         *row,
                                                   gint         *column,
                                                   gint         *row_span,
                                                   gint         *column_span);
static gint jaw_table_cell_get_row_span(AtkTableCell *cell);
static gint jaw_table_cell_get_column_span(AtkTableCell *cell);

typedef struct _TableCellData {
  jobject atk_table_cell;
  gchar* description;
  jstring jstrDescription;
} TableCellData;

void
jaw_table_cell_interface_init (AtkTableCellIface *iface, gpointer data)
{
    JAW_DEBUG_ALL("%p, %p", iface, data);
  iface->get_table = jaw_table_cell_get_table;
  iface->get_position = jaw_table_cell_get_position;
  iface->get_row_column_span = jaw_table_cell_get_row_column_span;
  iface->get_row_span = jaw_table_cell_get_row_span;
  iface->get_column_span = jaw_table_cell_get_column_span;
}

gpointer
jaw_table_cell_data_init (jobject ac)
{
    JAW_DEBUG_ALL("%p", ac);
  TableCellData *data = g_new0(TableCellData, 1);

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classTableCell = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkTableCell");
  jmethodID jmid = (*jniEnv)->GetStaticMethodID(jniEnv, classTableCell, "createAtkTableCell", "(Ljavax/accessibility/AccessibleContext;)Lorg/GNOME/Accessibility/AtkTableCell;");
  jobject jatk_table_cell = (*jniEnv)->CallStaticObjectMethod(jniEnv, classTableCell, jmid, ac);
  data->atk_table_cell = (*jniEnv)->NewWeakGlobalRef(jniEnv, jatk_table_cell);

  return data;
}

void
jaw_table_cell_data_finalize (gpointer p)
{
    JAW_DEBUG_ALL("%p", p);
  TableCellData *data = (TableCellData*)p;
  JNIEnv *jniEnv = jaw_util_get_jni_env();

  if (data && data->atk_table_cell)
  {
    if (data->description != NULL)
    {
      (*jniEnv)->ReleaseStringUTFChars(jniEnv, data->jstrDescription, data->description);
      (*jniEnv)->DeleteGlobalRef(jniEnv, data->jstrDescription);
      data->jstrDescription = NULL;
      data->description = NULL;
    }

    (*jniEnv)->DeleteWeakGlobalRef(jniEnv, data->atk_table_cell);
    data->atk_table_cell = NULL;
  }
}

static AtkObject*
jaw_table_cell_get_table(AtkTableCell *cell)
{
    JAW_DEBUG_C("%p", cell);
  JawObject *jaw_obj = JAW_OBJECT(cell);
  TableCellData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE_CELL);
  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jobject jatk_table_cell = (*jniEnv)->NewGlobalRef(jniEnv, data->atk_table_cell);
  if (!jatk_table_cell) {
    return NULL;
  }

  jclass classAtkTableCell = (*jniEnv)->FindClass(jniEnv,
                                                  "org/GNOME/Accessibility/AtkTableCell");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkTableCell,
                                          "getTable",
                                          "()Ljavax/accessibility/AccessibleTable;");
  jobject jac = (*jniEnv)->CallObjectMethod(jniEnv, jatk_table_cell, jmid);
  (*jniEnv)->DeleteGlobalRef(jniEnv, jatk_table_cell);

  if (!jac)
    return NULL;

  JawImpl* jaw_impl = jaw_impl_get_instance_from_jaw(jniEnv, jac);

  return ATK_OBJECT(jaw_impl);
}

static gboolean
jaw_table_cell_get_position(AtkTableCell *cell, gint *row, gint *column)
{
    JAW_DEBUG_C("%p, %d, %d", cell, row, column);
  JawObject *jaw_obj = JAW_OBJECT(cell);
  TableCellData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE_CELL);
  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jobject jatk_table_cell = (*jniEnv)->NewGlobalRef(jniEnv, data->atk_table_cell);
  if (!jatk_table_cell) {
    return FALSE;
  }

  jclass classAtkTableCell = (*jniEnv)->FindClass(jniEnv,
                                                  "org/GNOME/Accessibility/AtkTableCell");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkTableCell,
                                          "getPosition",
                                          "(II)Z;");
 jboolean jposition = (*jniEnv)->CallBooleanMethod(jniEnv,
                                                   jatk_table_cell,
                                                   jmid,
                                                   (jint)GPOINTER_TO_INT(row),
                                                   (jint)GPOINTER_TO_INT(column));
  (*jniEnv)->DeleteGlobalRef(jniEnv, jatk_table_cell);

  if (jposition == JNI_TRUE)
    return TRUE;

  return FALSE;
}

static gboolean jaw_table_cell_get_row_column_span(AtkTableCell *cell,
                                                   gint         *row,
                                                   gint         *column,
                                                   gint         *row_span,
                                                   gint         *column_span)
{
    JAW_DEBUG_C("%p, %d, %d, %d, %d", cell, row, column, row_span, column_span);
  JawObject *jaw_obj = JAW_OBJECT(cell);
  TableCellData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE_CELL);
  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jobject jatk_table_cell = (*jniEnv)->NewGlobalRef(jniEnv, data->atk_table_cell);
  if (!jatk_table_cell) {
    return FALSE;
  }

  jclass classAtkTableCell = (*jniEnv)->FindClass(jniEnv,
                                                  "org/GNOME/Accessibility/AtkTableCell");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkTableCell,
                                          "getRowColumnSpan",
                                          "(IIII)Z;");
  jboolean jspan = (*jniEnv)->CallBooleanMethod(jniEnv,
                                                jatk_table_cell,
                                                jmid,
                                                (jint)GPOINTER_TO_INT(row),
                                                (jint)GPOINTER_TO_INT(column),
                                                (jint)GPOINTER_TO_INT(row_span),
                                                (jint)GPOINTER_TO_INT(column_span)
                                                );
  (*jniEnv)->DeleteGlobalRef(jniEnv, jatk_table_cell);
  if (jspan == JNI_TRUE)
    return TRUE;

  return FALSE;
}

static gint
jaw_table_cell_get_row_span(AtkTableCell *cell)
{
    JAW_DEBUG_C("%p", cell);
  JawObject *jaw_obj = JAW_OBJECT(cell);
  TableCellData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE_CELL);
  JNIEnv *env = jaw_util_get_jni_env();
  jobject jatk_table_cell = (*env)->NewGlobalRef(env, data->atk_table_cell);
  if (!jatk_table_cell) {
    return 0;
  }

  jclass classAtkTableCell = (*env)->FindClass(env,
                                               "org/GNOME/Accessibility/AtkTableCell");
  jmethodID jmid = (*env)->GetMethodID(env,
                                       classAtkTableCell,
                                       "getRowSpan",
                                       "()I;");
  gint ret = (gint) (*env)->CallIntMethod(env, jatk_table_cell, jmid);
  (*env)->DeleteGlobalRef(env, jatk_table_cell);
  return ret;
}

static gint
jaw_table_cell_get_column_span(AtkTableCell *cell)
{
    JAW_DEBUG_C("%p", cell);
  JawObject *jaw_obj = JAW_OBJECT(cell);
  TableCellData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE_CELL);
  JNIEnv *env = jaw_util_get_jni_env();
  jobject jatk_table_cell = (*env)->NewGlobalRef(env, data->atk_table_cell);
  if (!jatk_table_cell) {
    return 0;
  }

  jclass classAtkTableCell = (*env)->FindClass(env,
                                               "org/GNOME/Accessibility/AtkTableCell");
  jmethodID jmid = (*env)->GetMethodID(env,
                                       classAtkTableCell,
                                       "getColumnSpan",
                                       "()I;");
  gint ret = (gint) (*env)->CallIntMethod(env, jatk_table_cell, jmid);
  (*env)->DeleteGlobalRef(env, jatk_table_cell);
  return ret;
}
