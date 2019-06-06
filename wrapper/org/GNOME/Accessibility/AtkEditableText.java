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
import java.awt.Toolkit;
import java.awt.datatransfer.StringSelection;
import javax.swing.text.*;
import javax.swing.*;

public class AtkEditableText extends AtkText {

  AccessibleEditableText acc_edt_text;

  public AtkEditableText (AccessibleContext ac) {
    super(ac);
    acc_edt_text = ac.getAccessibleEditableText();
  }

    private class TextContentsRunner implements Runnable {
       private AccessibleEditableText acc_edt_text;
       private String s;

       public TextContentsRunner (AccessibleEditableText acc_edt_text, String s) {
           this.acc_edt_text = acc_edt_text;
           this.s = s;
       }

       public void run () {
           acc_edt_text.setTextContents(s);
       }
   }

  public void set_text_contents (String s) {
     SwingUtilities.invokeLater(new TextContentsRunner(acc_edt_text, s));
  }

    private class insertTextAtIndexRunner implements Runnable {
        private AccessibleEditableText acc_edt_text;
        private int position;
        private String s;

        public insertTextAtIndexRunner (AccessibleEditableText acc_edt_text, int position, String s) {
            this.acc_edt_text = acc_edt_text;
            this.position = position;
            this.s = s;
        }

        public void run () {
            acc_edt_text.insertTextAtIndex(position, s);
        }
    }

  public void insert_text (String s, int position) {
    if (position < 0) {
      position = 0;
    }

    SwingUtilities.invokeLater(new insertTextAtIndexRunner(acc_edt_text, position, s));
  }

  public void copy_text (int start, int end) {
    int n = acc_edt_text.getCharCount();

    if (start < 0) {
      start = 0;
    }

    if (end > n || end == -1) {
      end = n;
    } else if (end < -1) {
      end = 0;
    }

    String s = acc_edt_text.getTextRange(start, end);
    if (s != null) {
      StringSelection stringSel = new StringSelection(s);
      Toolkit.getDefaultToolkit().getSystemClipboard().setContents(stringSel,
                                                                   stringSel
                                                                   );
    }
  }

    private class CutRunner implements Runnable {
        private AccessibleEditableText acc_edt_text;
        private int start;
        private int end;

        public CutRunner (AccessibleEditableText acc_edt_text, int start, int end) {
            this.acc_edt_text = acc_edt_text;
            this.start = start;
            this.end = end;
        }

        public void run () {
            acc_edt_text.cut(start, end);
        }
    }

  public void cut_text (int start, int end) {
    SwingUtilities.invokeLater(new CutRunner(acc_edt_text, start, end));
  }

    private class DeleteRunner implements Runnable {
        private AccessibleEditableText acc_edt_text;
        private int start;
        private int end;

        public DeleteRunner(AccessibleEditableText acc_edt_text, int start, int end) {
            this.acc_edt_text = acc_edt_text;
            this.start = start;
            this.end = end;
        }

        public void run () {
            acc_edt_text.delete(start, end);
        }
    }

  public void delete_text (int start, int end) {
    SwingUtilities.invokeLater(new DeleteRunner(acc_edt_text, start, end));
  }

    private class PasteRunner implements Runnable {
        private AccessibleEditableText acc_edt_text;
        private int position;

        public PasteRunner (AccessibleEditableText acc_edt_text, int position) {
            this.acc_edt_text = acc_edt_text;
            this.position = position;
        }

        public void run () {
            acc_edt_text.paste(position);
        }
    }

  public void paste_text (int position) {
    SwingUtilities.invokeLater(new PasteRunner(acc_edt_text, position));
  }

    private class SetAttributesRunner implements Runnable {
       private AccessibleEditableText acc_edt_text;
       AttributeSet as;
       private int start;
       private int end;

       public SetAttributesRunner (AccessibleEditableText acc_edt_text, int start, int end, AttributeSet as) {
           this.acc_edt_text = acc_edt_text;
           this.start = start;
           this.end = end;
           this.as = as;
       }

       public void run () {
           acc_edt_text.setAttributes(start, end, as);
       }
   }

 /**
  * Sets run attributes for the text between two indices.
  *
  * @param as the AttributeSet for the text
  * @param start the start index of the text as an int
  * @param end the end index for the text as an int
  * @return whether setRunAttributes was called
  *              TODO return is a bit presumptious. This should ideally include a check for whether
  *              attributes were set.
  */
  public boolean setRunAttributes(AttributeSet as, int start, int end) {
    SwingUtilities.invokeLater(new SetAttributesRunner(acc_edt_text, start, end, as));
    return true;
  }
}
