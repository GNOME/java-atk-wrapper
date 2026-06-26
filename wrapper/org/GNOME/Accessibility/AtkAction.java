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
import java.awt.event.InputEvent;
import java.awt.event.KeyEvent;
import java.lang.ref.WeakReference;

/**
 * The ATK Action interface implementation for Java accessibility.
 * <p>
 * This class provides a bridge between Java's AccessibleAction interface
 * and the ATK (Accessibility Toolkit) action interface.
 */
public class AtkAction {

    private final WeakReference<AccessibleContext> accessibleContextWeakRef;
    private final WeakReference<AccessibleAction> accessibleActionWeakRef;
    private final WeakReference<AccessibleExtendedComponent> _acc_ext_component;
    private final String[] descriptions;
    private final int nactions;

    private AtkAction(AccessibleContext ac) {
        super();
        this.accessibleContextWeakRef = new WeakReference<AccessibleContext>(ac);
        AccessibleAction accessibleAction = ac.getAccessibleAction();
        this.accessibleActionWeakRef = new WeakReference<AccessibleAction>(accessibleAction);
        this.nactions = accessibleAction.getAccessibleActionCount();
        this.descriptions = new String[nactions];
        AccessibleComponent accessibleComponent = ac.getAccessibleComponent();
        if (accessibleComponent instanceof AccessibleExtendedComponent) {
            this._acc_ext_component =
                    new WeakReference<AccessibleExtendedComponent>(
                            (AccessibleExtendedComponent) accessibleComponent);
        } else {
            this._acc_ext_component = null;
        }
    }

    private String convertModString(String mods) {
        if (mods == null || mods.length() == 0) {
            return "";
        }

        String[] modStrs = mods.split("\\+");
        String newModString = "";
        for (int i = 0; i < modStrs.length; i++) {
            newModString += "<" + modStrs[i] + ">";
        }

        return newModString;
    }

    // JNI upcalls section

    /**
     * Factory method to create an AtkAction instance from an AccessibleContext.
     * Called from native code via JNI.
     *
     * @param ac the AccessibleContext to wrap
     * @return a new AtkAction instance, or null if creation fails
     */
    private static AtkAction createAtkAction(AccessibleContext ac) {
        return AtkUtil.invokeInSwing(() -> {
            return new AtkAction(ac);
        }, null);
    }

    /**
     * Performs the specified action on the object.
     * Called from native code via JNI.
     *
     * @param index the action index corresponding to the action to be performed
     * @return true if the action was successfully performed, false otherwise
     */
    private boolean do_action(int index) {
        AccessibleAction accessibleAction = accessibleActionWeakRef.get();
        if (accessibleAction == null)
            return false;

        AtkUtil.invokeInSwing(() -> {
            accessibleAction.doAccessibleAction(index);
        });
        return true;
    }

    /**
     * Gets the number of accessible actions available on the object.
     * Called from native code via JNI.
     *
     * @return the number of actions, or 0 if this object does not implement actions
     */
    private int get_n_actions() {
        return this.nactions;
    }

    /**
     * Returns a description of the specified action of the object.
     * Called from native code via JNI.
     *
     * @param index the action index corresponding to the action
     * @return a description string, or null if the action does not exist
     */
    private String get_description(int index) {
        AccessibleAction accessibleAction = accessibleActionWeakRef.get();
        if (accessibleAction == null)
            return null;

        if (index >= nactions) {
            return null;
        }
        if (descriptions[index] != null) {
            return descriptions[index];
        }
        descriptions[index] = AtkUtil.invokeInSwing(() -> {
            return accessibleAction.getAccessibleActionDescription(index);
        }, "");
        return descriptions[index];
    }

    /**
     * Sets a description of the specified action of the object.
     * Called from native code via JNI.
     *
     * @param index       the action index corresponding to the action
     * @param description the description to be assigned to this action
     * @return true if the description was successfully set, false otherwise
     */
    private boolean setDescription(int index, String description) {
        if (index >= nactions) {
            return false;
        }
        descriptions[index] = description;
        return true;
    }

