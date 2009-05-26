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

