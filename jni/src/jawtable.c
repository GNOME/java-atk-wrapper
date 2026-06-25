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

#include "jawimpl.h"
#include "jawutil.h"
#include <atk/atk.h>
#include <glib.h>

/**
 * (From Atk documentation)
 *
 * AtkTable:
 *
 * The ATK interface implemented for UI components which contain tabular or
 * row/column information.
 *
 * #AtkTable should be implemented by components which present
 * elements ordered via rows and columns.  It may also be used to
 * present tree-structured information if the nodes of the trees can
 * be said to contain multiple "columns".  Individual elements of an
 * #AtkTable are typically referred to as "cells". Those cells should
 * implement the interface #AtkTableCell, but #Atk doesn't require
 * them to be direct children of the current #AtkTable. They can be
 * grand-children, grand-grand-children etc. #AtkTable provides the
 * API needed to get a individual cell based on the row and column
 * numbers.
 *
 * Children of #AtkTable are frequently "lightweight" objects, that
 * is, they may not have backing widgets in the host UI toolkit.  They
 * are therefore often transient.
 *
 * Since tables are often very complex, #AtkTable includes provision
 * for offering simplified summary information, as well as row and
 * column headers and captions.  Headers and captions are #AtkObjects
 * which may implement other interfaces (#AtkText, #AtkImage, etc.) as
 * appropriate.  #AtkTable summaries may themselves be (simplified)
 * #AtkTables, etc.
 *
 * Note for implementors: in the past, #AtkTable required that all the
 * cells should be direct children of #AtkTable, and provided some
 * index based methods to request the cells. The practice showed that
 * that forcing made #AtkTable implementation complex, and hard to
 * expose other kind of children, like rows or captions. Right now,
 * index-based methods are deprecated.
 */

static AtkObject *jaw_table_ref_at (AtkTable *table, gint row, gint column);
static gint jaw_table_get_index_at (AtkTable *table, gint row, gint column);
static gint jaw_table_get_column_at_index (AtkTable *table, gint index);
static gint jaw_table_get_row_at_index (AtkTable *table, gint index);
static gint jaw_table_get_n_columns (AtkTable *table);
static gint jaw_table_get_n_rows (AtkTable *table);
static gint jaw_table_get_column_extent_at (AtkTable *table, gint row, gint column);
static gint jaw_table_get_row_extent_at (AtkTable *table, gint row, gint column);
static AtkObject *jaw_table_get_caption (AtkTable *table);
static const gchar *jaw_table_get_column_description (AtkTable *table, gint column);
static const gchar *jaw_table_get_row_description (AtkTable *table, gint row);
static AtkObject *jaw_table_get_column_header (AtkTable *table, gint column);
static AtkObject *jaw_table_get_row_header (AtkTable *table, gint row);
static AtkObject *jaw_table_get_summary (AtkTable *table);
static gint jaw_table_get_selected_columns (AtkTable *table, gint **selected);
static gint jaw_table_get_selected_rows (AtkTable *table, gint **selected);
static gboolean jaw_table_is_column_selected (AtkTable *table, gint column);
static gboolean jaw_table_is_row_selected (AtkTable *table, gint row);
static gboolean jaw_table_is_selected (AtkTable *table, gint row, gint column);
static gboolean jaw_table_add_row_selection (AtkTable *table, gint row);
static gboolean jaw_table_remove_row_selection (AtkTable *table, gint row);
static gboolean jaw_table_add_column_selection (AtkTable *table, gint column);
static gboolean jaw_table_remove_column_selection (AtkTable *table, gint column);
static void jaw_table_set_row_description (AtkTable *table,
                                           gint row,
                                           const gchar *description);
static void jaw_table_set_column_description (AtkTable *table,
                                              gint column,
                                              const gchar *description);
static void jaw_table_set_row_header (AtkTable *table, gint row, AtkObject *header);
static void jaw_table_set_column_header (AtkTable *table, gint column, AtkObject *header);
static void jaw_table_set_caption (AtkTable *table, AtkObject *caption);
static void jaw_table_set_summary (AtkTable *table, AtkObject *summary);

typedef struct _TableData
{
  jobject atk_table;
  gchar *description;
  jstring jstrDescription;
} TableData;