    /**
     * Returns the localized name of the specified action of the object.
     * Called from native code via JNI.
     *
     * @param index the action index corresponding to the action
     * @return a localized name string, or null if the action does not exist
     */
    private String getLocalizedName(int index) {
        AccessibleContext accessibleContext = accessibleContextWeakRef.get();
        if (accessibleContext == null)
            return null;
        AccessibleAction accessibleAction = accessibleActionWeakRef.get();
        if (accessibleAction == null)
            return null;

        if (index >= nactions) {
            return null;
        }
        if (descriptions[index] != null) {
            return descriptions[index];
        }
        return AtkUtil.invokeInSwing(() -> {
            descriptions[index] = accessibleAction.getAccessibleActionDescription(index);
            if (descriptions[index] != null)
                return descriptions[index];
            String name = accessibleContext.getAccessibleName();
            if (name != null)
                return name;
            descriptions[index] = "";
            return descriptions[index];
        }, null);
    }

    private String get_keybinding(int index) {
        AccessibleExtendedComponent acc_ext_component;
        if (_acc_ext_component == null)
            return "";

        acc_ext_component = _acc_ext_component.get();

        // TODO: improve/fix conversion to strings, concatenate,
        //       and follow our formatting convention for the role of
        //       various keybindings (i.e. global, transient, etc.)

        //
        // Presently, JAA doesn't define a relationship between the index used
        // and the action associated. As such, all keybindings are only
        // associated with the default (index 0 in GNOME) action.
        //
        if (index > 0) {
            return "";
        }

        if (acc_ext_component != null) {
            AccessibleKeyBinding akb = acc_ext_component.getAccessibleKeyBinding();

            if (akb != null && akb.getAccessibleKeyBindingCount() > 0) {
                String rVal = "";
                int i;

                // Privately Agreed interface with StarOffice to workaround
                // deficiency in JAA.
                //
                // The aim is to use an array of keystrokes, if there is more
                // than one keypress involved meaning that we would have:
                //
                //	KeyBinding(0)    -> nmeumonic       KeyStroke
                //	KeyBinding(1)    -> full key path   KeyStroke[]
                //	KeyBinding(2)    -> accelerator     KeyStroke
                //
                // GNOME Expects a string in the format:
                //
                //	<nmemonic>;<full-path>;<accelerator>
                //
                // The keybindings in <full-path> should be separated by ":"
                //
                // Since only the first three are relevant, ignore others
                for (i = 0; (i < akb.getAccessibleKeyBindingCount() && i < 3); i++) {
                    Object o = akb.getAccessibleKeyBinding(i);

                    if (i > 0) {
                        rVal += ";";
                    }

                    if (o instanceof KeyStroke keyStroke) {
                        String modString = InputEvent.getModifiersExText(keyStroke.getModifiers());
                        String keyString = KeyEvent.getKeyText(keyStroke.getKeyCode());

                        if (keyString != null) {
                            if (modString != null && modString.length() > 0) {
                                rVal += convertModString(modString) + keyString;
                            } else {
                                rVal += keyString;
                            }
                        }
                    } else if (o instanceof KeyStroke[] keyStroke) {
                        for (int j = 0; j < keyStroke.length; j++) {
                            String modString = InputEvent.getModifiersExText(keyStroke[j].getModifiers());
                            String keyString = KeyEvent.getKeyText(keyStroke[j].getKeyCode());

                            if (j > 0) {
                                rVal += ":";
                            }

                            if (keyString != null) {
                                if (modString != null && modString.length() > 0) {
                                    rVal += convertModString(modString) + keyString;
                                } else {
                                    rVal += keyString;
                                }
                            }
                        }
                    }
                }

                if (i < 2) rVal += ";";
                if (i < 3) rVal += ";";

                return rVal;
            }
        }

        return "";
    }
}
