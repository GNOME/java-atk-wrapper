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

  public static AtkComponent createAtkComponent(AccessibleContext ac){
      return AtkUtil.invokeInSwing ( () -> { return new AtkComponent(ac); }, null);
  }

  public boolean contains (int x, int y, int coord_type) {
      return AtkUtil.invokeInSwing ( () -> {
          if(!acc_component.isVisible()){
              final int rightX;
              final int rightY;
              if (coord_type == AtkCoordType.SCREEN) {
                  Point p = acc_component.getLocationOnScreen();
                  rightX = x - p.x;
                  rightY = y - p.y;
              }
              else{
                  rightX = x;
                  rightY = y;
              }
              return acc_component.contains(new Point(rightX, rightY));
          }
          return false;
      }, false);
  }

  public AccessibleContext get_accessible_at_point (int x, int y, int coord_type) {
      return AtkUtil.invokeInSwing ( () -> {
           if(acc_component.isVisible()){
              final int rightX;
              final int rightY;
              if (coord_type == AtkCoordType.SCREEN) {
                  Point p = acc_component.getLocationOnScreen();
                  rightX = x - p.x;
                  rightY = y - p.y;
              }
              else{
                  rightX = x;
                  rightY = y;
              }
              Accessible accessible = acc_component.getAccessibleAt(new Point(rightX, rightY));
              if (accessible == null)
                  return null;
              return accessible.getAccessibleContext();
          }
          return null;
      }, null);
  }

    public boolean grab_focus () {
        return AtkUtil.invokeInSwing ( () -> {
            if (!acc_component.isFocusTraversable())
                return false;
            acc_component.requestFocus();
            return true;
        }, false);
    }

    public boolean set_extents(int x, int y, int width, int height, int coord_type) {
        AtkUtil.invokeInSwing( () -> {
            if(acc_component.isVisible()){
                final int rightX;
                final int rightY;
                if (coord_type == AtkCoordType.SCREEN) {
                    Point p = acc_component.getLocationOnScreen();
                    rightX = x - p.x;
                    rightY = y - p.y;
                }
                else{
                    rightX = x;
                    rightY = y;
                }
                acc_component.setBounds(new Rectangle(rightX, rightY, width, height));
                return true;
            }
            return false;
        }, false);
    }

    public Rectangle get_extents() {
        return AtkUtil.invokeInSwing ( () -> {
            if(acc_component.isVisible()){
                Rectangle rect = acc_component.getBounds();
                Point p = acc_component.getLocationOnScreen();
                rect.x = p.x;
                rect.y = p.y;
                return rect;
            }
            return null;
        },null);
    }

    public int get_layer () {
        return AtkUtil.invokeInSwing ( () -> {
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
            if (role == AccessibleRole.WINDOW) {
                return AtkLayer.WINDOW;
            }
            return AtkLayer.WIDGET;
        }, AtkLayer.INVALID);
    }

}
