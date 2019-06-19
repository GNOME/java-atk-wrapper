/*
 * Copyright (c) 2018, Red Hat, Inc.
 * Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Oracle designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Oracle in the LICENSE file that accompanied this code.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

package org.GNOME.Accessibility;

import javax.accessibility.*;
import java.util.Locale;

/**
* AtkObject:
*   That class is used to wrap AccessibleContext Java object
*   to avoid the concurrency of AWT objects.
* @autor Giuseppe Capaldo
*/
public class AtkObject{

    public static AccessibleContext getAccessibleParent(AccessibleContext ac){
        return AtkUtil.invokeInSwing( () -> {
            Accessible father = ac.getAccessibleParent();
            if (father != null)
                return father.getAccessibleContext();
            else
                return null;
        }, null);
    }

    public static void setAccessibleParent(AccessibleContext ac, AccessibleContext pa){
        AtkUtil.invokeInSwing( () -> {
            if (pa instanceof Accessible){
                Accessible father = (Accessible) pa;
                ac.setAccessibleParent(father);
            }
        } );
    }

    public static String getAccessibleName(AccessibleContext ac){
        return AtkUtil.invokeInSwing( () -> { return ac.getAccessibleName(); }, "");
    }

    public static void setAccessibleName(AccessibleContext ac, String name){
        AtkUtil.invokeInSwing( () -> { ac.setAccessibleName(name); } );
    }

    public static String getAccessibleDescription(AccessibleContext ac){
        return AtkUtil.invokeInSwing( () -> { return ac.getAccessibleDescription(); }, "");
    }

    public static void setAccessibleDescription(AccessibleContext ac, String description){
        AtkUtil.invokeInSwing( () -> { ac.setAccessibleDescription(description); } );
    }

    public static int getAccessibleChildrenCount(AccessibleContext ac){
        return AtkUtil.invokeInSwing( () -> { return ac.getAccessibleChildrenCount(); }, 0);
    }

    public static int getAccessibleIndexInParent(AccessibleContext ac){
        return AtkUtil.invokeInSwing( () -> { return ac.getAccessibleIndexInParent(); }, -1);
    }

    public static AccessibleRole getAccessibleRole(AccessibleContext ac){
        return AtkUtil.invokeInSwing( () -> { return ac.getAccessibleRole(); }, AccessibleRole.UNKNOWN);
    }

    public static boolean equalsIgnoreCaseLocaleWithRole(AccessibleRole role){
        String displayString = role.toDisplayString(Locale.US);
        return displayString.equalsIgnoreCase("paragraph");
    }

    public static AccessibleState[] getArrayAccessibleState(AccessibleContext ac){
        return AtkUtil.invokeInSwing( () -> {
            AccessibleStateSet stateSet = ac.getAccessibleStateSet();
            if (stateSet == null)
                return null;
            else
                return stateSet.toArray();
        }, null);
    }

    public static Locale getLocale(AccessibleContext ac){
        return AtkUtil.invokeInSwing( () -> { return ac.getLocale(); }, null);
    }

    public static AccessibleContext getAccessibleChild(AccessibleContext ac, int i){
        return AtkUtil.invokeInSwing( () -> {
            Accessible child = ac.getAccessibleChild(i);
            if (child == null)
                return null;
            else
                return child.getAccessibleContext();
        }, null);
    }

    public static int hashCode(AccessibleContext ac){
        return AtkUtil.invokeInSwing( () -> { return ac.hashCode(); }, 0);
    }

}
