/*
 * Java ATK Wrapper for GNOME
 * Copyright 2009 Sun Microsystems Inc.
 *
 * This file is part of Java ATK Wrapper.

 * Java ATK Wrapper is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Java ATK Wrapper is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public License
 * along with Java ATK Wrapper.  If not, see <http://www.gnu.org/licenses/>.
 */

package org.GNOME.Accessibility;

import javax.accessibility.*;

public class AtkSelection {

	AccessibleContext ac;
	AccessibleSelection acc_selection;

	public AtkSelection (AccessibleContext ac) {
		super();
		this.ac = ac;
		this.acc_selection = ac.getAccessibleSelection();
	}

	public boolean add_selection (int i) {
		acc_selection.addAccessibleSelection(i);
		return is_child_selected(i);
	}

	public boolean clear_selection () {
		acc_selection.clearAccessibleSelection();
		return true;
	}

	public javax.accessibility.Accessible ref_selection (int i) {
		return acc_selection.getAccessibleSelection(i);
	}

	public int get_selection_count () {
		int count = 0;
		for(int i = 0; i < ac.getAccessibleChildrenCount(); i++) {
			if (acc_selection.isAccessibleChildSelected(i))
				count++;
		}

		return count;
		//A bug in AccessibleJMenu??
		//return acc_selection.getAccessibleSelectionCount();
	}

	public boolean is_child_selected (int i) {
		return acc_selection.isAccessibleChildSelected(i);
	}

	public boolean remove_selection (int i) {
		acc_selection.removeAccessibleSelection(i);
		return !is_child_selected(i);
	}

	public boolean select_all_selection () {
		AccessibleStateSet stateSet = ac.getAccessibleStateSet();

		if (stateSet.contains(AccessibleState.MULTISELECTABLE)) {
			acc_selection.selectAllAccessibleSelection();
			return true;
		}

		return false;
	}
}

