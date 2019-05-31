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

	public boolean add_selection (int i) {
		RunnableFuture<RemoveSelectionRunner> wf = new FutureTask<>( () ->
			acc_selection.addAccessibleSelection(i);
			return is_child_selected(i);
		);
		SwingUtilities.invokeLater(wf);
	    try {
	        return wf.get();
	    } catch (InterruptedException|ExecutionException ex) {
	        ex.printStackTrace(); // we can do better than this
	    }
	}


	private class ClearSelectionRunner implements Runnable {
		private AccessibleSelection acc_selection;

		public ClearSelectionRunner (AccessibleAction acc_selection) {
			this.acc_selection = acc_selection;
		}

		public void run () {
			acc_selection.clearAccessibleSelection();
		}
	}

	public boolean clear_selection () {
		SwingUtilities.invokeLater(new ClearSelectionRunner(acc_selection));
		return true;
	}

	public Accessible ref_selection (int i) {
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
		RunnableFuture<RemoveSelectionRunner> wf = new FutureTask<>( () ->
			acc_selection.addAccessibleSelection(i);
			return !is_child_selected(i);
		);
		SwingUtilities.invokeLater(wf);
	    try {
	        return wf.get();
	    } catch (InterruptedException|ExecutionException ex) {
	        ex.printStackTrace(); // we can do better than this
	    }
	}

	private class SelectionAllRunner implements Runnable {
		private AccessibleSelection acc_selection;

		public SelectionAllRunner (AccessibleAction acc_selection) {
			this.acc_selection = acc_selection;
		}

		public void run () {
			acc_selection.selectAllAccessibleSelection();
		}
	}

	public boolean select_all_selection () {
		AccessibleStateSet stateSet = ac.getAccessibleStateSet();

		if (stateSet.contains(AccessibleState.MULTISELECTABLE)) {
			SwingUtilities.invokeLater(new SelectionAllRunner(acc_selection));
			return true;
		}

		return false;
	}
}
