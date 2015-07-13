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
import java.awt.event.KeyEvent;

public class AtkAction {

	AccessibleContext ac;
	AccessibleAction acc_action;
	AccessibleExtendedComponent acc_ext_component;

	public AtkAction (AccessibleContext ac) {
		super();
		this.ac = ac;
		this.acc_action = ac.getAccessibleAction();
		AccessibleComponent acc_component = ac.getAccessibleComponent();
		if (acc_component instanceof AccessibleExtendedComponent) {
			this.acc_ext_component = (AccessibleExtendedComponent)acc_component;
		}
	}

	private class ActionRunner implements Runnable {
		private AccessibleAction acc_action;
		private int index;

		public ActionRunner (AccessibleAction acc_action, int index) {
			this.acc_action = acc_action;
			this.index = index;
		}

		public void run () {
			acc_action.doAccessibleAction(index);
		}
	}
	
	public boolean do_action (int i) {
		SwingUtilities.invokeLater(new ActionRunner(acc_action, i));
		return true;
	}

	public int get_n_actions () {
		return acc_action.getAccessibleActionCount();
	}

	public String get_description (int i) {
		String description = "<description>";
		return description;
	}

        public boolean setDescription(int i, String description) {
               if (description ==  acc_action.getAccessibleActionDescription(i) &&
                   description != null)
                     return true;
               return false;
        }

	public String get_name (int i) {
		String name = acc_action.getAccessibleActionDescription(i);
		if (name == null) {
			name = " ";
		}

		return name;
	}

 /**
  * @param i an integer holding the index of the name of
  *          the accessible.
  * @return  the localized name of the object or otherwise,
  *          null if the "action" object does not have a
  *          name (really, java's AccessibleAction class
  *          does not provide
  *          a getter for an AccessibleAction
  *          name so a getter from the AcccessibleContext
  *          class is one way to work around that)
  */
  public String getLocalizedName (int i) {
    String name        = ac.getAccessibleName();
    String description = acc_action.getAccessibleActionDescription(i);

    if (description == name && description != null)
      return description;
    if (description == null && name != null)
      return name;
    else if (description != null)
      return description;

    return null;
  }

	private String convertModString (String mods) {
		if (mods == null || mods.length() == 0) {
			return "";
		}

		String modStrs[] = mods.split("\\+");
		String newModString = "";
		for (int i = 0; i < modStrs.length; i++) {
			newModString += "<" + modStrs[i] + ">";
		}
		
		return newModString;
	}

	public String get_keybinding (int index) {
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
		
		if(acc_ext_component != null) {
			AccessibleKeyBinding akb = acc_ext_component.getAccessibleKeyBinding();
			
			if (akb != null && akb.getAccessibleKeyBindingCount() > 0) {
				String  rVal = "";
				int     i;

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
				for (i = 0;( i < akb.getAccessibleKeyBindingCount() && i < 3); i++) {
					Object o = akb.getAccessibleKeyBinding(i);
					
					if ( i > 0 ) {
						rVal += ";";
					}
					
					if (o instanceof javax.swing.KeyStroke) {
						javax.swing.KeyStroke keyStroke = (javax.swing.KeyStroke)o;
						String modString = KeyEvent.getKeyModifiersText(keyStroke.getModifiers());
						String keyString = KeyEvent.getKeyText(keyStroke.getKeyCode());
						
						if ( keyString != null ) {
							if ( modString != null && modString.length() > 0 ) {
								rVal += convertModString(modString) + keyString;
							} else {
								rVal += keyString;
							}
						}
					} else if (o instanceof javax.swing.KeyStroke[]) {
						javax.swing.KeyStroke[] keyStroke = (javax.swing.KeyStroke[])o;
						for ( int j = 0; j < keyStroke.length; j++ ) {
							String modString = KeyEvent.getKeyModifiersText(keyStroke[j].getModifiers());
							String keyString = KeyEvent.getKeyText(keyStroke[j].getKeyCode());
							
							if (j > 0) {
								rVal += ":";
							}
							
							if ( keyString != null ) {
								if (modString != null && modString.length() > 0) {
									rVal += convertModString(modString) + keyString;
								} else {
									rVal += keyString;
								}
							}
						}
					}
				}
				
				if ( i < 2 ) rVal += ";";
				if ( i < 3 ) rVal += ";";
				
				return rVal;
			}
		}
		
		return "";
	}
}

