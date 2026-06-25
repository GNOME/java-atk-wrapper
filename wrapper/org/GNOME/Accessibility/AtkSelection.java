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
import java.lang.ref.WeakReference;

public class AtkSelection {

    WeakReference<AccessibleContext> accessibleContextWeakRef;
    WeakReference<AccessibleSelection> accessibleSelectionWeakRef;

    public AtkSelection(AccessibleContext ac) {
        super();
        this.accessibleContextWeakRef = new WeakReference<AccessibleContext>(ac);
        this.accessibleSelectionWeakRef =
                new WeakReference<AccessibleSelection>(ac.getAccessibleSelection());
    }

    public static AtkSelection createAtkSelection(AccessibleContext ac) {
        return AtkUtil.invokeInSwing(() -> {
            return new AtkSelection(ac);
        }, null);
    }

    public boolean add_selection(int index) {
        AccessibleSelection accessibleSelection = accessibleSelectionWeakRef.get();
        if (accessibleSelection == null)
            return false;

        return AtkUtil.invokeInSwing(() -> {
            accessibleSelection.addAccessibleSelection(index);
            return is_child_selected(index);
        }, false);
    }

    public boolean clear_selection() {
        AccessibleSelection accessibleSelection = accessibleSelectionWeakRef.get();
        if (accessibleSelection == null)
            return false;

        AtkUtil.invokeInSwing(() -> {
            accessibleSelection.clearAccessibleSelection();
        });
        return true;
    }

    public AccessibleContext ref_selection(int index) {
        AccessibleSelection accessibleSelection = accessibleSelectionWeakRef.get();
        if (accessibleSelection == null)
            return null;

        return AtkUtil.invokeInSwing(() -> {
            Accessible selectedChild = accessibleSelection.getAccessibleSelection(index);
            if (selectedChild == null)
                return null;
            return selectedChild.getAccessibleContext();
        }, null);
    }

    public int get_selection_count() {
        AccessibleContext accessibleContext = accessibleContextWeakRef.get();
        if (accessibleContext == null)
            return 0;
        AccessibleSelection accessibleSelection = accessibleSelectionWeakRef.get();
        if (accessibleSelection == null)
            return 0;

        return AtkUtil.invokeInSwing(() -> {
            int count = 0;
            for (int i = 0; i < accessibleContext.getAccessibleChildrenCount(); i++) {
                if (accessibleSelection.isAccessibleChildSelected(i))
                    count++;
            }
            return count;
        }, 0);
        //A bug in AccessibleJMenu??
        //return acc_selection.getAccessibleSelectionCount();
    }

    public boolean is_child_selected(int i) {
        AccessibleSelection accessibleSelection = accessibleSelectionWeakRef.get();
        if (accessibleSelection == null)
            return false;

        return AtkUtil.invokeInSwing(() -> {
            return accessibleSelection.isAccessibleChildSelected(i);
        }, false);
    }

    public boolean remove_selection(int i) {
        AccessibleSelection accessibleSelection = accessibleSelectionWeakRef.get();
        if (accessibleSelection == null)
            return false;

        return AtkUtil.invokeInSwing(() -> {
            accessibleSelection.removeAccessibleSelection(i);
            return !is_child_selected(i);
        }, false);
    }

    public boolean select_all_selection() {
        AccessibleContext accessibleContext = accessibleContextWeakRef.get();
        if (accessibleContext == null)
            return false;
        AccessibleSelection accessibleSelection = accessibleSelectionWeakRef.get();
        if (accessibleSelection == null)
            return false;

        AccessibleStateSet stateSet = accessibleContext.getAccessibleStateSet();
        return AtkUtil.invokeInSwing(() -> {
            if (stateSet.contains(AccessibleState.MULTISELECTABLE)) {
                accessibleSelection.selectAllAccessibleSelection();
                return true;
            }
            return false;
        }, false);
    }
}
