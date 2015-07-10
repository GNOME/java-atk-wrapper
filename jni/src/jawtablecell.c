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

extern void jaw_table_cell_interface_init (AtkTableCellIface*);
extern gpointer jaw_table_cell_data_init (jobject ac);
extern void jaw_table_cell_data_finalize (gpointer);

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
jaw_table_cell_interface_init (AtkTableCellIface *iface)
{
  iface->get_table = jaw_table_cell_get_table;
  iface->get_position = jaw_table_cell_get_position;
  iface->get_row_column_span = jaw_table_cell_get_row_column_span;
  iface->get_row_span = jaw_table_cell_get_row_span;
  iface->get_column_span = jaw_table_cell_get_column_span;
}

gpointer
jaw_table_cell_data_init (jobject ac)
{
  TableCellData *data = g_new0(TableCellData, 1);

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classTableCell = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkTableCell");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classTableCell, "<init>", "(Ljavax/accessibility/AccessibleContext;)V");
  jobject jatk_table_cell = (*jniEnv)->NewObject(jniEnv, classTableCell, jmid, ac);
  data->atk_table_cell = (*jniEnv)->NewGlobalRef(jniEnv, jatk_table_cell);

  return data;
}

void
jaw_table_cell_data_finalize (gpointer p)
{
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

    (*jniEnv)->DeleteGlobalRef(jniEnv, data->atk_table_cell);
    data->atk_table_cell = NULL;
  }
}

static AtkObject*
jaw_table_cell_get_table(AtkTableCell *cell)
{
  JawObject *jaw_obj = JAW_OBJECT(cell);
  TableCellData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE_CELL);
  jobject jatk_table_cell = data->atk_table_cell;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classAtkTableCell = (*jniEnv)->FindClass(jniEnv,
                                                  "org/GNOME/Accessibility/AtkTableCell");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkTableCell,
                                          "getTable",
                                          "()Ljavax/accessibility/AccessibleTable;");
  jobject jac = (*jniEnv)->CallObjectMethod(jniEnv, jatk_table_cell, jmid);

  if (!jac)
    return NULL;

  JawImpl* jaw_impl = jaw_impl_get_instance(jniEnv, jac);

  return ATK_OBJECT(jaw_impl);
}

static gboolean
jaw_table_cell_get_position(AtkTableCell *cell, gint *row, gint *column)
{
  JawObject *jaw_obj = JAW_OBJECT(cell);
  TableCellData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE_CELL);
  jobject jatk_table_cell = data->atk_table_cell;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
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
  JawObject *jaw_obj = JAW_OBJECT(cell);
  TableCellData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE_CELL);
  jobject jatk_table_cell = data->atk_table_cell;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
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
  if (jspan == JNI_TRUE)
    return TRUE;

  return FALSE;
}

static gint
jaw_table_cell_get_row_span(AtkTableCell *cell)
{
  JawObject *jaw_obj = JAW_OBJECT(cell);
  TableCellData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE_CELL);
  jobject jatk_table_cell = data->atk_table_cell;

  JNIEnv *env = jaw_util_get_jni_env();
  jclass classAtkTableCell = (*env)->FindClass(env,
                                               "org/GNOME/Accessibility/AtkTableCell");
  jmethodID jmid = (*env)->GetMethodID(env,
                                       classAtkTableCell,
                                       "getRowSpan",
                                       "()I;");
  return (gint) (*env)->CallIntMethod(env, jatk_table_cell, jmid);
}

static gint
jaw_table_cell_get_column_span(AtkTableCell *cell)
{
  JawObject *jaw_obj = JAW_OBJECT(cell);
  TableCellData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TABLE_CELL);
  jobject jatk_table_cell = data->atk_table_cell;

  JNIEnv *env = jaw_util_get_jni_env();
  jclass classAtkTableCell = (*env)->FindClass(env,
                                               "org/GNOME/Accessibility/AtkTableCell");
  jmethodID jmid = (*env)->GetMethodID(env,
                                       classAtkTableCell,
                                       "getColumnSpan",
                                       "()I;");
  return (gint) (*env)->CallIntMethod(env, jatk_table_cell, jmid);
}

