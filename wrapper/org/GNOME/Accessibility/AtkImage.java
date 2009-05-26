/*
 * Java ATK Wrapper for GNOME
 * Copyright 2009 Sun Microsystems Inc.
 *
 * This file is part of Java ATK Wrapper.

 * Java ATK Wrapper is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Java ATK Wrapper is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public License
 * along with Java ATK Wrapper.  If not, see <http://www.gnu.org/licenses/>.
 */

package org.GNOME.Accessibility;

import javax.accessibility.*;
import java.awt.Point;
import java.awt.Dimension;

public class AtkImage {

	AccessibleContext ac;
	AccessibleIcon[] acc_icons;

	public AtkImage (AccessibleContext ac) {
		super();
		this.ac = ac;
		this.acc_icons = ac.getAccessibleIcon();
	}
	
	public Point get_image_position (int coord_type) {
		AccessibleComponent acc_component = ac.getAccessibleComponent();
		if (acc_component == null) {
			return null;
		}

		if (coord_type == AtkCoordType.SCREEN) {
			return acc_component.getLocationOnScreen();
		}

		return acc_component.getLocation();
	}

	public String get_image_description () {
		String desc = "";
		if (acc_icons != null && acc_icons.length > 0) {
			desc = acc_icons[0].getAccessibleIconDescription();

			if (desc == null) {
				desc = "";
			}
		}

		return desc;
	}

	public Dimension get_image_size () {
		Dimension d = new Dimension(0, 0);
		if (acc_icons != null && acc_icons.length > 0) {
			d.height = acc_icons[0].getAccessibleIconHeight();
			d.width = acc_icons[0].getAccessibleIconWidth();
		}

		return d;
	}
}

