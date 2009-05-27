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

public class AtkValue {

	AccessibleContext ac;
	AccessibleValue acc_value;

	public AtkValue (AccessibleContext ac) {
		super();
		this.acc_value = ac.getAccessibleValue();
	}

	public Number get_current_value () {
		return acc_value.getCurrentAccessibleValue();
	}

	public Number get_maximum_value () {
		return acc_value.getMaximumAccessibleValue();
	}

	public Number get_minimum_value () {
		return acc_value.getMinimumAccessibleValue();
	}

	public boolean set_current_value (Number n) {
		return acc_value.setCurrentAccessibleValue(n);
	}
}

