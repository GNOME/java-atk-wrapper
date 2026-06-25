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

import javax.accessibility.AccessibleContext;
import javax.accessibility.AccessibleValue;
import java.lang.ref.WeakReference;

public class AtkValue {

    WeakReference<AccessibleValue> accessibleValueWeakRef;

    public AtkValue(AccessibleContext ac) {
        super();
        this.accessibleValueWeakRef = new WeakReference<AccessibleValue>(ac.getAccessibleValue());
    }

    public static AtkValue createAtkValue(AccessibleContext ac) {
        return AtkUtil.invokeInSwing(() -> {
            return new AtkValue(ac);
        }, null);
    }

    public Number get_current_value() {
        AccessibleValue accessibleValue = accessibleValueWeakRef.get();
        if (accessibleValue == null)
            return 0.0;

        return AtkUtil.invokeInSwing(() -> {
            return accessibleValue.getCurrentAccessibleValue();
        }, 0.0);
    }

    public double getMaximumValue() {
        AccessibleValue accessibleValue = accessibleValueWeakRef.get();
        if (accessibleValue == null)
            return 0.0;

        return AtkUtil.invokeInSwing(() -> {
            return accessibleValue.getMaximumAccessibleValue().doubleValue();
        }, 0.0);
    }

    public double getMinimumValue() {
        AccessibleValue accessibleValue = accessibleValueWeakRef.get();
        if (accessibleValue == null)
            return 0.0;

        return AtkUtil.invokeInSwing(() -> {
            return accessibleValue.getMinimumAccessibleValue().doubleValue();
        }, 0.0);
    }

    public void setValue(Number n) {
        AccessibleValue accessibleValue = accessibleValueWeakRef.get();
        if (accessibleValue == null)
            return;

        AtkUtil.invokeInSwing(() -> {
            accessibleValue.setCurrentAccessibleValue(n);
        });
    }

    public double getIncrement() {
        return Double.MIN_VALUE;
    }
}
