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

public class AtkSelection {

	AccessibleContext ac;
	AccessibleSelection acc_selection;

	public AtkSelection (AccessibleContext ac) {
		super();
		this.ac = ac;
		this.acc_selection = ac.getAccessibleSelection();
	}

	public static AtkSelection createAtkSelection(AccessibleContext ac){
        return AtkUtil.invokeInSwing ( () -> { return new AtkSelection(ac); }, null);
    }

	public boolean add_selection (int i) {
		return AtkUtil.invokeInSwing( () -> {
			acc_selection.addAccessibleSelection(i);
			return is_child_selected(i);
		}, false);
	}

	public boolean clear_selection () {
		AtkUtil.invokeInSwing( () -> { acc_selection.clearAccessibleSelection(); });
		return true;
	}

	public AccessibleContext ref_selection (int i) {
		return AtkUtil.invokeInSwing ( () -> { return acc_selection.getAccessibleSelection(i).getAccessibleContext(); }, null);
	}

	public int get_selection_count () {
		return AtkUtil.invokeInSwing ( () -> {
			int count = 0;
			for(int i = 0; i < ac.getAccessibleChildrenCount(); i++) {
				if (acc_selection.isAccessibleChildSelected(i))
					count++;
			}
			return count;
		}, 0);
		//A bug in AccessibleJMenu??
		//return acc_selection.getAccessibleSelectionCount();
	}

	public boolean is_child_selected (int i) {
		return AtkUtil.invokeInSwing ( () -> { return acc_selection.isAccessibleChildSelected(i); }, false);
	}

	public boolean remove_selection (int i) {
		return AtkUtil.invokeInSwing( () -> {
			acc_selection.removeAccessibleSelection(i);
			return !is_child_selected(i);
		}, false);
	}

	public boolean select_all_selection () {
		AccessibleStateSet stateSet = ac.getAccessibleStateSet();
		return AtkUtil.invokeInSwing ( () -> {
			if (stateSet.contains(AccessibleState.MULTISELECTABLE)) {
				acc_selection.selectAllAccessibleSelection();
				return true;
			}
			return false;
		}, false);
	}
}
