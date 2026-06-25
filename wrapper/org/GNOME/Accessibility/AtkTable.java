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

package org.GNOME.Accessibility;

import javax.accessibility.Accessible;
import javax.accessibility.AccessibleContext;
import javax.accessibility.AccessibleExtendedTable;
import javax.accessibility.AccessibleTable;
import java.lang.ref.WeakReference;

public class AtkTable {

    WeakReference<AccessibleContext> accessibleContextWeakRef;
    WeakReference<AccessibleTable> accessibleTableWeakRef;

    public AtkTable(AccessibleContext ac) {
        this.accessibleContextWeakRef = new WeakReference<AccessibleContext>(ac);
        this.accessibleTableWeakRef = new WeakReference<AccessibleTable>(ac.getAccessibleTable());
    }

    public static AtkTable createAtkTable(AccessibleContext ac) {
        return AtkUtil.invokeInSwing(() -> {
            return new AtkTable(ac);
        }, null);
    }

    public AccessibleContext ref_at(int row, int column) {
        AccessibleTable accessibleTable = accessibleTableWeakRef.get();
        if (accessibleTable == null)
            return null;

        return AtkUtil.invokeInSwing(() -> {
            Accessible accessible = accessibleTable.getAccessibleAt(row, column);
            if (accessible != null)
                return accessible.getAccessibleContext();
            return null;
        }, null);
    }

    public int get_index_at(int row, int column) {
        AccessibleTable accessibleTable = accessibleTableWeakRef.get();
        if (accessibleTable == null)
            return -1;

        return AtkUtil.invokeInSwing(() -> {
            if (accessibleTable instanceof AccessibleExtendedTable)
                return ((AccessibleExtendedTable) accessibleTable).getAccessibleIndex(row, column);
            Accessible child = accessibleTable.getAccessibleAt(row, column);
            if (child == null)
                return -1;
            AccessibleContext childAccessibleContext = child.getAccessibleContext();
            if (childAccessibleContext == null)
                return -1;
            return childAccessibleContext.getAccessibleIndexInParent();
        }, -1);
    }

    public int get_column_at_index(int index) {
        AccessibleTable accessibleTable = accessibleTableWeakRef.get();
        if (accessibleTable == null)
            return -1;

        return AtkUtil.invokeInSwing(() -> {
            int column = -1;
            if (accessibleTable instanceof AccessibleExtendedTable)
                column = ((AccessibleExtendedTable) accessibleTable).getAccessibleColumn(index);
            return column;
        }, -1);
    }

    public int get_row_at_index(int index) {
        AccessibleTable accessibleTable = accessibleTableWeakRef.get();
        if (accessibleTable == null)
            return -1;

        return AtkUtil.invokeInSwing(() -> {
            int row = -1;
            if (accessibleTable instanceof AccessibleExtendedTable)
                row = ((AccessibleExtendedTable) accessibleTable).getAccessibleRow(index);
            return row;
        }, -1);
    }

    public int get_n_columns() {
        AccessibleTable accessibleTable = accessibleTableWeakRef.get();
        if (accessibleTable == null)
            return 0;

        return AtkUtil.invokeInSwing(() -> {
            return accessibleTable.getAccessibleColumnCount();
        }, 0);
    }

    public int get_n_rows() {
        AccessibleTable accessibleTable = accessibleTableWeakRef.get();
        if (accessibleTable == null)
            return 0;

        return AtkUtil.invokeInSwing(() -> {
            return accessibleTable.getAccessibleRowCount();
        }, 0);
    }

    public int get_column_extent_at(int row, int column) {
        AccessibleTable accessibleTable = accessibleTableWeakRef.get();
        if (accessibleTable == null)
            return 0;

        return AtkUtil.invokeInSwing(() -> {
            return accessibleTable.getAccessibleColumnExtentAt(row, column);
        }, 0);
    }

    public int get_row_extent_at(int row, int column) {
        AccessibleTable accessibleTable = accessibleTableWeakRef.get();
        if (accessibleTable == null)
            return 0;

        return AtkUtil.invokeInSwing(() -> {
            return accessibleTable.getAccessibleRowExtentAt(row, column);
        }, 0);
    }

    public AccessibleContext get_caption() {
        AccessibleTable accessibleTable = accessibleTableWeakRef.get();
        if (accessibleTable == null)
            return null;

        return AtkUtil.invokeInSwing(() -> {
            Accessible accessible = accessibleTable.getAccessibleCaption();
            if (accessible != null)
                return accessible.getAccessibleContext();
            return null;
        }, null);
    }

    /**
     *
     * @param a an Accessible object
     */
    public void setCaption(Accessible a) {
        AccessibleTable accessibleTable = accessibleTableWeakRef.get();
        if (accessibleTable == null)
            return;

        AtkUtil.invokeInSwing(() -> {
            accessibleTable.setAccessibleCaption(a);
        });
    }

    public String get_column_description(int column) {
        AccessibleTable accessibleTable = accessibleTableWeakRef.get();
        if (accessibleTable == null)
            return "";

        return AtkUtil.invokeInSwing(() -> {
            Accessible accessible = accessibleTable.getAccessibleColumnDescription(column);
            if (accessible != null) {
                AccessibleContext accessibleContext = accessible.getAccessibleContext();
                if (accessibleContext != null)
                    return accessibleContext.getAccessibleDescription();
            }
            return "";
        }, "");
    }

