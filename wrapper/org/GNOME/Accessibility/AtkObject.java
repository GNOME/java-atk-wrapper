/*
 * Copyright (c) 2018, Red Hat, Inc.
 * Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Oracle designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Oracle in the LICENSE file that accompanied this code.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

package org.GNOME.Accessibility;

import javax.accessibility.*;
import javax.swing.*;
import java.awt.event.InputEvent;
import java.awt.event.KeyEvent;
import java.util.Locale;

/**
 * AtkObject:
 * <p>
 * Java-side utility class used by the GNOME Accessibility Bridge.
 * <p>
 * That class is used to wrap AccessibleContext Java object
 * to avoid the concurrency of AWT objects.
 *
 * @autor Giuseppe Capaldo
 */
public class AtkObject {

    public static final int INTERFACE_ACTION = 0x00000001;
    public static final int INTERFACE_COMPONENT = 0x00000002;
    public static final int INTERFACE_DOCUMENT = 0x00000004;
    public static final int INTERFACE_EDITABLE_TEXT = 0x00000008;
    public static final int INTERFACE_HYPERLINK = 0x00000010;
    public static final int INTERFACE_HYPERTEXT = 0x00000020;
    public static final int INTERFACE_IMAGE = 0x00000040;
    public static final int INTERFACE_SELECTION = 0x00000080;
    public static final int INTERFACE_STREAMABLE_CONTENT = 0x00000100;
    public static final int INTERFACE_TABLE = 0x00000200;
    public static final int INTERFACE_TABLE_CELL = 0x00000400;
    public static final int INTERFACE_TEXT = 0x00000800;
    public static final int INTERFACE_VALUE = 0x00001000;

    /**
     * Returns the JMenuItem accelerator. Similar implementation is used on
     * macOS, see CAccessibility.getAcceleratorText(AccessibleContext) in OpenJDK, and
     * on Windows, see AccessBridge.getAccelerator(AccessibleContext) in OpenJDK.
     */
    private static String getAcceleratorText(AccessibleContext ac) {
        String acceleratorText = "";
        Accessible parent = ac.getAccessibleParent();
        if (parent != null) {
            int indexInParent = ac.getAccessibleIndexInParent();
            Accessible child = parent.getAccessibleContext()
                    .getAccessibleChild(indexInParent);
            if (child instanceof JMenuItem menuItem) {
                KeyStroke keyStroke = menuItem.getAccelerator();
                if (keyStroke != null) {
                    int modifiers = keyStroke.getModifiers();
                    String modifiersText = modifiers > 0 ? InputEvent.getModifiersExText(modifiers) : "";

                    int keyCode = keyStroke.getKeyCode();
                    String keyCodeText = keyCode != 0 ? KeyEvent.getKeyText(keyCode) : String.valueOf(keyStroke.getKeyChar());

                    acceleratorText += modifiersText;
                    if (!modifiersText.isEmpty() && !keyCodeText.isEmpty()) {
                        acceleratorText += "+";
                    }
                    acceleratorText += keyCodeText;
                }
            }
        }
        return acceleratorText;
    }

    // JNI upcalls section

    /**
     * Gets the ATK interface flags for the given accessible object.
     * Called from native code via JNI.
     *
     * @param o the accessible object (AccessibleContext or Accessible)
     * @return bitwise OR of ATK interface flags from {@link AtkInterface}
     */
    private static int getTFlagFromObj(Object o) {
        return AtkUtil.invokeInSwing(() -> {
            int flags = 0;
            AccessibleContext ac;

            if (o instanceof AccessibleContext)
                ac = (AccessibleContext) o;
            else if (o instanceof Accessible)
                ac = ((Accessible) o).getAccessibleContext();
            else
                return flags;

            if (ac.getAccessibleAction() != null)
                flags |= AtkObject.INTERFACE_ACTION;
            if (ac.getAccessibleComponent() != null)
                flags |= AtkObject.INTERFACE_COMPONENT;
            AccessibleText text = ac.getAccessibleText();
            if (text != null) {
                flags |= AtkObject.INTERFACE_TEXT;
                if (text instanceof AccessibleHypertext)
                    flags |= AtkObject.INTERFACE_HYPERTEXT;
                if (ac.getAccessibleEditableText() != null)
                    flags |= AtkObject.INTERFACE_EDITABLE_TEXT;
            }
            if (ac.getAccessibleIcon() != null)
                flags |= AtkObject.INTERFACE_IMAGE;
            if (ac.getAccessibleSelection() != null)
                flags |= AtkObject.INTERFACE_SELECTION;
            AccessibleTable table = ac.getAccessibleTable();
            if (table != null) {
                flags |= AtkObject.INTERFACE_TABLE;
            }
            Accessible parent = ac.getAccessibleParent();
            if (parent != null) {
                AccessibleContext parentAccessibleContext = parent.getAccessibleContext();
                if (parentAccessibleContext != null) {
                    table = parentAccessibleContext.getAccessibleTable();
                    // Unfortunately without the AccessibleExtendedTable interface
                    // we can't determine the column/row of this accessible in the
                    // table
                    if (table != null && table instanceof AccessibleExtendedTable) {
                        flags |= AtkObject.INTERFACE_TABLE_CELL;
                    }
                }
            }
            if (ac.getAccessibleValue() != null)
                flags |= AtkObject.INTERFACE_VALUE;
            return flags;
        }, 0);
    }

