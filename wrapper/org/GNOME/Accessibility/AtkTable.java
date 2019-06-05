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
import javax.swing.*;

public class AtkTable {

	AccessibleContext ac;
	AccessibleTable acc_table;

	public AtkTable (AccessibleContext ac) {
		this.ac = ac;
		this.acc_table = ac.getAccessibleTable();
	}

	public AccessibleContext ref_at (int row, int column) {
		Accessible accessible = acc_table.getAccessibleAt(row, column);
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
		Accessible accessible = acc_table.getAccessibleCaption();
		if (accessible != null) {
			return accessible.getAccessibleContext();
		}
		return null;
	}
	private class SetCaptionRunner implements Runnable {
        private AccessibleTable acc_table;
        private Accessible a;

        public SetCaptionRunner (AccessibleTable acc_table, Accessible a) {
            this.acc_table = acc_table;
            this.a = a;
        }

        public void run () {
            acc_table.setAccessibleCaption(a);
        }
    }

    /**
     *
     * @param a an Accessible object
     */
    public void setCaption(Accessible a) {
		SwingUtilities.invokeLater(new SetCaptionRunner(acc_table, a));
    }

	public String get_column_description (int column) {
		Accessible accessible = acc_table.getAccessibleColumnDescription(column);
		if (accessible != null) {
			AccessibleContext ac = accessible.getAccessibleContext();
			if (ac != null) {
				return ac.getAccessibleDescription();
			}
		}
		return "";
	}
	private class SetColumnDescriptionRunner implements Runnable {
	    private AccessibleTable acc_table;
		private int column;
	    private Accessible accessible;

	    public SetColumnDescriptionRunner (AccessibleTable acc_table, int column, Accessible accessible) {
	        this.acc_table = acc_table;
	        this.column = column;
			this.accessible = accessible;
	    }

	    public void run () {
	        acc_table.setAccessibleColumnDescription(column, accessible);
	    }
	}

	/**
	*
	* @param column an int representing a column in table
	* @param description a String object representing the description text to set for the
	*  specified column of the table
	*/
	public void setColumnDescription(int column, String description) {
		Accessible accessible = acc_table.getAccessibleColumnDescription(column);
		if (description.equals(accessible.toString()) && accessible != null) {
			SwingUtilities.invokeLater(new SetColumnDescriptionRunner(acc_table, column, accessible));
		}
	}

	public String get_row_description (int row) {
		Accessible accessible = acc_table.getAccessibleRowDescription(row);
		if (accessible != null) {
			AccessibleContext ac = accessible.getAccessibleContext();
			if (ac != null) {
				return ac.getAccessibleDescription();
			}
		}
		return "";
	}
	private class SetRowDescriptionRunner implements Runnable {
		private AccessibleTable acc_table;
		private int row;
		private Accessible accessible;

		public SetRowDescriptionRunner (AccessibleTable acc_table, int row, Accessible accessible) {
			this.acc_table = acc_table;
			this.row = row;
			this.accessible = accessible;
		}

		public void run () {
			acc_table.setAccessibleRowDescription(row, accessible);
		}
	}

	/**
	*
	* @param row an int representing a row in table
	* @param description a String object representing the description text to set for the
	* specified row of the table
	*/
	public void setRowDescription(int row, String description) {
		Accessible accessible = acc_table.getAccessibleRowDescription(row);
		if (description.equals(accessible.toString()) && accessible != null) {
			SwingUtilities.invokeLater(new SetRowDescriptionRunner(acc_table, row, accessible));
		}
	}

	public AccessibleContext get_column_header (int column) {
		AccessibleTable accessibleTable = acc_table.getAccessibleColumnHeader();
		if (accessibleTable != null) {
			Accessible accessible = accessibleTable.getAccessibleAt(0, column);
			if (accessible != null) {
				return accessible.getAccessibleContext();
			}
		}
		return null;
	}

	private class SetColumnHeaderRunner implements Runnable {
		private AccessibleTable acc_table;
		private AccessibleTable table;

		public SetColumnHeaderRunner (AccessibleTable acc_table, AccessibleTable table) {
			this.acc_table = acc_table;
			this.table = table;
		}

		public void run () {
			acc_table.setAccessibleColumnHeader(table);
		}
	}

    /**
     *
     * @param column an int representing a column in table
     * @param table an AccessibleTable object
     */
    public void setColumnHeader (int column, AccessibleTable table) {
		//do we need column for anything?
		SwingUtilities.invokeLater(new SetColumnHeaderRunner(acc_table, table));
    }

	public AccessibleContext get_row_header (int row) {
		AccessibleTable accessibleTable = acc_table.getAccessibleRowHeader();
		if (accessibleTable != null) {
			Accessible accessible = accessibleTable.getAccessibleAt(row, 0);
			if (accessible != null) {
				return accessible.getAccessibleContext();
			}
		}
		return null;
	}

	private class SetRowHeaderRunner implements Runnable {
		private AccessibleTable acc_table;
		private AccessibleTable table;

		public SetRowHeaderRunner (AccessibleTable acc_table, AccessibleTable table) {
			this.acc_table = acc_table;
			this.table = table;
		}

		public void run () {
			acc_table.setAccessibleRowHeader(table);
		}
	}

    public void setRowHeader (int row, AccessibleTable table) {
		//do we need row for anything?
        acc_table.setAccessibleRowHeader(table);
    }

	public AccessibleContext get_summary () {
		Accessible accessible = acc_table.getAccessibleSummary();
		if (accessible != null) {
			return accessible.getAccessibleContext();
		}
		return null;
	}

	private class SetSummaryRunner implements Runnable {
		private AccessibleTable acc_table;
		private Accessible a;

		public SetSummaryRunner (AccessibleTable acc_table, Accessible a) {
			this.acc_table = acc_table;
			this.a = a;
		}

		public void run () {
			acc_table.setAccessibleSummary(a);
		}
	}

    /**
     *
     * @param a the Accessible object to set summary for
     */
    public void setSummary(Accessible a) {
		SwingUtilities.invokeLater(new SetSummaryRunner(acc_table, a));
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
//I think this method are not implemented.
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
