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

	public static AtkHyperlink createAtkHyperlink(AccessibleHyperlink hl){
		return AtkUtil.invokeInSwing ( () -> { return new AtkHyperlink(hl); }, null);
	}

	public String get_uri (int i) {
		return AtkUtil.invokeInSwing ( () -> {
			Object o = acc_hyperlink.getAccessibleActionObject(i);
			if (o != null)
				return o.toString();
			return "";
		}, "");
	}

	public AccessibleContext get_object (int i) {
		return AtkUtil.invokeInSwing ( () -> {
			Object anchor = acc_hyperlink.getAccessibleActionAnchor(i);
			if (anchor instanceof Accessible)
				return ((Accessible)anchor).getAccessibleContext();
			return null;
		}, null);
	}

	public int get_end_index () {
		return AtkUtil.invokeInSwing ( () -> { return acc_hyperlink.getEndIndex(); }, 0);
	}

	public int get_start_index () {
		return AtkUtil.invokeInSwing ( () -> { return acc_hyperlink.getStartIndex(); }, 0);
	}

	public boolean is_valid () {
		return AtkUtil.invokeInSwing ( () -> { return acc_hyperlink.isValid(); }, false);
	}

	public int get_n_anchors () {
		return AtkUtil.invokeInSwing ( () -> { return acc_hyperlink.getAccessibleActionCount(); }, 0);
	}
}
