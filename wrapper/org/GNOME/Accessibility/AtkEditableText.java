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
import javax.accessibility.AccessibleEditableText;
import javax.swing.text.AttributeSet;
import java.awt.*;
import java.awt.datatransfer.StringSelection;
import java.lang.ref.WeakReference;

public class AtkEditableText extends AtkText {

    WeakReference<AccessibleEditableText> accessibleEditableTextWeakRef;

    public AtkEditableText(AccessibleContext ac) {
        super(ac);
        accessibleEditableTextWeakRef =
                new WeakReference<AccessibleEditableText>(ac.getAccessibleEditableText());
    }

    public static AtkEditableText createAtkEditableText(AccessibleContext ac) {
        return AtkUtil.invokeInSwing(() -> {
            return new AtkEditableText(ac);
        }, null);
    }

    public void set_text_contents(String textContent) {
        AccessibleEditableText accessibleEditableText = accessibleEditableTextWeakRef.get();
        if (accessibleEditableText == null)
            return;

        AtkUtil.invokeInSwing(() -> {
            accessibleEditableText.setTextContents(textContent);
        });
    }

    public void insert_text(String textToInsert, int codePointIndex) {
        AccessibleEditableText accessibleEditableText = accessibleEditableTextWeakRef.get();
        if (accessibleEditableText == null)
            return;

        if (codePointIndex < 0)
            codePointIndex = 0;
        final int rightPosition = codePointIndex;
        AtkUtil.invokeInSwing(() -> {
            accessibleEditableText.insertTextAtIndex(rightPosition, textToInsert);
        });
    }

    public void copy_text(int startCodePointIndex, int endCodePointIndex) {
        AccessibleEditableText accessibleEditableText = accessibleEditableTextWeakRef.get();
        if (accessibleEditableText == null)
            return;

        int n = accessibleEditableText.getCharCount();
        if (startCodePointIndex < 0) {
            startCodePointIndex = 0;
        }
        if (endCodePointIndex > n || endCodePointIndex == -1) {
            endCodePointIndex = n;
        } else if (endCodePointIndex < -1) {
            endCodePointIndex = 0;
        }
        final int rightStart = startCodePointIndex;
        final int rightEnd = endCodePointIndex;
        AtkUtil.invokeInSwing(() -> {
            String textContent = accessibleEditableText.getTextRange(rightStart, rightEnd);
            if (textContent != null) {
                StringSelection stringSel = new StringSelection(textContent);
                Toolkit.getDefaultToolkit().getSystemClipboard().setContents(stringSel, stringSel);
            }
        });
    }

    public void cut_text(int startCodePointIndex, int endCodePointIndex) {
        AccessibleEditableText accessibleEditableText = accessibleEditableTextWeakRef.get();
        if (accessibleEditableText == null)
            return;

        AtkUtil.invokeInSwing(() -> {
            accessibleEditableText.cut(startCodePointIndex, endCodePointIndex);
        });
    }

    public void delete_text(int startCodePointIndex, int endCodePointIndex) {
        AccessibleEditableText accessibleEditableText = accessibleEditableTextWeakRef.get();
        if (accessibleEditableText == null)
            return;

        AtkUtil.invokeInSwing(() -> {
            accessibleEditableText.delete(startCodePointIndex, endCodePointIndex);
        });
    }

    public void paste_text(int codePointOffset) {
        AccessibleEditableText accessibleEditableText = accessibleEditableTextWeakRef.get();
        if (accessibleEditableText == null)
            return;

        AtkUtil.invokeInSwing(() -> {
            accessibleEditableText.paste(codePointOffset);
        });
    }

    /**
     * Sets run attributes for the text between two indices.
     *
     * @param as    the AttributeSet for the text
     * @param start the start index of the text as an int
     * @param end   the end index for the text as an int
     * @return whether setRunAttributes was called
     * TODO return is a bit presumptious. This should ideally include a check for whether
     *      attributes were set.
     */
    public boolean setRunAttributes(AttributeSet as, int start, int end) {
        AccessibleEditableText accessibleEditableText = accessibleEditableTextWeakRef.get();
        if (accessibleEditableText == null)
            return false;

        return AtkUtil.invokeInSwing(() -> {
            accessibleEditableText.setAttributes(start, end, as);
            return true;
        }, false);
    }

}