    /**
     * Gets the parent AccessibleContext of the given accessible context.
     * Called from native code via JNI.
     *
     * @param ac the accessible context
     * @return the parent accessible context, or null if no parent exists
     */
    private static AccessibleContext getAccessibleParent(AccessibleContext ac) {
        return AtkUtil.invokeInSwing(() -> {
            Accessible accessibleParent = ac.getAccessibleParent();
            if (accessibleParent != null)
                return accessibleParent.getAccessibleContext();
            else
                return null;
        }, null);
    }

    /**
     * Sets the parent of the given accessible context.
     * Called from native code via JNI.
     *
     * @param ac                      the accessible context whose parent should be set
     * @param parentAccessibleContext the new parent accessible context (must be Accessible)
     */
    private static void setAccessibleParent(
            AccessibleContext ac, AccessibleContext parentAccessibleContext) {
        AtkUtil.invokeInSwing(() -> {
            if (parentAccessibleContext instanceof Accessible parentAccessible) {
                ac.setAccessibleParent(parentAccessible);
            }
        });
    }

    /**
     * Gets the accessible name of the given accessible context.
     * Called from native code via JNI.
     *
     * @param ac the accessible context
     * @return the accessible name, with accelerator text appended, or null if no name is set
     */
    private static String getAccessibleName(AccessibleContext ac) {
        return AtkUtil.invokeInSwing(() -> {
            String accessibleName = ac.getAccessibleName();
            if (accessibleName == null) {
                return null;
            }
            final String acceleratorText = getAcceleratorText(ac);
            if (!acceleratorText.isEmpty()) {
                return accessibleName + " " + acceleratorText;
            }
            return accessibleName;
        }, "");
    }

    /**
     * Sets the accessible name of the given accessible context.
     * Called from native code via JNI.
     *
     * @param ac   the accessible context
     * @param name the new accessible name
     */
    private static void setAccessibleName(AccessibleContext ac, String name) {
        AtkUtil.invokeInSwing(() -> {
            ac.setAccessibleName(name);
        });
    }

    /**
     * Gets the accessible description of the given accessible context.
     * Called from native code via JNI.
     *
     * @param ac the accessible context
     * @return the accessible description, or empty string if no description is set
     */
    private static String getAccessibleDescription(AccessibleContext ac) {
        return AtkUtil.invokeInSwing(() -> {
            return ac.getAccessibleDescription();
        }, "");
    }

    /**
     * Sets the accessible description of the given accessible context.
     * Called from native code via JNI.
     *
     * @param ac          the accessible context
     * @param description the new accessible description
     */
    private static void setAccessibleDescription(AccessibleContext ac, String description) {
        AtkUtil.invokeInSwing(() -> {
            ac.setAccessibleDescription(description);
        });
    }

    /**
     * Gets the number of accessible children of the given accessible context.
     * Called from native code via JNI.
     *
     * @param ac the accessible context
     * @return the number of accessible children, or 0 if there are no children
     */
    private static int getAccessibleChildrenCount(AccessibleContext ac) {
        return AtkUtil.invokeInSwing(() -> {
            return ac.getAccessibleChildrenCount();
        }, 0);
    }

    /**
     * Gets the index of this accessible context within its parent's children.
     * Called from native code via JNI.
     *
     * @param ac the accessible context
     * @return the zero-based index in parent, or -1 if no parent exists or index cannot be determined
     */
    private static int getAccessibleIndexInParent(AccessibleContext ac) {
        return AtkUtil.invokeInSwing(() -> {
            return ac.getAccessibleIndexInParent();
        }, -1);
    }

