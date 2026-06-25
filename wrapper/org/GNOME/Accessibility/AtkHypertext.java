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

import javax.accessibility.AccessibleContext;
import javax.accessibility.AccessibleHyperlink;
import javax.accessibility.AccessibleHypertext;
import javax.accessibility.AccessibleText;
import java.lang.ref.WeakReference;

public class AtkHypertext extends AtkText {

    WeakReference<AccessibleHypertext> accessibleHypertextRef;

    public AtkHypertext(AccessibleContext ac) {
        super(ac);

        AccessibleText accessibleText = ac.getAccessibleText();
        if (accessibleText instanceof AccessibleHypertext) {
            accessibleHypertextRef =
                    new WeakReference<AccessibleHypertext>((AccessibleHypertext) accessibleText);
        } else {
            accessibleHypertextRef = null;
        }
    }

    public static AtkHypertext createAtkHypertext(AccessibleContext ac) {
        return AtkUtil.invokeInSwing(() -> {
            return new AtkHypertext(ac);
        }, null);
    }

    public AtkHyperlink get_link(int linkIndex) {
        if (accessibleHypertextRef == null)
            return null;
        AccessibleHypertext accessibleHypertext = accessibleHypertextRef.get();
        if (accessibleHypertext == null)
            return null;

        return AtkUtil.invokeInSwing(() -> {
            AccessibleHyperlink link = accessibleHypertext.getLink(linkIndex);
            if (link != null)
                return new AtkHyperlink(link);
            return null;
        }, null);
    }

    public int get_n_links() {
        if (accessibleHypertextRef == null)
            return 0;
        AccessibleHypertext accessibleHypertext = accessibleHypertextRef.get();
        if (accessibleHypertext == null)
            return 0;

        return AtkUtil.invokeInSwing(() -> {
            return accessibleHypertext.getLinkCount();
        }, 0);
    }

    public int get_link_index(int charIndex) {
        if (accessibleHypertextRef == null)
            return 0;
        AccessibleHypertext accessibleHypertext = accessibleHypertextRef.get();
        if (accessibleHypertext == null)
            return 0;

        return AtkUtil.invokeInSwing(() -> {
            return accessibleHypertext.getLinkIndex(charIndex);
        }, 0);
    }
}