#define JAW_GET_TABLE(table, def_ret) \
  JAW_GET_OBJ_IFACE (table, INTERFACE_TABLE, TableData, atk_table, env, atk_table, def_ret)

void
jaw_table_interface_init (AtkTableIface *iface, gpointer data)
{
  JAW_DEBUG_ALL ("%p, %p", iface, data);
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
  iface->get_column_header = jaw_table_get_column_header;
  iface->get_row_description = jaw_table_get_row_description;
  iface->get_row_header = jaw_table_get_row_header;
  iface->get_summary = jaw_table_get_summary;
  iface->set_caption = jaw_table_set_caption;
  iface->set_column_description = jaw_table_set_column_description;
  iface->set_column_header = jaw_table_set_column_header;
  iface->set_row_description = jaw_table_set_row_description;
  iface->set_row_header = jaw_table_set_row_header;
  iface->set_summary = jaw_table_set_summary;
  iface->get_selected_columns = jaw_table_get_selected_columns;
  iface->get_selected_rows = jaw_table_get_selected_rows;
  iface->is_column_selected = jaw_table_is_column_selected;
  iface->is_row_selected = jaw_table_is_row_selected;
  iface->is_selected = jaw_table_is_selected;
  iface->add_row_selection = jaw_table_add_row_selection;
  iface->remove_row_selection = jaw_table_remove_row_selection;
  iface->add_column_selection = jaw_table_add_column_selection;
  iface->remove_column_selection = jaw_table_remove_column_selection;
}

/**
 * jaw_table_data_init:
 * @ac: a Java AccessibleContext object
 *
 * Initializes the table interface data for an accessible object.
 * Creates and returns a TableData structure containing a global reference
 * to the Java AtkTable object.
 *
 * Explicitly manages a JNI local reference frame using
 * PushLocalFrame/PopLocalFrame; all local references are released
 * before the function returns.
 *
 * Returns: (nullable): pointer to TableData or NULL on failure
 **/

