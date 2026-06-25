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

#include "jawimpl.h"
#include "jawutil.h"
#include <atk/atk.h>
#include <glib.h>

/**
 * (From Atk documentation)
 *
 * AtkTableCell:
 *
 * The ATK interface implemented for a cell inside a two-dimentional #AtkTable
 *
 * Being #AtkTable a component which present elements ordered via rows
 * and columns, an #AtkTableCell is the interface which each of those
 * elements, so "cells" should implement.
 *
 * See [iface@AtkTable]
 */

static AtkObject *jaw_table_cell_get_table (AtkTableCell *cell);
static GPtrArray *jaw_table_cell_get_column_header_cells (AtkTableCell *cell);
static gboolean jaw_table_cell_get_position (AtkTableCell *cell, gint *row, gint *column);
static gboolean jaw_table_cell_get_row_column_span (AtkTableCell *cell,
                                                    gint *row,
                                                    gint *column,
                                                    gint *row_span,
                                                    gint *column_span);
static gint jaw_table_cell_get_row_span (AtkTableCell *cell);
static GPtrArray *jaw_table_cell_get_row_header_cells (AtkTableCell *cell);
static gint jaw_table_cell_get_column_span (AtkTableCell *cell);

typedef struct _TableCellData
{
  jobject atk_table_cell;
  gchar *description;
  jstring jstrDescription;
} TableCellData;

#define JAW_GET_TABLECELL(cell, def_ret) \
  JAW_GET_OBJ_IFACE (cell, INTERFACE_TABLE_CELL, TableCellData, atk_table_cell, jniEnv, jatk_table_cell, def_ret)

/**
 * AtkTableCellIface:
 * @get_column_span: virtual function that returns the number of
 *   columns occupied by this cell accessible
 * @get_column_header_cells: virtual function that returns the column
 *   headers as an array of cell accessibles
 * @get_position: virtual function that retrieves the tabular position
 *   of this cell
 * @get_row_span: virtual function that returns the number of rows
 *   occupied by this cell
 * @get_row_header_cells: virtual function that returns the row
 *   headers as an array of cell accessibles
 * @get_row_column_span: virtual function that get the row an column
 *   indexes and span of this cell
 * @get_table: virtual function that returns a reference to the
 *   accessible of the containing table
 *
 * AtkTableCell is an interface for cells inside an #AtkTable.
 *
 * Since: 2.12
 */

void
jaw_table_cell_interface_init (AtkTableCellIface *iface, gpointer data)
{
  JAW_DEBUG_ALL ("%p, %p", iface, data);
  iface->get_column_span = jaw_table_cell_get_column_span;
  iface->get_column_header_cells = jaw_table_cell_get_column_header_cells;
  iface->get_position = jaw_table_cell_get_position;
  iface->get_row_span = jaw_table_cell_get_row_span;
  iface->get_row_header_cells = jaw_table_cell_get_row_header_cells;
  iface->get_row_column_span = jaw_table_cell_get_row_column_span;
  iface->get_table = jaw_table_cell_get_table;
}

/**
 * jaw_table_cell_data_init:
 * @ac: a Java AccessibleContext object
 *
 * Initializes the table cell interface data for an accessible object.
 * Creates and returns a TableCellData structure containing a global reference
 * to the Java AtkTableCell object.
 *
 * Explicitly manages a JNI local reference frame using
 * PushLocalFrame/PopLocalFrame; all local references are released
 * before the function returns.
 *
 * Returns: (nullable): pointer to TableCellData or NULL on failure
 **/

gpointer
jaw_table_cell_data_init (jobject ac)
{
  JAW_DEBUG_ALL ("%p", ac);
  TableCellData *data = g_new0 (TableCellData, 1);

  JNIEnv *jniEnv = jaw_util_get_jni_env ();
  jclass classTableCell = (*jniEnv)->FindClass (jniEnv, "org/GNOME/Accessibility/AtkTableCell");
  jmethodID jmid = (*jniEnv)->GetStaticMethodID (jniEnv, classTableCell, "createAtkTableCell", "(Ljavax/accessibility/AccessibleContext;)Lorg/GNOME/Accessibility/AtkTableCell;");
  jobject jatk_table_cell = (*jniEnv)->CallStaticObjectMethod (jniEnv, classTableCell, jmid, ac);
  data->atk_table_cell = (*jniEnv)->NewGlobalRef (jniEnv, jatk_table_cell);

  return data;
}

