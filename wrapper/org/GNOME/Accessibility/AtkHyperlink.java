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

import javax.accessibility.Accessible;
import javax.accessibility.AccessibleContext;
import javax.accessibility.AccessibleHyperlink;
import java.lang.ref.WeakReference;

public class AtkHyperlink {

    WeakReference<AccessibleHyperlink> accessibleHyperlinkWeakRef;

    public AtkHyperlink(AccessibleHyperlink accessibleHyperlink) {
        super();
        accessibleHyperlinkWeakRef = new WeakReference<AccessibleHyperlink>(accessibleHyperlink);
    }

    public static AtkHyperlink createAtkHyperlink(AccessibleHyperlink accessibleHyperlink) {
        return AtkUtil.invokeInSwing(() -> {
            return new AtkHyperlink(accessibleHyperlink);
        }, null);
    }

    public String get_uri(int index) {
        AccessibleHyperlink accessibleHyperlink = accessibleHyperlinkWeakRef.get();
        if (accessibleHyperlink == null)
            return "";

        return AtkUtil.invokeInSwing(() -> {
            Object o = accessibleHyperlink.getAccessibleActionObject(index);
            if (o != null)
                return o.toString();
            return "";
        }, "");
    }

    public AccessibleContext get_object(int index) {
        AccessibleHyperlink accessibleHyperlink = accessibleHyperlinkWeakRef.get();
        if (accessibleHyperlink == null)
            return null;

        return AtkUtil.invokeInSwing(() -> {
            Object anchor = accessibleHyperlink.getAccessibleActionAnchor(index);
            if (anchor instanceof Accessible)
                return ((Accessible) anchor).getAccessibleContext();
            return null;
        }, null);
    }

    public int get_end_index() {
        AccessibleHyperlink accessibleHyperlink = accessibleHyperlinkWeakRef.get();
        if (accessibleHyperlink == null)
            return 0;

        return AtkUtil.invokeInSwing(() -> {
            return accessibleHyperlink.getEndIndex();
        }, 0);
    }

    public int get_start_index() {
        AccessibleHyperlink accessibleHyperlink = accessibleHyperlinkWeakRef.get();
        if (accessibleHyperlink == null)
            return 0;

        return AtkUtil.invokeInSwing(() -> {
            return accessibleHyperlink.getStartIndex();
        }, 0);
    }

    public boolean is_valid() {
        AccessibleHyperlink accessibleHyperlink = accessibleHyperlinkWeakRef.get();
        if (accessibleHyperlink == null)
            return false;

        return AtkUtil.invokeInSwing(() -> {
            return accessibleHyperlink.isValid();
        }, false);
    }

    public int get_n_anchors() {
        AccessibleHyperlink accessibleHyperlink = accessibleHyperlinkWeakRef.get();
        if (accessibleHyperlink == null)
            return 0;

        return AtkUtil.invokeInSwing(() -> {
            return accessibleHyperlink.getAccessibleActionCount();
        }, 0);
    }
}
