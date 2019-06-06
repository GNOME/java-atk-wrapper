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
import javax.swing.*;
import java.util.concurrent.*;

public class AtkSelection {

	AccessibleContext ac;
	AccessibleSelection acc_selection;

	public AtkSelection (AccessibleContext ac) {
		super();
		this.ac = ac;
		this.acc_selection = ac.getAccessibleSelection();
	}

	private <T> T wrapRunnabeFuture(Callable function, T d){
		RunnableFuture<T> wf = new FutureTask<>(function);
		SwingUtilities.invokeLater(wf);
	    try {
	        return wf.get();
	    } catch (InterruptedException|ExecutionException ex) {
	        ex.printStackTrace(); // we can do better than this
			return d;
	    }
	}

	private class AddSelectionRunner implements Callable<Boolean> {
		private AccessibleSelection acc_selection;
		private int i;

		public AddSelectionRunner (AccessibleSelection acc_selection, int i) {
			this.acc_selection = acc_selection;
			this.i = i;
		}

		public Boolean call () {
			acc_selection.addAccessibleSelection(i);
			return is_child_selected(i);
		}
	}

	public boolean add_selection (int i) {
		return wrapRunnabeFuture(new AddSelectionRunner(acc_selection, i), false);
	}

	private class ClearSelectionRunner implements Runnable {
		private AccessibleSelection acc_selection;

		public ClearSelectionRunner (AccessibleSelection acc_selection) {
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

	private class RemoveSelectionRunner implements Callable<Boolean> {
		private AccessibleSelection acc_selection;
		private int i;

		public RemoveSelectionRunner (AccessibleSelection acc_selection, int i) {
			this.acc_selection = acc_selection;
			this.i = i;
		}

		public Boolean call() {
			acc_selection.removeAccessibleSelection(i);
			return !is_child_selected(i);
		}
	}

	public boolean remove_selection (int i) {
		return wrapRunnabeFuture(new RemoveSelectionRunner(acc_selection, i), false);
	}

	private class SelectionAllRunner implements Runnable {
		private AccessibleSelection acc_selection;

		public SelectionAllRunner (AccessibleSelection acc_selection) {
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