/**
 * jaw_table_cell_data_finalize:
 * @p: TableCellData pointer to finalize
 *
 * Cleans up TableCellData when the parent GObject is finalized.
 * Called from jaw_impl_finalize() when the object's reference count reaches
 * zero.
 */

void
jaw_table_cell_data_finalize (gpointer p)
{
  JAW_DEBUG_ALL ("%p", p);
  TableCellData *data = (TableCellData *) p;
  JNIEnv *jniEnv = jaw_util_get_jni_env ();

  if (data && data->atk_table_cell)
    {
      if (data->description != NULL)
        {
          (*jniEnv)->ReleaseStringUTFChars (jniEnv, data->jstrDescription, data->description);
          (*jniEnv)->DeleteGlobalRef (jniEnv, data->jstrDescription);
          data->jstrDescription = NULL;
          data->description = NULL;
        }

      (*jniEnv)->DeleteGlobalRef (jniEnv, data->atk_table_cell);
      data->atk_table_cell = NULL;
    }
}

/**
 * jaw_table_cell_get_table:
 * @cell: a GObject instance that implements AtkTableCellIface
 *
 * Returns a reference to the accessible of the containing table.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 *
 * Returns: (transfer full): the atk object for the containing table.
 */

static AtkObject *
jaw_table_cell_get_table (AtkTableCell *cell)
{
  JAW_DEBUG_C ("%p", cell);
  JAW_GET_TABLECELL (cell, NULL);

  jclass classAtkTableCell = (*jniEnv)->FindClass (jniEnv,
                                                   "org/GNOME/Accessibility/AtkTableCell");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkTableCell,
                                           "getTable",
                                           "()Ljavax/accessibility/AccessibleTable;");
  jobject jac = (*jniEnv)->CallObjectMethod (jniEnv, jatk_table_cell, jmid);
  (*jniEnv)->DeleteGlobalRef (jniEnv, jatk_table_cell);

  if (!jac)
    return NULL;

  JawImpl *jaw_impl = jaw_impl_get_instance_from_jaw (jniEnv, jac);
  /* FIXME: atk_table_cell_get_table is documented to return with transfer full,
   * but it doesn't seem so??
   * see https://gitlab.gnome.org/GNOME/at-spi2-core/-/issues/207 */

  return ATK_OBJECT (jaw_impl);
}

/**
 * getPosition:
 * @jniEnv: JNI environment pointer.
 * @jatk_table_cell: a Java object implementing the AtkTableCell interface.
 * @row: (out): return location for the zero-based row index of the cell.
 * @column: (out): return location for the zero-based column index of the cell.
 *
 * Retrieves the row and column index of the cell.
 * If the operation succeeds, the values of @row and @column are updated.
 * If it fails, the output arguments are left unchanged.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 *
 * Returns: %TRUE if successful; %FALSE otherwise.
 */

static void
getPosition (JNIEnv *jniEnv, jobject jatk_table_cell, jclass classAtkTableCell, gint *row, gint *column)
{
  jfieldID id_row = (*jniEnv)->GetFieldID (jniEnv, classAtkTableCell, "row", "I");
  jfieldID id_column = (*jniEnv)->GetFieldID (jniEnv, classAtkTableCell, "column", "I");
  jint jrow = (*jniEnv)->GetIntField (jniEnv, jatk_table_cell, id_row);
  jint jcolumn = (*jniEnv)->GetIntField (jniEnv, jatk_table_cell, id_column);
  (*row) = (gint) jrow;
  (*column) = (gint) jcolumn;
}

/**
 * jaw_table_cell_get_position:
 * @cell: an #AtkTableCell.
 * @row: (out): the row of the given cell.
 * @column: (out): the column of the given cell.
 *
 * Retrieves the tabular position of this cell.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 *
 * Returns: %TRUE if successful; %FALSE otherwise.
 *
 * Since: 2.12
 */

static gboolean
jaw_table_cell_get_position (AtkTableCell *cell, gint *row, gint *column)
{
  JAW_DEBUG_C ("%p, %p, %p", cell, row, column);
  JAW_GET_TABLECELL (cell, FALSE);

  jclass classAtkTableCell = (*jniEnv)->FindClass (jniEnv, "org/GNOME/Accessibility/AtkTableCell");
  getPosition (jniEnv, jatk_table_cell, classAtkTableCell, row, column);
  (*jniEnv)->DeleteGlobalRef (jniEnv, jatk_table_cell);
  return TRUE;
}

