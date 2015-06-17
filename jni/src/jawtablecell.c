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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <atk/atk.h>
#include <glib.h>
#include "jawimpl.h"
#include "jawutil.h"

extern void jaw_table_cell_interface_init (AtkTableCellIface*);
extern gpointer jaw_table_cell_data_init (jobject ac);
extern void jaw_table_cell_data_finalize (gpointer);

static AtkObject *jaw_table_cell_get_table (AtkTableCell *cell);

typedef struct _TableCellData {
  jobject atk_table_cell;
  gchar* description;
  jstring jstrDescription;
} TableCellData;

void
jaw_table_cell_interface_init (AtkTableCellIface *iface)
{
  iface->get_table = jaw_table_cell_get_table;
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
                                          "getTableCell",
                                          "(II)Ljavax/accessibility/AccessibleContext;");
  jobject jac = (*jniEnv)->CallObjectMethod(jniEnv, jatk_table_cell, jmid);

  if (!jac)
    return NULL;

  JawImpl* jaw_impl = jaw_impl_get_instance(jniEnv, jac);

  return ATK_OBJECT(jaw_impl);
}
