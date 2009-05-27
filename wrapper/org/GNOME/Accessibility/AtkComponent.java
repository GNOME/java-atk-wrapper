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
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.Dimension;

public class AtkComponent {

	AccessibleContext ac;
	AccessibleComponent acc_component;

	public AtkComponent (AccessibleContext ac) {
		super();
		this.ac = ac;
		this.acc_component = ac.getAccessibleComponent();
	}

	public boolean contains (int x, int y, int coord_type) {
		if (coord_type == AtkCoordType.SCREEN) {
			Point p = acc_component.getLocationOnScreen();
			x -= p.x;
			y -= p.y;
		}

		return acc_component.contains(new Point(x, y));
	}

	public AccessibleContext get_accessible_at_point (int x, int y, int coord_type) {
		if (coord_type == AtkCoordType.SCREEN) {
			Point p = acc_component.getLocationOnScreen();
			x -= p.x;
			y -= p.y;
		}

		javax.accessibility.Accessible accessible = acc_component.getAccessibleAt(new Point(x, y));
		if (accessible == null) {
			return null;
		}

		return accessible.getAccessibleContext();
	}

	public Point get_position (int coord_type) {
		if (coord_type == AtkCoordType.SCREEN) {
			return acc_component.getLocationOnScreen();
		}

		return acc_component.getLocation();
	}

	public Dimension get_size () {
		return acc_component.getSize();
	}

	public boolean grab_focus () {
		if (!acc_component.isFocusTraversable()) {
			return false;
		}

		acc_component.requestFocus();
		return true;
	}

	public int get_layer () {
		AccessibleRole role = ac.getAccessibleRole();

		if (role == AccessibleRole.MENU ||
			role == AccessibleRole.MENU_ITEM ||
			role == AccessibleRole.POPUP_MENU ) {
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
			role == AccessibleRole.LAYERED_PANE ) {
			return AtkLayer.CANVAS;
		}

		return AtkLayer.WIDGET;
	}
}

