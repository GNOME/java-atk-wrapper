/*
 * Java ATK Wrapper for GNOME
 * Copyright (C) 2009 Sun Microsystems Inc.
 * Copyright (C) 2015 Magdalen Berns <m.berns@thismagpie.com>
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
import javax.accessibility.AccessibleComponent;
import javax.accessibility.AccessibleContext;
import javax.accessibility.AccessibleRole;
import java.awt.*;
import java.lang.ref.WeakReference;

/**
 * The ATK Component interface implementation for Java accessibility.
 * <p>
 * This class provides a bridge between Java's AccessibleComponent interface
 * and the ATK (Accessibility Toolkit) component interface.
 */
public class AtkComponent {

    private final WeakReference<AccessibleContext> accessibleContextWeakRef;
    private final WeakReference<AccessibleComponent> accessibleComponentWeakRef;

    public AtkComponent(AccessibleContext ac) {
        super();
        this.accessibleContextWeakRef = new WeakReference<AccessibleContext>(ac);
        this.accessibleComponentWeakRef =
                new WeakReference<AccessibleComponent>(ac.getAccessibleComponent());
    }

    static public Point getWindowLocation(AccessibleContext ac) {
        while (ac != null) {
            AccessibleRole role = ac.getAccessibleRole();
            if (role == AccessibleRole.DIALOG ||
                    role == AccessibleRole.FRAME ||
                    role == AccessibleRole.WINDOW) {
                AccessibleComponent acc_comp = ac.getAccessibleComponent();
                if (acc_comp == null)
                    return null;
                return acc_comp.getLocationOnScreen();
            }
            Accessible parent = ac.getAccessibleParent();
            if (parent == null)
                return null;
            ac = parent.getAccessibleContext();
        }
        return null;
    }

    // Return the position of the object relative to the coordinate type
    public static Point getComponentOrigin(
            AccessibleContext ac, AccessibleComponent accessibleComponent, int coordType) {
        if (coordType == AtkCoordType.SCREEN)
            return accessibleComponent.getLocationOnScreen();

        if (coordType == AtkCoordType.WINDOW) {
            Point win_p = getWindowLocation(ac);
            if (win_p == null)
                return null;
            Point p = accessibleComponent.getLocationOnScreen();
            if (p == null)
                return null;
            p.translate(-win_p.x, -win_p.y);
            return p;
        }

        if (coordType == AtkCoordType.PARENT)
            return accessibleComponent.getLocation();

        return null;
    }

    // Return the position of the parent relative to the coordinate type
    public static Point getParentOrigin(
            AccessibleContext ac, AccessibleComponent accessibleComponent, int coordType) {
        if (coordType == AtkCoordType.PARENT)
            return new Point(0, 0);

        Accessible parent = ac.getAccessibleParent();
        if (parent == null)
            return null;
        AccessibleContext parentAccessibleContext = parent.getAccessibleContext();
        if (parentAccessibleContext == null)
            return null;
        AccessibleComponent parentComponent = parentAccessibleContext.getAccessibleComponent();
        if (parentComponent == null)
            return null;

        if (coordType == AtkCoordType.SCREEN) {
            return parentComponent.getLocationOnScreen();
        }

        if (coordType == AtkCoordType.WINDOW) {
            Point window_origin = getWindowLocation(ac);
            if (window_origin == null)
                return null;
            Point parent_origin = parentComponent.getLocationOnScreen();
            if (parent_origin == null)
                return null;
            parent_origin.translate(-window_origin.x, -window_origin.y);
            return parent_origin;
        }
        return null;
    }

    // JNI upcalls section

    /**
     * Factory method to create an AtkComponent instance from an AccessibleContext.
     * Called from native code via JNI.
     *
     * @param ac the AccessibleContext to wrap
     * @return a new AtkComponent instance, or null if creation fails
     */
    public static AtkComponent createAtkComponent(AccessibleContext ac) {
        return AtkUtil.invokeInSwing(() -> {
            return new AtkComponent(ac);
        }, null);
    }

    /**
     * Checks whether the specified point is within the extent of the component.
     * Called from native code via JNI.
     *
     * @param x         x coordinate
     * @param y         y coordinate
     * @param coordType specifies whether the coordinates are relative to the screen,
     *                  the component's toplevel window, or the component's parent
     * @return true if the specified point is within the extent of the component
     */
    public boolean contains(int x, int y, int coordType) {
        AccessibleContext accessibleContext = accessibleContextWeakRef.get();
        if (accessibleContext == null)
            return false;
        AccessibleComponent accessibleComponent = accessibleComponentWeakRef.get();
        if (accessibleComponent == null)
            return false;

        return AtkUtil.invokeInSwing(() -> {
            if (accessibleComponent.isVisible()) {
                Point p = getComponentOrigin(accessibleContext, accessibleComponent, coordType);
                if (p == null)
                    return false;

                return accessibleComponent.contains(new Point(x - p.x, y - p.y));
            }
            return false;
        }, false);
    }