    /**
     *
     * @param column      an int representing a column in table
     * @param description a String object representing the description text to set for the
     *                    specified column of the table
     */
    public void setColumnDescription(int column, String description) {
        AccessibleTable accessibleTable = accessibleTableWeakRef.get();
        if (accessibleTable == null)
            return;

        AtkUtil.invokeInSwing(() -> {
            Accessible accessible = accessibleTable.getAccessibleColumnDescription(column);
            if (accessible != null && description.equals(accessible.toString()))
                accessibleTable.setAccessibleColumnDescription(column, accessible);
        });
    }

    public String get_row_description(int row) {
        AccessibleTable accessibleTable = accessibleTableWeakRef.get();
        if (accessibleTable == null)
            return "";

        return AtkUtil.invokeInSwing(() -> {
            Accessible accessible = accessibleTable.getAccessibleRowDescription(row);
            if (accessible != null) {
                AccessibleContext accessibleContext = accessible.getAccessibleContext();
                if (accessibleContext != null)
                    return accessibleContext.getAccessibleDescription();
            }
            return "";
        }, "");
    }

    /**
     *
     * @param row         an int representing a row in table
     * @param description a String object representing the description text to set for the
     *                    specified row of the table
     */
    public void setRowDescription(int row, String description) {
        AccessibleTable accessibleTable = accessibleTableWeakRef.get();
        if (accessibleTable == null)
            return;

        AtkUtil.invokeInSwing(() -> {
            Accessible accessible = accessibleTable.getAccessibleRowDescription(row);
            if (accessible != null && description.equals(accessible.toString()))
                accessibleTable.setAccessibleRowDescription(row, accessible);
        });
    }

    public AccessibleContext get_column_header(int column) {
        AccessibleTable accessibleTable = accessibleTableWeakRef.get();
        if (accessibleTable == null)
            return null;

        return AtkUtil.invokeInSwing(() -> {
            AccessibleTable headerTable = accessibleTable.getAccessibleColumnHeader();
            if (headerTable != null) {
                Accessible accessible = headerTable.getAccessibleAt(0, column);
                if (accessible != null)
                    return accessible.getAccessibleContext();
            }
            return null;
        }, null);
    }

    public AccessibleContext get_row_header(int row) {
        AccessibleTable accessibleTable = accessibleTableWeakRef.get();
        if (accessibleTable == null)
            return null;

        return AtkUtil.invokeInSwing(() -> {
            AccessibleTable headerTable = accessibleTable.getAccessibleRowHeader();
            if (headerTable != null) {
                Accessible accessible = headerTable.getAccessibleAt(row, 0);
                if (accessible != null)
                    return accessible.getAccessibleContext();
            }
            return null;
        }, null);
    }

    public AccessibleContext get_summary() {
        AccessibleTable accessibleTable = accessibleTableWeakRef.get();
        if (accessibleTable == null)
            return null;

        return AtkUtil.invokeInSwing(() -> {
            Accessible accessible = accessibleTable.getAccessibleSummary();
            if (accessible != null)
                return accessible.getAccessibleContext();
            return null;
        }, null);
    }

    /**
     *
     * @param a the Accessible object to set summary for
     */
    public void setSummary(Accessible a) {
        AccessibleTable accessibleTable = accessibleTableWeakRef.get();
        if (accessibleTable == null)
            return;

        AtkUtil.invokeInSwing(() -> {
            accessibleTable.setAccessibleSummary(a);
        });
    }

    public int[] get_selected_columns() {
        int[] d = new int[0];
        AccessibleTable accessibleTable = accessibleTableWeakRef.get();
        if (accessibleTable == null)
            return d;

        return AtkUtil.invokeInSwing(() -> {
            return accessibleTable.getSelectedAccessibleColumns();
        }, d);
    }

    public int[] get_selected_rows() {
        int[] d = new int[0];
        AccessibleTable accessibleTable = accessibleTableWeakRef.get();
        if (accessibleTable == null)
            return d;

        return AtkUtil.invokeInSwing(() -> {
            return accessibleTable.getSelectedAccessibleRows();
        }, d);
    }

    public boolean is_column_selected(int column) {
        AccessibleTable accessibleTable = accessibleTableWeakRef.get();
        if (accessibleTable == null)
            return false;

        return AtkUtil.invokeInSwing(() -> {
            return accessibleTable.isAccessibleColumnSelected(column);
        }, false);
    }

    public boolean is_row_selected(int row) {
        AccessibleTable accessibleTable = accessibleTableWeakRef.get();
        if (accessibleTable == null)
            return false;

        return AtkUtil.invokeInSwing(() -> {
            return accessibleTable.isAccessibleRowSelected(row);
        }, false);
    }

    public boolean is_selected(int row, int column) {
        AccessibleTable accessibleTable = accessibleTableWeakRef.get();
        if (accessibleTable == null)
            return false;

        return AtkUtil.invokeInSwing(() -> {
            return accessibleTable.isAccessibleSelected(row, column);
        }, false);
    }
}
