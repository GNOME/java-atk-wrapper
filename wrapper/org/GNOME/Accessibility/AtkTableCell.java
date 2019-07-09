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

import javax.accessibility.*;

public class AtkTableCell extends AtkTable {

    AccessibleContext ac;

    AccessibleExtendedTable acc_table_cell;
    private int rowSpan, columnSpan;

    public AtkTableCell (AccessibleContext ac) {
        super(ac);
        this.ac = ac;
        AccessibleTable acc_table = ac.getAccessibleTable();

        if (acc_table instanceof AccessibleExtendedTable) {
            acc_table_cell = (AccessibleExtendedTable)acc_table;
        } else {
            acc_table_cell = null;
        }
        rowSpan = 0;
        columnSpan = 0;
    }

    public static AtkTableCell createAtkTableCell(AccessibleContext ac){
        return AtkUtil.invokeInSwing ( () -> { return new AtkTableCell(ac); }, null);
    }

    /**
    * getTable
    * @return: Reference to the accessible of the containing table as an
    *          AccessibleTable instance.
    */
    public AccessibleTable getTable() {
        return AtkUtil.invokeInSwing ( () -> { return acc_table_cell; }, null);
    }

    /**
    * @param row the row of the accessible table cell
    * @param column the column of the accessible table cell
    * @return: whether the accessible index of the table cell is found
    */
    public boolean getPosition(int row, int column) {
        return AtkUtil.invokeInSwing ( () -> {
            int index = acc_table_cell.getAccessibleIndex(row, column);
            if (index < 0)
                return false;
            return true;
        }, false);
    }

    /**
    * @param row the row of the accessible table cell
    * @param column the column of the accessible table cell
    * @param rowSpan the row span of the accessible table cell the
    * @param columnSpan the column span of the accessible table cell
    * @return: whether the column and row span was retrieved
    */
    public boolean getRowColumnSpan(int row, int column, int rowSpan, int columnSpan) {
        return AtkUtil.invokeInSwing ( () -> {
            this.rowSpan = rowSpan;
            this.columnSpan = columnSpan;
            int chekRowSpan = acc_table.getAccessibleRowExtentAt(row, column);
            int checkColumnSpan = acc_table.getAccessibleColumnExtentAt(row, column);
            if (chekRowSpan < 0 && checkColumnSpan < 0)
                return false;
            return true;
        }, false);
    }

    /**
    * @return: span of the table_cell row as an int
    */
    public int getRowSpan() {
        return AtkUtil.invokeInSwing ( () -> {
            if (rowSpan < 0)
                return -1;
            return rowSpan;
        }, -1);
    }

    /**
    * @return: span of the table_cell column as an int
    */
    public int getColumnSpan() {
        return AtkUtil.invokeInSwing ( () -> {
            if (columnSpan < 0)
                return -1;
            return columnSpan;
        }, -1);
    }

}