gpointer
jaw_table_data_init (jobject ac)
{
  JAW_DEBUG_ALL ("%p", ac);
  TableData *data = g_new0 (TableData, 1);

  JNIEnv *env = jaw_util_get_jni_env ();
  jclass classTable = (*env)->FindClass (env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetStaticMethodID (env,
                                              classTable,
                                              "createAtkTable",
                                              "(Ljavax/accessibility/AccessibleContext;)Lorg/GNOME/Accessibility/AtkTable;");

  jobject jatk_table = (*env)->CallStaticObjectMethod (env, classTable, jmid, ac);
  data->atk_table = (*env)->NewGlobalRef (env, jatk_table);

  return data;
}

/**
 * jaw_table_data_finalize:
 * @p: TableData pointer to finalize
 *
 * Cleans up TableData when the parent GObject is finalized.
 * Called from jaw_impl_finalize() when the object's reference count reaches
 * zero.
 */

void
jaw_table_data_finalize (gpointer p)
{
  JAW_DEBUG_ALL ("%p", p);
  TableData *data = (TableData *) p;
  JNIEnv *env = jaw_util_get_jni_env ();

  if (data && data->atk_table)
    {
      if (data->description != NULL)
        {
          (*env)->ReleaseStringUTFChars (env, data->jstrDescription, data->description);
          (*env)->DeleteGlobalRef (env, data->jstrDescription);
          data->jstrDescription = NULL;
          data->description = NULL;
        }

      (*env)->DeleteGlobalRef (env, data->atk_table);
      data->atk_table = NULL;
    }
}

/**
 * jaw_table_ref_at:
 * @table: a GObject instance that implements AtkTableIface
 * @row: a #gint representing a row in @table
 * @column: a #gint representing a column in @table
 *
 * Get a reference to the table cell at @row, @column. This cell
 * should implement the interface #AtkTableCell
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 *
 * Returns: (transfer full): an #AtkObject representing the referred
 * to accessible
 **/

static AtkObject *
jaw_table_ref_at (AtkTable *table, gint row, gint column)
{
  JAW_DEBUG_C ("%p, %d, %d", table, row, column);
  JawObject *jaw_obj = JAW_OBJECT (table);
  if (!jaw_obj)
    {
      JAW_DEBUG_I ("jaw_obj == NULL");
      return NULL;
    }
  TableData *data = jaw_object_get_interface_data (jaw_obj, INTERFACE_TABLE);
  JNIEnv *env = jaw_util_get_jni_env ();
  jobject atk_table = (*env)->NewGlobalRef (env, data->atk_table);
  if (!atk_table)
    {
      JAW_DEBUG_I ("atk_table == NULL");
      return NULL;
    }

  jclass classAtkTable = (*env)->FindClass (env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID (env,
                                        classAtkTable,
                                        "ref_at",
                                        "(II)Ljavax/accessibility/AccessibleContext;");
  jobject jac = (*env)->CallObjectMethod (env, atk_table, jmid, (jint) row, (jint) column);
  (*env)->DeleteGlobalRef (env, atk_table);

  if (!jac)
    return NULL;

  JawImpl *jaw_impl = jaw_impl_get_instance_from_jaw (env, jac);

  if (G_OBJECT (jaw_impl) != NULL)
    g_object_ref (G_OBJECT (jaw_impl));

  return ATK_OBJECT (jaw_impl);
}

/**
 * jaw_table_get_index_at:
 * @table: a GObject instance that implements AtkTableIface
 * @row: a #gint representing a row in @table
 * @column: a #gint representing a column in @table
 *
 * Gets a #gint representing the index at the specified @row and
 * @column.
 *
 * Deprecated in atk: Since 2.12. Use atk_table_ref_at() in order to get the
 * accessible that represents the cell at (@row, @column)
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 *
 * Returns: a #gint representing the index at specified position.
 * The value -1 is returned if the object at row,column is not a child
 * of table or table does not implement this interface.
 **/

static gint
jaw_table_get_index_at (AtkTable *table, gint row, gint column)
{
  JAW_DEBUG_C ("%p, %d, %d", table, row, column);
  JAW_GET_TABLE (table, 0);

  jclass classAtkTable = (*env)->FindClass (env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID (env, classAtkTable, "get_index_at", "(II)I");
  jint jindex = (*env)->CallIntMethod (env, atk_table, jmid, (jint) row, (jint) column);
  (*env)->DeleteGlobalRef (env, atk_table);

  return (gint) jindex;
}

/**
 * jaw_table_get_column_at_index:
 * @table: a GObject instance that implements AtkTableInterface
 * @index: a #gint representing an index in @table
 *
 * Gets a #gint representing the column at the specified @index.
 *
 * Deprecated in atk: Since 2.12.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 *
 * Returns: a gint representing the column at the specified index,
 * or -1 if the table does not implement this method.
 **/

static gint
jaw_table_get_column_at_index (AtkTable *table, gint index)
{
  JAW_DEBUG_C ("%p, %d", table, index);
  JAW_GET_TABLE (table, 0);

  jclass classAtkTable = (*env)->FindClass (env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID (env, classAtkTable, "get_column_at_index", "(I)I");
  jint jcolumn = (*env)->CallIntMethod (env, atk_table, jmid, (jint) index);
  (*env)->DeleteGlobalRef (env, atk_table);

  return (gint) jcolumn;
}

/**
 * atk_table_get_row_at_index:
 * @table: a GObject instance that implements AtkTableInterface
 * @index: a #gint representing an index in @table
 *
 * Gets a #gint representing the row at the specified @index.
 *
 * Deprecated in atk: since 2.12.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 *
 * Returns: a gint representing the row at the specified index,
 * or -1 if the table does not implement this method.
 **/

static gint
jaw_table_get_row_at_index (AtkTable *table, gint index)
{
  JAW_DEBUG_C ("%p, %d", table, index);
  JAW_GET_TABLE (table, 0);

  jclass classAtkTable = (*env)->FindClass (env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID (env, classAtkTable, "get_row_at_index", "(I)I");
  jint jrow = (*env)->CallIntMethod (env, atk_table, jmid, (jint) index);
  (*env)->DeleteGlobalRef (env, atk_table);

  return (gint) jrow;
}

/**
 * atk_table_get_n_columns:
 * @table: a GObject instance that implements AtkTableIface
 *
 * Gets the number of columns in the table.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 *
 * Returns: a gint representing the number of columns, or 0
 * if value does not implement this interface.
 **/

static gint
jaw_table_get_n_columns (AtkTable *table)
{
  JAW_DEBUG_C ("%p", table);
  JAW_GET_TABLE (table, 0);

  jclass classAtkTable = (*env)->FindClass (env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID (env, classAtkTable, "get_n_columns", "()I");
  jint jcolumns = (*env)->CallIntMethod (env, atk_table, jmid);
  (*env)->DeleteGlobalRef (env, atk_table);

  return (gint) jcolumns;
}

/**
 * jaw_table_get_n_rows:
 * @table: a GObject instance that implements AtkTableIface
 *
 * Gets the number of rows in the table.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 *
 * Returns: a gint representing the number of rows, or 0
 * if value does not implement this interface.
 **/

static gint
jaw_table_get_n_rows (AtkTable *table)
{
  JAW_DEBUG_C ("%p", table);
  JAW_GET_TABLE (table, 0);

  jclass classAtkTable = (*env)->FindClass (env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID (env, classAtkTable, "get_n_rows", "()I");
  jint jrows = (*env)->CallIntMethod (env, atk_table, jmid);
  (*env)->DeleteGlobalRef (env, atk_table);

  return (gint) jrows;
}

/**
 * jaw_table_get_column_extent_at:
 * @table: a GObject instance that implements AtkTableIface
 * @row: a #gint representing a row in @table
 * @column: a #gint representing a column in @table
 *
 * Gets the number of columns occupied by the accessible object
 * at the specified @row and @column in the @table.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 *
 * Returns: a gint representing the column extent at specified position, or 0
 * if value does not implement this interface.
 **/

static gint
jaw_table_get_column_extent_at (AtkTable *table, gint row, gint column)
{
  JAW_DEBUG_C ("%p, %d, %d", table, row, column);
  JAW_GET_TABLE (table, 0);

  jclass classAtkTable = (*env)->FindClass (env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID (env, classAtkTable, "get_column_extent_at", "(II)I");
  jint jextent = (*env)->CallIntMethod (env, atk_table, jmid, (jint) row, (jint) column);
  (*env)->DeleteGlobalRef (env, atk_table);

  return (gint) jextent;
}

/**
 * jaw_table_get_row_extent_at:
 * @table: a GObject instance that implements AtkTableIface
 * @column: a #gint representing a column in @table
 *
 * Gets the description text of the specified @column in the table
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 *
 * Returns: a gint representing the row extent at specified position, or 0
 * if value does not implement this interface.
 **/

static gint
jaw_table_get_row_extent_at (AtkTable *table, gint row, gint column)
{
  JAW_DEBUG_C ("%p, %d, %d", table, row, column);
  JAW_GET_TABLE (table, 0);

  jclass classAtkTable = (*env)->FindClass (env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID (env, classAtkTable, "get_row_extent_at", "(II)I");
  jint jextent = (*env)->CallIntMethod (env, atk_table, jmid, (jint) row, (jint) column);
  (*env)->DeleteGlobalRef (env, atk_table);

  return (gint) jextent;
}

/**
 * jaw_table_get_caption:
 * @table: a GObject instance that implements AtkTableInterface
 *
 * Gets the caption for the @table.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 *
 * Returns: (nullable) (transfer none): a AtkObject* representing the
 * table caption, or %NULL if value does not implement this interface.
 **/

static AtkObject *
jaw_table_get_caption (AtkTable *table)
{
  JAW_DEBUG_C ("%p", table);
  JAW_GET_TABLE (table, NULL);

  jclass classAtkTable = (*env)->FindClass (env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID (env,
                                        classAtkTable,
                                        "get_caption",
                                        "()Ljavax/accessibility/AccessibleContext;");

  jobject jac = (*env)->CallObjectMethod (env, atk_table, jmid);
  (*env)->DeleteGlobalRef (env, atk_table);

  if (!jac)
    return NULL;

  JawImpl *jaw_impl = jaw_impl_get_instance_from_jaw (env, jac);
  /* get_caption returns with transfer: none */

  return ATK_OBJECT (jaw_impl);
}

/**
 * jaw_table_get_column_description:
 * @table: a GObject instance that implements AtkTableIface
 * @column: a #gint representing a column in @table
 *
 * Returns the description text for the specified @column of the @table.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 *
 * Returns: a const gchar representing the description text for the specified
 * column, or NULL
 **/

static const gchar *
jaw_table_get_column_description (AtkTable *table, gint column)
{
  JAW_DEBUG_C ("%p, %d", table, column);
  JAW_GET_TABLE (table, NULL);

  jclass classAtkTable = (*env)->FindClass (env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID (env, classAtkTable, "get_column_description", "(I)Ljava/lang/String;");
  jstring jstr = (*env)->CallObjectMethod (env, atk_table, jmid, (jint) column);
  (*env)->DeleteGlobalRef (env, atk_table);

  if (data->description != NULL)
    {
      (*env)->ReleaseStringUTFChars (env, data->jstrDescription, data->description);
      (*env)->DeleteGlobalRef (env, data->jstrDescription);
    }

  data->jstrDescription = (*env)->NewGlobalRef (env, jstr);
  data->description = (gchar *) (*env)->GetStringUTFChars (env, data->jstrDescription, NULL);

  return data->description;
}

/**
 * jaw_table_get_row_description:
 * @table: a GObject instance that implements AtkTableIface
 * @row: a #gint representing a row in @table
 *
 * Gets the description text of the specified row in the table
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 *
 * Returns: (nullable): a gchar* representing the row description, or
 * %NULL if value does not implement this interface.
 **/

static const gchar *
jaw_table_get_row_description (AtkTable *table, gint row)
{
  JAW_DEBUG_C ("%p, %d", table, row);
  JAW_GET_TABLE (table, NULL);

  jclass classAtkTable = (*env)->FindClass (env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID (env, classAtkTable, "get_row_description", "(I)Ljava/lang/String;");
  jstring jstr = (*env)->CallObjectMethod (env, atk_table, jmid, (jint) row);
  (*env)->DeleteGlobalRef (env, atk_table);

  if (data->description != NULL)
    {
      (*env)->ReleaseStringUTFChars (env, data->jstrDescription, data->description);
      (*env)->DeleteGlobalRef (env, data->jstrDescription);
    }

  data->jstrDescription = (*env)->NewGlobalRef (env, jstr);
  data->description = (gchar *) (*env)->GetStringUTFChars (env, data->jstrDescription, NULL);

  return data->description;
}

/**
 * jaw_table_get_column_header:
 * @table: a GObject instance that implements AtkTableIface
 * @column: a #gint representing a column in the table
 *
 * Gets the column header of a specified column in an accessible table.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 *
 * Returns: (nullable) (transfer none): a AtkObject* representing the
 * specified column header, or %NULL if value does not implement this
 * interface.
 **/

static AtkObject *
jaw_table_get_column_header (AtkTable *table, gint column)
{
  JAW_DEBUG_C ("%p, %d", table, column);
  JAW_GET_TABLE (table, NULL);

  jclass classAtkTable = (*env)->FindClass (env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID (env, classAtkTable, "get_column_header", "(I)Ljavax/accessibility/AccessibleContext;");
  jobject jac = (*env)->CallObjectMethod (env, atk_table, jmid, (jint) column);
  (*env)->DeleteGlobalRef (env, atk_table);

  if (!jac)
    return NULL;

  JawImpl *jaw_impl = jaw_impl_get_instance_from_jaw (env, jac);
  /* get_column_header returns with transfer: none */

  return ATK_OBJECT (jaw_impl);
}

/**
 * jaw_table_get_row_header:
 * @table: a GObject instance that implements AtkTableIface
 * @row: a #gint representing a row in the table
 *
 * Gets the row header of a specified row in an accessible table.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 *
 * Returns: (nullable) (transfer none): a AtkObject* representing the
 * specified row header, or %NULL if value does not implement this
 * interface.
 **/

static AtkObject *
jaw_table_get_row_header (AtkTable *table, gint row)
{
  JAW_DEBUG_C ("%p, %d", table, row);
  JAW_GET_TABLE (table, NULL);

  jclass classAtkTable = (*env)->FindClass (env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID (env, classAtkTable, "get_row_header", "(I)Ljavax/accessibility/AccessibleContext;");
  jobject jac = (*env)->CallObjectMethod (env, atk_table, jmid, (jint) row);
  (*env)->DeleteGlobalRef (env, atk_table);

  if (!jac)
    return NULL;

  JawImpl *jaw_impl = jaw_impl_get_instance_from_jaw (env, jac);
  /* get_row_header returns with transfer: none */

  return ATK_OBJECT (jaw_impl);
}

/**
 * jaw_table_get_summary:
 * @table: a GObject instance that implements AtkTableIface
 *
 * Gets the summary description of the table.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 *
 * Returns: (transfer full): a AtkObject* representing a summary description
 * of the table, or zero if value does not implement this interface.
 **/

static AtkObject *
jaw_table_get_summary (AtkTable *table)
{
  JAW_DEBUG_C ("%p", table);
  JAW_GET_TABLE (table, NULL);

  jclass classAtkTable = (*env)->FindClass (env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID (env, classAtkTable, "get_summary", "()Ljavax/accessibility/AccessibleContext;");
  jobject jac = (*env)->CallObjectMethod (env, atk_table, jmid);
  (*env)->DeleteGlobalRef (env, atk_table);

  if (!jac)
    return NULL;

  JawImpl *jaw_impl = jaw_impl_get_instance_from_jaw (env, jac);
  /* FIXME: get_summary is documented to return with transfer full,
   * but used with transfer null in atk_object_real_get_property,
   * see https://gitlab.gnome.org/GNOME/at-spi2-core/-/issues/207 */

  return ATK_OBJECT (jaw_impl);
}

/**
 * jaw_table_get_selected_columns:
 * @table: a GObject instance that implements AtkTableIface
 * @selected: a #gint** that is to contain the selected columns numbers
 *
 * Gets the selected columns of the table by initializing **selected with
 * the selected column numbers. This array should be freed by the caller.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 *
 * Returns: a gint representing the number of selected columns,
 * or %0 if value does not implement this interface.
 **/

static gint
jaw_table_get_selected_columns (AtkTable *table, gint **selected)
{
  JAW_DEBUG_C ("%p, %p", table, selected);
  JAW_GET_TABLE (table, 0);

  jclass classAtkTable = (*env)->FindClass (env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID (env, classAtkTable, "get_selected_columns", "()[I");
  jintArray jcolumnArray = (*env)->CallObjectMethod (env, atk_table, jmid);
  (*env)->DeleteGlobalRef (env, atk_table);

  if (!jcolumnArray)
    return 0;

  jsize length = (*env)->GetArrayLength (env, jcolumnArray);
  jint *jcolumns = (*env)->GetIntArrayElements (env, jcolumnArray, NULL);
  gint *columns = g_new (gint, length);

  gint i;
  for (i = 0; i < length; i++)
    {
      columns[i] = (gint) jcolumns[i];
    }

  (*env)->ReleaseIntArrayElements (env, jcolumnArray, jcolumns, JNI_ABORT);

  return (gint) length;
}

/**
 * jaw_table_get_selected_rows:
 * @table: a GObject instance that implements AtkTableIface
 * @selected: a #gint** that is to contain the selected row numbers
 *
 * Gets the selected rows of the table by initializing **selected with
 * the selected row numbers. This array should be freed by the caller.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 *
 * Returns: a gint representing the number of selected rows,
 * or zero if value does not implement this interface.
 **/

static gint
jaw_table_get_selected_rows (AtkTable *table, gint **selected)
{
  JAW_DEBUG_C ("%p, %p", table, selected);
  JAW_GET_TABLE (table, 0);

  jclass classAtkTable = (*env)->FindClass (env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID (env, classAtkTable, "get_selected_rows", "()[I");
  jintArray jrowArray = (*env)->CallObjectMethod (env, atk_table, jmid);
  (*env)->DeleteGlobalRef (env, atk_table);

  if (!jrowArray)
    return 0;

  jsize length = (*env)->GetArrayLength (env, jrowArray);
  jint *jrows = (*env)->GetIntArrayElements (env, jrowArray, NULL);
  gint *rows = g_new (gint, length);

  gint i;
  for (i = 0; i < length; i++)
    {
      rows[i] = (gint) jrows[i];
    }

  (*env)->ReleaseIntArrayElements (env, jrowArray, jrows, JNI_ABORT);

  return (gint) length;
}

/**
 * jaw_table_is_column_selected:
 * @table: a GObject instance that implements AtkTableIface
 * @column: a #gint representing a column in @table
 *
 * Gets a boolean value indicating whether the specified @column
 * is selected
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 *
 * Returns: a gboolean representing if the column is selected, or 0
 * if value does not implement this interface.
 **/

static gboolean
jaw_table_is_column_selected (AtkTable *table, gint column)
{
  JAW_DEBUG_C ("%p, %d", table, column);
  JAW_GET_TABLE (table, FALSE);

  jclass classAtkTable = (*env)->FindClass (env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID (env, classAtkTable, "is_column_selected", "(I)Z");
  jboolean jselected = (*env)->CallBooleanMethod (env, atk_table, jmid, (jint) column);
  (*env)->DeleteGlobalRef (env, atk_table);
  return jselected;
}

/**
 * jaw_table_is_row_selected:
 * @table: a GObject instance that implements AtkTableIface
 * @row: a #gint representing a row in @table
 *
 * Gets a boolean value indicating whether the specified @row
 * is selected
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 *
 * Returns: a gboolean representing if the row is selected, or 0
 * if value does not implement this interface.
 **/

static gboolean
jaw_table_is_row_selected (AtkTable *table, gint row)
{
  JAW_DEBUG_C ("%p, %d", table, row);
  JAW_GET_TABLE (table, FALSE);

  jclass classAtkTable = (*env)->FindClass (env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID (env, classAtkTable, "is_row_selected", "(I)Z");
  jboolean jselected = (*env)->CallBooleanMethod (env, atk_table, jmid, (jint) row);
  (*env)->DeleteGlobalRef (env, atk_table);
  return jselected;
}

/**
 * jaw_table_is_selected:
 * @table: a GObject instance that implements AtkTableIface
 * @row: a #gint representing a row in @table
 * @column: a #gint representing a column in @table
 *
 * Gets a boolean value indicating whether the accessible object
 * at the specified @row and @column is selected
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 *
 * Returns: a gboolean representing if the cell is selected, or 0
 * if value does not implement this interface.
 **/

static gboolean
jaw_table_is_selected (AtkTable *table, gint row, gint column)
{
  JAW_DEBUG_C ("%p, %d, %d", table, row, column);
  JAW_GET_TABLE (table, FALSE);

  jclass classAtkTable = (*env)->FindClass (env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID (env, classAtkTable, "is_selected", "(II)Z");
  jboolean jselected = (*env)->CallBooleanMethod (env, atk_table, jmid, (jint) row, (jint) column);
  (*env)->DeleteGlobalRef (env, atk_table);

  return jselected;
}

static gboolean
jaw_table_add_row_selection (AtkTable *table, gint row)
{
  g_warning ("It is impossible to add row selection on AccessibleTable Java Object");
  return FALSE;
}

static gboolean
jaw_table_remove_row_selection (AtkTable *table, gint row)
{
  g_warning ("It is impossible to remove row selection on AccessibleTable Java Object");
  return FALSE;
}

static gboolean
jaw_table_add_column_selection (AtkTable *table, gint column)
{
  g_warning ("It is impossible to add column selection on AccessibleTable Java Object");
  return FALSE;
}

static gboolean
jaw_table_remove_column_selection (AtkTable *table, gint column)
{
  g_warning ("It is impossible to remove column selection on AccessibleTable Java Object");
  return FALSE;
}

/**
 * jaw_table_set_row_description:
 * @table: a GObject instance that implements AtkTableIface
 * @row: a #gint representing a row in @table
 * @description: a #gchar representing the description text
 * to set for the specified @row of @table
 *
 * Sets the description text for the specified @row of @table.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 **/

static void
jaw_table_set_row_description (AtkTable *table, gint row, const gchar *description)
{
  JAW_DEBUG_C ("%p, %d, %s", table, row, description);
  JAW_GET_TABLE (table, );

  jclass classAtkTable = (*env)->FindClass (env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID (env,
                                        classAtkTable,
                                        "setRowDescription",
                                        "(ILjava/lang/String;)V");
  jstring jstr = (*env)->NewStringUTF (env, description);
  (*env)->CallVoidMethod (env, atk_table, jmid, (jint) row, jstr);
  (*env)->DeleteGlobalRef (env, atk_table);
}

/**
 * jaw_table_set_column_description:
 * @table: a GObject instance that implements AtkTableIface
 * @column: a #gint representing a column in @table
 * @description: a #gchar representing the description text
 * to set for the specified @column of the @table
 *
 * Sets the description text for the specified @column of the @table.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 **/

static void
jaw_table_set_column_description (AtkTable *table, gint column, const gchar *description)
{
  JAW_DEBUG_C ("%p, %d, %s", table, column, description);
  JAW_GET_TABLE (table, );

  jclass classAtkTable = (*env)->FindClass (env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID (env,
                                        classAtkTable,
                                        "setColumnDescription",
                                        "(ILjava/lang/String;)V");
  jstring jstr = (*env)->NewStringUTF (env, description);
  (*env)->CallVoidMethod (env, atk_table, jmid, (jint) column, jstr);
  (*env)->DeleteGlobalRef (env, atk_table);
}

static void
jaw_table_set_row_header (AtkTable *table, gint row, AtkObject *header)
{
  g_warning ("It is impossible to set a single row header on AccessibleTable Java Object");
}

static void
jaw_table_set_column_header (AtkTable *table, gint column, AtkObject *header)
{
  g_warning ("It is impossible to set a single column header on AccessibleTable Java Object");
}

/**
 * jaw_table_set_caption:
 * @table: a GObject instance that implements AtkTableIface
 * @caption: a #AtkObject representing the caption to set for @table
 *
 * Sets the caption for the table.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 **/

static void
jaw_table_set_caption (AtkTable *table, AtkObject *caption)
{
  JAW_DEBUG_C ("%p, %p", table, caption);
  JAW_GET_TABLE (table, );

  JawObject *jcaption = JAW_OBJECT (caption);
  if (!jcaption)
    {
      JAW_DEBUG_I ("jcaption == NULL");
      (*env)->DeleteGlobalRef (env, atk_table);
      return;
    }
  jclass accessible = (*env)->FindClass (env, "javax/accessibility/Accessible");
  if (!((*env)->IsInstanceOf (env, jcaption->acc_context, accessible)))
    {
      (*env)->DeleteGlobalRef (env, atk_table);
      return;
    }
  jobject obj = (*env)->NewGlobalRef (env, jcaption->acc_context);
  if (!obj)
    {
      JAW_DEBUG_I ("jcaption obj == NULL");
      (*env)->DeleteGlobalRef (env, atk_table);
      return;
    }
  jclass classAtkTable = (*env)->FindClass (env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID (env, classAtkTable, "setCaption", "(Ljavax/accessibility/Accessible;)V");
  (*env)->CallVoidMethod (env, atk_table, jmid, obj);
  (*env)->DeleteGlobalRef (env, obj);
  (*env)->DeleteGlobalRef (env, atk_table);
}

/**
 * jaw_table_set_summary:
 * @table: a GObject instance that implements AtkTableIface
 * @accessible: an #AtkObject representing the summary description
 * to set for @table
 *
 * Sets the summary description of the table.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed
 **/

static void
jaw_table_set_summary (AtkTable *table, AtkObject *summary)
{
  JAW_DEBUG_C ("%p, %p", table, summary);
  JAW_GET_TABLE (table, );

  JawObject *jsummary = JAW_OBJECT (summary);
  if (!jsummary)
    {
      JAW_DEBUG_I ("jsummary == NULL");
      (*env)->DeleteGlobalRef (env, atk_table);
      return;
    }
  jclass accessible = (*env)->FindClass (env, "javax/accessibility/Accessible");
  if (!((*env)->IsInstanceOf (env, jsummary->acc_context, accessible)))
    {
      (*env)->DeleteGlobalRef (env, atk_table);
      return;
    }
  jobject obj = (*env)->NewGlobalRef (env, jsummary->acc_context);
  if (!obj)
    {
      JAW_DEBUG_I ("jsummary obj == NULL");
      (*env)->DeleteGlobalRef (env, atk_table);
      return;
    }

  jclass classAtkTable = (*env)->FindClass (env, "org/GNOME/Accessibility/AtkTable");
  jmethodID jmid = (*env)->GetMethodID (env, classAtkTable, "setSummary", "(Ljavax/accessibility/Accessible;)V");
  (*env)->CallVoidMethod (env, atk_table, jmid, obj);
  (*env)->DeleteGlobalRef (env, obj);
  (*env)->DeleteGlobalRef (env, atk_table);
}