    /**
     * Gets a reference to the accessible child, if one exists, at the coordinate point
     * specified by x and y.
     * Called from native code via JNI.
     *
     * @param x         x coordinate
     * @param y         y coordinate
     * @param coordType specifies whether the coordinates are relative to the screen,
     *                  the component's toplevel window, or the component's parent
     * @return the AccessibleContext of the child at the specified point, or null if none exists
     */
    public AccessibleContext get_accessible_at_point(int x, int y, int coordType) {
        AccessibleContext accessibleContext = accessibleContextWeakRef.get();
        if (accessibleContext == null)
            return null;
        AccessibleComponent accessibleComponent = accessibleComponentWeakRef.get();
        if (accessibleComponent == null)
            return null;

        return AtkUtil.invokeInSwing(() -> {
            if (accessibleComponent.isVisible()) {
                Point p = getComponentOrigin(accessibleContext, accessibleComponent, coordType);
                if (p == null)
                    return null;

                Accessible accessible =
                        accessibleComponent.getAccessibleAt(new Point(x - p.x, y - p.y));
                if (accessible == null)
                    return null;
                return accessible.getAccessibleContext();
            }
            return null;
        }, null);
    }

    /**
     * Grabs focus for this component.
     * Called from native code via JNI.
     *
     * @return true if successful, false otherwise
     */
    public boolean grab_focus() {
        AccessibleComponent accessibleComponent = accessibleComponentWeakRef.get();
        if (accessibleComponent == null)
            return false;

        return AtkUtil.invokeInSwing(() -> {
            if (!accessibleComponent.isFocusTraversable())
                return false;
            accessibleComponent.requestFocus();
            return true;
        }, false);
    }

    /**
     * Sets the extents of the component.
     * Called from native code via JNI.
     *
     * @param x         x coordinate
     * @param y         y coordinate
     * @param width     width to set for the component
     * @param height    height to set for the component
     * @param coordType specifies whether the coordinates are relative to the screen,
     *                  the component's toplevel window, or the component's parent
     * @return true if the extents were set successfully, false otherwise
     */
    public boolean set_extents(int x, int y, int width, int height, int coordType) {
        AccessibleContext accessibleContext = accessibleContextWeakRef.get();
        if (accessibleContext == null)
            return false;
        AccessibleComponent accessibleComponent = accessibleComponentWeakRef.get();
        if (accessibleComponent == null)
            return false;

        return AtkUtil.invokeInSwing(() -> {
            if (accessibleComponent.isVisible()) {
                Point p = getParentOrigin(accessibleContext, accessibleComponent, coordType);
                if (p == null)
                    return false;

                accessibleComponent.setBounds(new Rectangle(x - p.x, y - p.y, width, height));
                return true;
            }
            return false;
        }, false);
    }

    /**
     * Gets the rectangle which gives the extent of the component.
     * Called from native code via JNI.
     *
     * @param coordType specifies whether the coordinates are relative to the screen,
     *                  the component's toplevel window, or the component's parent
     * @return the Rectangle representing the component's extent, or null if it cannot be obtained
     */
    public Rectangle get_extents(int coordType) {
        AccessibleContext accessibleContext = accessibleContextWeakRef.get();
        if (accessibleContext == null)
            return null;
        AccessibleComponent accessibleComponent = accessibleComponentWeakRef.get();
        if (accessibleComponent == null)
            return null;

        return AtkUtil.invokeInSwing(() -> {
            if (accessibleComponent.isVisible()) {
                Rectangle rect = accessibleComponent.getBounds();
                if (rect == null)
                    return null;
                Point p = getParentOrigin(accessibleContext, accessibleComponent, coordType);
                if (p == null)
                    return null;

                rect.x += p.x;
                rect.y += p.y;
                return rect;
            }
            return null;
        }, null);
    }

    /**
     * Gets the AtkLayer of the component based on AccessibleRole.
     * Called from native code via JNI.
     *
     * @return an int representing the AtkLayer of the component, or AtkLayer.INVALID if an error occurs
     */
    public int get_layer() {
        AccessibleContext accessibleContext = accessibleContextWeakRef.get();
        if (accessibleContext == null)
            return AtkLayer.INVALID;

        return AtkUtil.invokeInSwing(() -> {
            AccessibleRole role = accessibleContext.getAccessibleRole();
            if (role == AccessibleRole.MENU ||
                    role == AccessibleRole.MENU_ITEM ||
                    role == AccessibleRole.POPUP_MENU) {
                return AtkLayer.POPUP;
            }
            if (role == AccessibleRole.INTERNAL_FRAME) {
                return AtkLayer.MDI;
            }
            if (role == AccessibleRole.GLASS_PANE) {
                return AtkLayer.OVERLAY;
            }
            if (role == AccessibleRole.CANVAS ||
                    role == AccessibleRole.ROOT_PANE ||
                    role == AccessibleRole.LAYERED_PANE) {
                return AtkLayer.CANVAS;
            }
            if (role == AccessibleRole.WINDOW) {
                return AtkLayer.WINDOW;
            }
            return AtkLayer.WIDGET;
        }, AtkLayer.INVALID);
    }

}