/**
 * getRowSpan:
 * @jniEnv: JNI environment pointer.
 * @jatk_table_cell: a Java object implementing the AtkTableCell interface.
 * @classAtkTableCell: the Java class of @jatk_table_cell.
 * @row_span: (out): return location for the row-span value.
 *
 * Retrieves the `rowSpan` field from a Java ATK table cell object.
 * On success, the value is stored in @row_span.
 * On failure, @row_span is left unchanged.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 *
 * Returns: %TRUE if the value was successfully retrieved; %FALSE otherwise.
 */

static void
getRowSpan (JNIEnv *jniEnv, jobject jatk_table_cell, jclass classAtkTableCell, gint *row_span)
{
  jfieldID id_row_span = (*jniEnv)->GetFieldID (jniEnv, classAtkTableCell, "rowSpan", "I");
  jint jrow_span = (*jniEnv)->GetIntField (jniEnv, jatk_table_cell, id_row_span);
  (*row_span) = (gint) jrow_span;
}

/**
 * getColumnSpan:
 * @jniEnv: a valid JNI environment pointer.
 * @jatk_table_cell: a Java object implementing the AtkTableCell interface.
 * @classAtkTableCell: the Java class of @jatk_table_cell.
 * @column_span: (out): return location for the column-span value.
 *
 * Retrieves the `columnSpan` field from a Java ATK table cell object.
 * On success, the value is stored in @column_span.
 * On failure, @column_span is left unchanged.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 *
 * Returns: %TRUE if the column span value was successfully retrieved and
 *          stored in @column_span; %FALSE on error.
 */

static void
getColumnSpan (JNIEnv *jniEnv, jobject jatk_table_cell, jclass classAtkTableCell, gint *column_span)
{
  jfieldID id_column_span = (*jniEnv)->GetFieldID (jniEnv, classAtkTableCell, "columnSpan", "I");
  jint jcolumn_span = (*jniEnv)->GetIntField (jniEnv, jatk_table_cell, id_column_span);
  (*column_span) = (gint) jcolumn_span;
}

/**
 * jaw_table_cell_get_row_column_span:
 * @cell: an #AtkTableCell.
 * @row: (out): the row index of the given cell.
 * @column: (out): the column index of the given cell.
 * @row_span: (out): the number of rows occupied by this cell.
 * @column_span: (out): the number of columns occupied by this cell.
 *
 * Gets the row and column indexes and span of this cell accessible.
 *
 * Note: Even if the function returns %FALSE, some of the output arguments
 *       may have been partially updated before the failure occurred.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 *
 * Returns: %TRUE if successful; %FALSE otherwise.
 *
 * Since: 2.12
 */

static gboolean
jaw_table_cell_get_row_column_span (AtkTableCell *cell, gint *row, gint *column, gint *row_span, gint *column_span)
{
  JAW_DEBUG_C ("%p, %p, %p, %p, %p", cell, row, column, row_span, column_span);
  JAW_GET_TABLECELL (cell, FALSE);

  jclass classAtkTableCell = (*jniEnv)->FindClass (jniEnv, "org/GNOME/Accessibility/AtkTableCell");
  getPosition (jniEnv, jatk_table_cell, classAtkTableCell, row, column);
  getRowSpan (jniEnv, jatk_table_cell, classAtkTableCell, row_span);
  getColumnSpan (jniEnv, jatk_table_cell, classAtkTableCell, column_span);
  (*jniEnv)->DeleteGlobalRef (jniEnv, jatk_table_cell);
  return TRUE;
}

/**
 * jaw_table_cell_get_row_span:
 * @cell: (nullable): an #AtkTableCell instance
 *
 * Returns the number of rows occupied by this cell accessible.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 *
 * Returns: (type gint):
 *     A gint representing the number of rows occupied by this cell, or 0 if the
 * cell does not implement this method.
 *
 * Since: 2.12
 */

static gint
jaw_table_cell_get_row_span (AtkTableCell *cell)
{
  JAW_DEBUG_C ("%p", cell);
  JAW_GET_TABLECELL (cell, 0);

  gint row_span = -1;
  jclass classAtkTableCell = (*jniEnv)->FindClass (jniEnv, "org/GNOME/Accessibility/AtkTableCell");
  getRowSpan (jniEnv, jatk_table_cell, classAtkTableCell, &row_span);
  (*jniEnv)->DeleteGlobalRef (jniEnv, jatk_table_cell);
  return row_span;
}

