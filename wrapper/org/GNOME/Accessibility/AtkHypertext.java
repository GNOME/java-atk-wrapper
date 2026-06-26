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

/**
 * The ATK Hypertext interface implementation for Java accessibility.
 * <p>
 * This class provides a bridge between Java's AccessibleHypertext interface
 * and the ATK (Accessibility Toolkit) hypertext interface.
 * Hypertext allows navigation through documents containing
 * embedded links or hyperlinks.
 */
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

    // JNI upcalls section

    /**
     * Factory method to create an AtkHypertext instance from an AccessibleContext.
     * Called from native code via JNI.
     *
     * @param ac the AccessibleContext to wrap
     * @return a new AtkHypertext instance, or null if creation fails
     */
    public static AtkHypertext createAtkHypertext(AccessibleContext ac) {
        return AtkUtil.invokeInSwing(() -> {
            return new AtkHypertext(ac);
        }, null);
    }

    /**
     * Gets the link in this hypertext document at the specified index.
     * Called from native code via JNI.
     *
     * @param linkIndex an integer specifying the desired link (zero-based)
     * @return the AtkHyperlink at the specified index, or null if not available
     */
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

    /**
     * Gets the number of links within this hypertext document.
     * Called from native code via JNI.
     *
     * @return the number of links within this hypertext document
     */
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

    /**
     * Gets the index into the array of hyperlinks that is associated with
     * the character specified by charIndex.
     * Called from native code via JNI.
     *
     * @param charIndex a character index
     * @return an index into the array of hyperlinks in this hypertext,
     * or -1 if there is no hyperlink associated with this character
     */
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
