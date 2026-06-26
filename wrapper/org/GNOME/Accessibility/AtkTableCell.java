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
import java.lang.ref.WeakReference;

/**
 * The ATK TableCell interface implementation for Java accessibility.
 * <p>
 * This class provides a bridge between Java's AccessibleTable interface
 * and the ATK (Accessibility Toolkit) table cell interface, representing
 * individual cells within an accessible table.
 */
public class AtkTableCell {

    public int row, rowSpan, column, columnSpan;
    WeakReference<AccessibleContext> _ac;
    WeakReference<AccessibleTable> accessibleTableWeakRef;

    public AtkTableCell(AccessibleContext ac) {
        this._ac = new WeakReference<AccessibleContext>(ac);
        Accessible accessibleParent = ac.getAccessibleParent();
        accessibleTableWeakRef = null;
        row = -1;
        rowSpan = -1;
        column = -1;
        columnSpan = -1;
        if (accessibleParent == null) {
            return;
        }
        AccessibleContext parentAccessibleContext = accessibleParent.getAccessibleContext();
        if (parentAccessibleContext == null) {
            return;
        }
        AccessibleTable accessibleTable = parentAccessibleContext.getAccessibleTable();
        if (accessibleTable == null) {
            return;
        }
        accessibleTableWeakRef = new WeakReference<AccessibleTable>(accessibleTable);
        int index = ac.getAccessibleIndexInParent();
        if (!(accessibleTable instanceof AccessibleExtendedTable accessibleExtendedTable)) {
            return;
        }
        row = accessibleExtendedTable.getAccessibleRow(index);
        column = accessibleExtendedTable.getAccessibleColumn(index);
        rowSpan = accessibleTable.getAccessibleRowExtentAt(row, column);
        columnSpan = accessibleTable.getAccessibleColumnExtentAt(row, column);
    }

    // JNI upcalls section

    /**
     * Factory method to create an AtkTableCell instance from an AccessibleContext.
     * Called from native code via JNI.
     *
     * @param ac the AccessibleContext representing a table cell
     * @return a new AtkTableCell instance, or null if creation fails
     */
    public static AtkTableCell createAtkTableCell(AccessibleContext ac) {
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
    public AccessibleTable getTable() {
        if (accessibleTableWeakRef == null)
            return null;
        return accessibleTableWeakRef.get();
    }

    /**
     * Returns the column headers as an array of AccessibleContext objects.
     * Called from native code via JNI.
     *
     * @return an array of AccessibleContext objects representing the column headers,
     * or null if column headers are not available
     */
    public AccessibleContext[] getAccessibleColumnHeader() {
        if (accessibleTableWeakRef == null)
            return null;
        return AtkUtil.invokeInSwing(() -> {
            AccessibleTable iteration = accessibleTableWeakRef.get().getAccessibleColumnHeader();
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
    public AccessibleContext[] getAccessibleRowHeader() {
        if (accessibleTableWeakRef == null)
            return null;
        return AtkUtil.invokeInSwing(() -> {
            AccessibleTable iteration = accessibleTableWeakRef.get().getAccessibleRowHeader();
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