/**
 * jaw_table_cell_get_column_span:
 * @cell: (nullable): an #AtkTableCell instance
 *
 * Returns the number of columns occupied by this table cell.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 *
 * Returns: (type gint):
 *     A gint representing the number of columns occupied by this cell,
 *     or 0 if the cell does not implement this method.
 *
 * Since: 2.12
 */

static gint
jaw_table_cell_get_column_span (AtkTableCell *cell)
{
  JAW_DEBUG_C ("%p", cell);
  JAW_GET_TABLECELL (cell, 0);

  gint column_span = -1;
  jclass classAtkTableCell = (*jniEnv)->FindClass (jniEnv, "org/GNOME/Accessibility/AtkTableCell");
  getColumnSpan (jniEnv, jatk_table_cell, classAtkTableCell, &column_span);
  (*jniEnv)->DeleteGlobalRef (jniEnv, jatk_table_cell);
  return column_span;
}

/**
 * jaw_table_cell_get_column_header_cells:
 * @cell: a GObject instance that implements AtkTableCellIface
 *
 * Returns the column headers as an array of cell accessibles.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 *
 * Returns: (element-type AtkObject) (transfer full): a GPtrArray of AtkObjects
 * representing the column header cells.
 */

static GPtrArray *
jaw_table_cell_get_column_header_cells (AtkTableCell *cell)
{
  JAW_DEBUG_C ("%p", cell);
  JAW_GET_TABLECELL (cell, NULL);

  jclass classAtkTableCell = (*jniEnv)->FindClass (jniEnv, "org/GNOME/Accessibility/AtkTableCell");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv, classAtkTableCell, "getAccessibleColumnHeader", "()[Ljavax/accessibility/AccessibleContext;");
  jobjectArray ja_ac = (jobjectArray) (*jniEnv)->CallObjectMethod (jniEnv, jatk_table_cell, jmid);
  (*jniEnv)->DeleteGlobalRef (jniEnv, jatk_table_cell);
  if (!ja_ac)
    return NULL;
  jsize length = (*jniEnv)->GetArrayLength (jniEnv, ja_ac);
  GPtrArray *result = g_ptr_array_sized_new ((guint) length);
  for (int i = 0; i < length; i++)
    {
      jobject jac = (*jniEnv)->GetObjectArrayElement (jniEnv, ja_ac, i);
      JawImpl *jaw_impl = jaw_impl_get_instance_from_jaw (jniEnv, jac);
      g_ptr_array_add (result, jaw_impl);
    }
  return result;
}

/**
 * jaw_table_cell_get_row_header_cells:
 * @cell: a GObject instance that implements AtkTableCellIface
 *
 * Returns the row headers as an array of cell accessibles.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 *
 * Returns: (element-type AtkObject) (transfer full): a GPtrArray of AtkObjects
 * representing the row header cells.
 */

static GPtrArray *
jaw_table_cell_get_row_header_cells (AtkTableCell *cell)
{
  JAW_DEBUG_C ("%p", cell);
  JAW_GET_TABLECELL (cell, NULL);

  jclass classAtkTableCell = (*jniEnv)->FindClass (jniEnv, "org/GNOME/Accessibility/AtkTableCell");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv, classAtkTableCell, "getAccessibleRowHeader", "()[Ljavax/accessibility/AccessibleContext;");
  jobjectArray ja_ac = (jobjectArray) (*jniEnv)->CallObjectMethod (jniEnv, jatk_table_cell, jmid);
  (*jniEnv)->DeleteGlobalRef (jniEnv, jatk_table_cell);
  if (!ja_ac)
    return NULL;
  jsize length = (*jniEnv)->GetArrayLength (jniEnv, ja_ac);
  GPtrArray *result = g_ptr_array_sized_new ((guint) length);
  for (int i = 0; i < length; i++)
    {
      jobject jac = (*jniEnv)->GetObjectArrayElement (jniEnv, ja_ac, i);
      JawImpl *jaw_impl = jaw_impl_get_instance_from_jaw (jniEnv, jac);
      g_ptr_array_add (result, jaw_impl);
    }
  return result;
}
