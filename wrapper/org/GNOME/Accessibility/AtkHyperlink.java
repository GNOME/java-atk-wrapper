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

public class AtkHyperlink {
	
	AccessibleHyperlink acc_hyperlink;

	public AtkHyperlink (AccessibleHyperlink hl) {
		super();
		acc_hyperlink = hl;
	}

	public String get_uri (int i) {
		String s = "";
		Object o = acc_hyperlink.getAccessibleActionObject(i);
		if (o != null) {
			s = o.toString();
		}

		return s;
	}

	public Object get_object (int i) {
		Object o = null;
		Object anchor = acc_hyperlink.getAccessibleActionAnchor(i);
		if (anchor instanceof javax.accessibility.Accessible) {
			o = anchor;
		}

		return o;
	}

	public int get_end_index () {
		return acc_hyperlink.getEndIndex();
	}

	public int get_start_index () {
		return acc_hyperlink.getStartIndex();
	}

	public boolean is_valid () {
		return acc_hyperlink.isValid();
	}

	public int get_n_anchors () {
		return acc_hyperlink.getAccessibleActionCount();
	}
}

