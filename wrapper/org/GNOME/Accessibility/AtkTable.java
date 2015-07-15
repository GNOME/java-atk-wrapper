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

import javax.accessibility.*;

public class AtkTable {

	AccessibleContext ac;
	AccessibleTable acc_table;

  public AtkTable (AccessibleContext ac) {
    this.ac = ac;
    this.acc_table = ac.getAccessibleTable();
  }

	public AccessibleContext ref_at (int row, int column) {
		javax.accessibility.Accessible accessible = acc_table.getAccessibleAt(row, column);
		if (accessible != null) {
			return accessible.getAccessibleContext();
		}

		return null;
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

    /**
     *
     * @param a an Accessible object
     */
    public void setCaption(Accessible a) {
        acc_table.setAccessibleCaption(a);
    }

	public String get_column_description (int column) {
		javax.accessibility.Accessible accessible =
			acc_table.getAccessibleColumnDescription(column);

		if (accessible != null) {
			return accessible.getAccessibleContext().getAccessibleDescription();
		}

		return "";
	}

/**
 *
 * @param column an int representing a column in table
 * @param description a String object representing the description text to set for the
 *                    specified column of the table
 */
  public void setColumnDescription(int column, String description) {
    javax.accessibility.Accessible accessible = acc_table.getAccessibleColumnDescription(column);
    if (description.equals(accessible.toString()) && accessible != null) {
      acc_table.setAccessibleColumnDescription(column, accessible);
    }
  }

	public String get_row_description (int row) {
		javax.accessibility.Accessible accessible =
			acc_table.getAccessibleRowDescription(row);

		if (accessible != null) {
			return accessible.getAccessibleContext().getAccessibleDescription();
		}

		return "";
	}

 /**
  *
  * @param row an int representing a row in table
  * @param description a String object representing the description text to set for the
  *                    specified row of the table
  */
  public void setRowDescription(int row, String description) {
    javax.accessibility.Accessible accessible = acc_table.getAccessibleRowDescription(row);
    if (description.equals(accessible.toString()) && accessible != null) {
      acc_table.setAccessibleRowDescription(row, accessible);
    }
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

    /**
     *
     * @param column an int representing a column in table
     * @param table an AccessibleTable object
     */
    public void setColumnHeader (int column, AccessibleTable table) {
        acc_table.setAccessibleColumnHeader(table);
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

    public void setRowHeader (int row, AccessibleTable table) {
        acc_table.setAccessibleRowHeader(table);
    }

	public AccessibleContext get_summary () {
		javax.accessibility.Accessible accessible = acc_table.getAccessibleSummary();

		if (accessible != null) {
			return accessible.getAccessibleContext();
		}

		return null;
	}

    /**
     *
     * @param a the Accessible object to set summary for
     */
    public void setSummary(Accessible a) {
        acc_table.setAccessibleSummary(a);
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

  public boolean addColumnSelection (int column) {
    return false;
  }

  public boolean addRowSelection (int row) {
    return false;
  }

	public boolean remove_column_selection (int column) {
		return false;
	}

	public boolean remove_row_selection (int row) {
		return false;
	}
}