    /**
     * Gets the accessible role of the given accessible context.
     * Called from native code via JNI.
     *
     * @param ac the accessible context
     * @return the accessible role, or null if the role cannot be determined
     */
    private static AccessibleRole getAccessibleRole(AccessibleContext ac) {
        return AtkUtil.invokeInSwing(() -> {
            return ac.getAccessibleRole();
        }, AccessibleRole.UNKNOWN);
    }

    private static boolean equalsIgnoreCaseLocaleWithRole(AccessibleRole role) {
        String displayString = role.toDisplayString(Locale.US);
        return displayString.equalsIgnoreCase("paragraph");
    }

    /**
     * Gets an array of accessible states for the given accessible context.
     * Called from native code via JNI.
     *
     * @param ac the accessible context
     * @return an array of accessible states, or null if no state set exists
     */
    private static AccessibleState[] getArrayAccessibleState(AccessibleContext ac) {
        return AtkUtil.invokeInSwing(() -> {
            AccessibleStateSet stateSet = ac.getAccessibleStateSet();
            if (stateSet == null)
                return null;
            else
                return stateSet.toArray();
        }, null);
    }

    /**
     * Gets the locale of the given accessible context.
     * Called from native code via JNI.
     *
     * @param ac the accessible context
     * @return the locale string in the format "language_country@script@variant", or null if locale cannot be determined
     */
    private static String getLocale(AccessibleContext ac) {
        return AtkUtil.invokeInSwing(() -> {
            Locale l = ac.getLocale();
            String locale = l.getLanguage();
            String country = l.getCountry();
            String script = l.getScript();
            String variant = l.getVariant();
            if (country.length() != 0) {
                locale += "_" + country;
            }
            if (script.length() != 0) {
                locale += "@" + script;
            }
            if (variant.length() != 0) {
                locale += "@" + variant;
            }
            return locale;
        }, null);
    }

    /**
     * Gets an array of accessible relations for the given accessible context.
     * Called from native code via JNI.
     *
     * @param ac the accessible context
     * @return an array of WrapKeyAndTarget records containing relation keys and targets,
     * or an empty array if no relations exist
     */
    private static WrapKeyAndTarget[] getArrayAccessibleRelation(AccessibleContext ac) {
        WrapKeyAndTarget[] d = new WrapKeyAndTarget[0];
        return AtkUtil.invokeInSwing(() -> {
            AccessibleRelationSet relationSet = ac.getAccessibleRelationSet();
            if (relationSet == null)
                return d;
            else {
                AccessibleRelation[] array = relationSet.toArray();
                WrapKeyAndTarget[] result = new WrapKeyAndTarget[array.length];
                for (int i = 0; i < array.length; i++) {
                    String key = array[i].getKey();
                    Object[] objs = array[i].getTarget();
                    AccessibleContext[] contexts = new AccessibleContext[objs.length];
                    for (int j = 0; j < objs.length; j++) {
                        if (objs[i] instanceof Accessible)
                            contexts[i] = ((Accessible) objs[i]).getAccessibleContext();
                        else
                            contexts[i] = null;
                    }
                    result[i] = new WrapKeyAndTarget(key, contexts);
                }
                return result;
            }
        }, d);
    }

    /**
     * Gets the accessible child at the specified index.
     * Called from native code via JNI.
     *
     * @param ac the parent accessible context
     * @param i  the zero-based index of the child
     * @return the child accessible context at the given index, or null if no child exists at that index
     */
    private static AccessibleContext getAccessibleChild(AccessibleContext ac, int i) {
        return AtkUtil.invokeInSwing(() -> {
            Accessible child = ac.getAccessibleChild(i);
            if (child == null)
                return null;
            else
                return child.getAccessibleContext();
        }, null);
    }

    private static int hashCode(AccessibleContext ac) {
        return AtkUtil.invokeInSwing(() -> {
            return ac.hashCode();
        }, 0);
    }

    public static class WrapKeyAndTarget {
        public final String key;
        public final AccessibleContext[] relations;

        /**
         * A record that wraps an accessible relation key with its target accessible contexts.
         * Used to pass relation information from Java to native code.
         */
        public WrapKeyAndTarget(String key, AccessibleContext[] relations) {
            this.key = key;
            this.relations = relations;
        }
    }

}
