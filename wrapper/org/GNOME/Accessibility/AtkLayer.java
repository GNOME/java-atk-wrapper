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

public interface AtkLayer {
	public int INVALID = 0;
	public int BACKGROUND = 1;
	public int CANVAS = 2;
	public int WIDGET = 3;
	public int MDI = 4;
	public int POPUP = 5;
	public int OVERLAY = 6;
	public int WINDOW = 7;
}

