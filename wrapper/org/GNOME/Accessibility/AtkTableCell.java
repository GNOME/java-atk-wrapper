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

package org.GNOME.Accessibility;

import javax.accessibility.Accessible;
import javax.accessibility.AccessibleContext;
import javax.accessibility.AccessibleExtendedTable;
import javax.accessibility.AccessibleTable;
import java.awt.EventQueue;
import java.lang.ref.WeakReference;

/**
 * The ATK TableCell interface implementation for Java accessibility.
 * <p>
 * This class provides a bridge between Java's AccessibleTable interface
 * and the ATK (Accessibility Toolkit) table cell interface, representing
 * individual cells within an accessible table.
 */
public class AtkTableCell {

    private final int row;
    private final int rowSpan;
    private final int column;
    private final int columnSpan;
    private final WeakReference<AccessibleContext> _ac;
    private final WeakReference<AccessibleTable> parentAccessibleTableWeakRef;

    private AtkTableCell(AccessibleContext ac) {
        assert EventQueue.isDispatchThread();

        if (ac == null) {
            throw new IllegalArgumentException("AccessibleContext must be not null");
        }

        this._ac = new WeakReference<AccessibleContext>(ac);
        Accessible accessibleParent = ac.getAccessibleParent();
        if (accessibleParent == null) {
            throw new IllegalArgumentException("AccessibleContext must have accessibleParent");
        }

        AccessibleContext parentAccessibleContext = accessibleParent.getAccessibleContext();
        if (parentAccessibleContext == null) {
            throw new IllegalArgumentException("AccessibleContext must have accessibleParent with AccessibleContext");
        }

        AccessibleTable parentAccessibleTable = parentAccessibleContext.getAccessibleTable();
        if (parentAccessibleTable == null) {
            throw new IllegalArgumentException("AccessibleContext must have accessibleParent with AccessibleTable");
        }
        parentAccessibleTableWeakRef = new WeakReference<AccessibleTable>(parentAccessibleTable);

        if (parentAccessibleTable instanceof AccessibleExtendedTable parentAccessibleExtendedTable) {
            int index = ac.getAccessibleIndexInParent();
            row = parentAccessibleExtendedTable.getAccessibleRow(index);
            column = parentAccessibleExtendedTable.getAccessibleColumn(index);
            rowSpan = parentAccessibleTable.getAccessibleRowExtentAt(row, column);
            columnSpan = parentAccessibleTable.getAccessibleColumnExtentAt(row, column);
        } else {
            row = -1;
            column = -1;
            rowSpan = -1;
            columnSpan = -1;
        }
    }

    // JNI upcalls section

    /**
     * Factory method to create an AtkTableCell instance from an AccessibleContext.
     * Called from native code via JNI.
     *
     * @param ac the AccessibleContext representing a table cell
     * @return a new AtkTableCell instance, or null if creation fails
     */
    private static AtkTableCell create_atk_table_cell(AccessibleContext ac) {
        return AtkUtil.invokeInSwing(() -> {
            return new AtkTableCell(ac);
        }, null);
    }

    /**
     * Gets the table containing this cell.
     * Called from native code via JNI.
     *
     * @return the AccessibleTable containing this cell, or null if unavailable
     */
    private AccessibleTable get_table() {
        if (parentAccessibleTableWeakRef == null)
            return null;
        return parentAccessibleTableWeakRef.get();
    }

    /**
     * Returns the column headers as an array of AccessibleContext objects.
     * Called from native code via JNI.
     *
     * @return an array of AccessibleContext objects representing the column headers,
     * or null if column headers are not available
     */
    private AccessibleContext[] get_accessible_column_header() {
        if (parentAccessibleTableWeakRef == null)
            return null;
        return AtkUtil.invokeInSwing(() -> {
            AccessibleTable iteration = parentAccessibleTableWeakRef.get().getAccessibleColumnHeader();
            if (iteration != null) {
                int length = iteration.getAccessibleColumnCount();
                AccessibleContext[] result = new AccessibleContext[length];
                for (int i = 0; i < length; i++) {
                    result[i] = iteration.getAccessibleAt(0, i).getAccessibleContext();
                }
                return result;
            }
            return null;
        }, null);
    }

    /**
     * Returns the row headers as an array of AccessibleContext objects.
     * Called from native code via JNI.
     *
     * @return an array of AccessibleContext objects representing the row headers,
     * or null if row headers are not available
     */
    private AccessibleContext[] get_accessible_row_header() {
        if (parentAccessibleTableWeakRef == null)
            return null;
        return AtkUtil.invokeInSwing(() -> {
            AccessibleTable iteration = parentAccessibleTableWeakRef.get().getAccessibleRowHeader();
            if (iteration != null) {
                int length = iteration.getAccessibleRowCount();
                AccessibleContext[] result = new AccessibleContext[length];
                for (int i = 0; i < length; i++) {
                    result[i] = iteration.getAccessibleAt(i, 0).getAccessibleContext();
                }
                return result;
            }
            return null;
        }, null);
    }

}
