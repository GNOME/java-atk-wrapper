/*
 * Java ATK Wrapper for GNOME
 * Copyright (C) 2009 Sun Microsystems Inc.
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

public class AtkTable {

	AccessibleContext ac;
	AccessibleTable acc_table;

	public AtkTable (AccessibleContext ac) {
		super();
		this.acc_table = ac.getAccessibleTable();
	}

	public AccessibleContext ref_at (int row, int column) {
		javax.accessibility.Accessible accessible = acc_table.getAccessibleAt(row, column);
		if (accessible != null) {
			return accessible.getAccessibleContext();
		}

		return null;
	}

	public int get_index_at (int row, int column) {
		int index = -1;

		if (acc_table instanceof AccessibleExtendedTable) {
			index = ((AccessibleExtendedTable)acc_table).getAccessibleIndex(row, column);
		}

		return index;
	}

	public int get_column_at_index (int index) {
		int column = -1;

		if (acc_table instanceof AccessibleExtendedTable) {
			column = ((AccessibleExtendedTable)acc_table).getAccessibleColumn(index);
		}

		return column;
	}

	public int get_row_at_index (int index) {
		int row = -1;

		if (acc_table instanceof AccessibleExtendedTable) {
			row = ((AccessibleExtendedTable)acc_table).getAccessibleRow(index);
		}

		return row;
	}

	public int get_n_columns () {
		return acc_table.getAccessibleColumnCount();
	}

	public int get_n_rows () {
		return acc_table.getAccessibleRowCount();
	}

	public int get_column_extent_at (int row, int column) {
		return acc_table.getAccessibleColumnExtentAt(row, column);
	}

	public int get_row_extent_at (int row, int column) {
		return acc_table.getAccessibleRowExtentAt(row, column);
	}

	public AccessibleContext get_caption () {
		javax.accessibility.Accessible accessible = acc_table.getAccessibleCaption();

		if (accessible != null) {
			return accessible.getAccessibleContext();
		}

		return null;
	}

	public String get_column_description (int column) {
		javax.accessibility.Accessible accessible =
			acc_table.getAccessibleColumnDescription(column);

		if (accessible != null) {
			return accessible.getAccessibleContext().getAccessibleDescription();
		}

		return "";
	}

	public String get_row_description (int row) {
		javax.accessibility.Accessible accessible =
			acc_table.getAccessibleRowDescription(row);

		if (accessible != null) {
			return accessible.getAccessibleContext().getAccessibleDescription();
		}

		return "";
	}

	public AccessibleContext get_column_header (int column) {
		AccessibleTable accessibleTable =
			acc_table.getAccessibleColumnHeader();

		if (accessibleTable != null) {
			javax.accessibility.Accessible accessible = accessibleTable.getAccessibleAt(0, column);

			if (accessible != null) {
				return accessible.getAccessibleContext();
			}
		}

		return null;
	}

	public AccessibleContext get_row_header (int row) {
		AccessibleTable accessibleTable =
			acc_table.getAccessibleRowHeader();

		if (accessibleTable != null) {
			javax.accessibility.Accessible accessible = accessibleTable.getAccessibleAt(row, 0);

			if (accessible != null) {
				return accessible.getAccessibleContext();
			}
		}

		return null;
	}

	public AccessibleContext get_summary () {
		javax.accessibility.Accessible accessible = acc_table.getAccessibleSummary();

		if (accessible != null) {
			return accessible.getAccessibleContext();
		}

		return null;
	}

	public int[] get_selected_columns () {
		return acc_table.getSelectedAccessibleColumns();
	}

	public int[] get_selected_rows () {
		return acc_table.getSelectedAccessibleRows();
	}

	public boolean is_column_selected (int column) {
		return acc_table.isAccessibleColumnSelected(column);
	}

	public boolean is_row_selected (int row) {
		return acc_table.isAccessibleRowSelected(row);
	}

	public boolean is_selected (int row, int column) {
		return acc_table.isAccessibleSelected(row, column);
	}

	public boolean add_column_selection (int column) {
		return false;
	}

	public boolean add_row_selection (int row) {
		return false;
	}

	public boolean remove_column_selection (int column) {
		return false;
	}

	public boolean remove_row_selection (int row) {
		return false;
	}
}

