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

public class AtkHypertext extends AtkText {

	AccessibleHypertext acc_hyper_text;

	public AtkHypertext (AccessibleContext ac) {
		super(ac);

		AccessibleText ac_text = ac.getAccessibleText();
		if (ac_text instanceof AccessibleHypertext) {
			acc_hyper_text = (AccessibleHypertext)ac_text;
		} else {
			acc_hyper_text = null;
		}
	}

	public AtkHyperlink get_link (int link_index) {
		if (acc_hyper_text != null) {
			AccessibleHyperlink link = acc_hyper_text.getLink(link_index);
			if (link != null) {
				return new AtkHyperlink(link);
			}
		}

		return null;
	}

	public int get_n_links () {
		int nLinks = 0;

		if (acc_hyper_text != null) {
			nLinks = acc_hyper_text.getLinkCount();
		}

		return nLinks;
	}

	public int get_link_index (int char_index) {
		int index = 0;

		if (acc_hyper_text != null) {
			index = acc_hyper_text.getLinkIndex(char_index);
		}

		return index;
	}
}

