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
import java.text.*;
import java.awt.Rectangle;
import java.awt.Point;

public class AtkText {

	AccessibleContext ac;
	AccessibleText acc_text;

	public class StringSequence {

		public String str;
		public int start_offset, end_offset;

		public StringSequence (String str, int start_offset, int end_offset) {
			this.str = str;
			this.start_offset = start_offset;
			this.end_offset = end_offset;
		}
	}

	public AtkText (AccessibleContext ac) {
		super();
		this.ac = ac;
		this.acc_text = ac.getAccessibleText();
	}

	/* Return string from start, up to, but not including end */
	public String get_text (int start, int end) {
		int count = acc_text.getCharCount();
		if (start < 0)
			start = 0;

		if (end > count || end ==-1)
			end = count;
		if (end < -1)
			end = 0;

		if (acc_text instanceof AccessibleExtendedText) {
			AccessibleExtendedText acc_ext_text = (AccessibleExtendedText)acc_text;
			return acc_ext_text.getTextRange(start, end);
		}

		StringBuffer buf = new StringBuffer();
		for (int i = start; i <= end-1; i++) {
			String str = acc_text.getAtIndex(
					AccessibleText.CHARACTER, i);
			buf.append(str);
		}

		return buf.toString();
	}

	public char get_character_at_offset (int offset) {
		String str =  acc_text.getAtIndex(
				AccessibleText.CHARACTER, offset);

		if (str == null || str.length() == 0) {
			return 0;
		}

		return str.charAt(0);
	}

	public StringSequence get_text_at_offset (int offset,
				int boundary_type) {
		if (acc_text instanceof AccessibleExtendedText) {
			AccessibleExtendedText acc_ext_text = (AccessibleExtendedText)acc_text;

			int part = getPartTypeFromBoundary(boundary_type);
			if (part == -1) {
				return null;
			}

			AccessibleTextSequence seq = acc_ext_text.getTextSequenceAt(part, offset);
			return new StringSequence(seq.text, seq.startIndex, seq.endIndex+1);


		} else {
			return private_get_text_at_offset(offset, boundary_type);
		}
	}

	public int get_caret_offset () {
		return acc_text.getCaretPosition();
	}

	public Rectangle get_character_extents (int offset, int coord_type) {
		Rectangle rect = acc_text.getCharacterBounds(offset);
		if (rect == null) {
			return null;
		}

		if (coord_type == AtkCoordType.SCREEN) {
			AccessibleComponent component = ac.getAccessibleComponent();
			if (component == null) {
				return null;
			}

			Point p = component.getLocationOnScreen();
			rect.x += p.x;
			rect.y += p.y;
		}

		return rect;
	}

	public int get_character_count () {
		return acc_text.getCharCount();
	}

	public int get_offset_at_point (int x, int y,
				int coord_type) {
		if (coord_type == AtkCoordType.SCREEN) {
			AccessibleComponent component = ac.getAccessibleComponent();
			if (component == null) {
				return -1;
			}

			Point p = component.getLocationOnScreen();
			x -= p.x;
			y -= p.y;
		}

		return acc_text.getIndexAtPoint(new Point(x, y));
	}

	public Rectangle get_range_extents (int start, int end,
					int coord_type) {
		if (acc_text instanceof AccessibleExtendedText) {
			AccessibleExtendedText acc_ext_text = (AccessibleExtendedText)acc_text;
			Rectangle rect = acc_ext_text.getTextBounds(start, end-1);
			if (rect == null) {
				return null;
			}

			if (coord_type == AtkCoordType.SCREEN) {
				AccessibleComponent component = ac.getAccessibleComponent();
				if (component == null) {
					return null;
				}

				Point p = component.getLocationOnScreen();
				rect.x += p.x;
				rect.y += p.y;
			}

			return rect;
		} else {
			return null;
		}
	}

	public int get_n_selections () {
		String str = acc_text.getSelectedText();

		if (str != null && str.length() > 0) {
			return 1;
		} else {
			return 0;
		}
	}

	public StringSequence get_selection () {
		int start = acc_text.getSelectionStart();
		int end = acc_text.getSelectionEnd() + 1;
		String text = acc_text.getSelectedText();

		if (text == null) {
			return null;
		}

		return new StringSequence(text, start, end);
	}

	public boolean add_selection (int start, int end) {
		AccessibleEditableText acc_edt_text = ac.getAccessibleEditableText();

		if (acc_edt_text == null || get_n_selections() > 0) {
			return false;
		}

		return set_selection(0, start, end);
	}

	public boolean remove_selection(int selection_num) {
		AccessibleEditableText acc_edt_text = ac.getAccessibleEditableText();

		if (acc_edt_text == null || selection_num > 0) {
			return false;
		}

		acc_edt_text.selectText(0, 0);
		return true;
	}

	public boolean set_selection (int selection_num,
			int start, int end) {
		AccessibleEditableText acc_edt_text = ac.getAccessibleEditableText();

		if (acc_edt_text == null || selection_num > 0) {
			return false;
		}

		acc_edt_text.selectText(start, end-1);
		return true;
	}

	public boolean set_caret_offset (int offset) {
		AccessibleEditableText acc_edt_text = ac.getAccessibleEditableText();

		if (acc_edt_text != null) {
			acc_edt_text.selectText(offset, offset);
			return true;
		}

		return false;
	}

	private int getPartTypeFromBoundary (int boundary_type) {
		switch (boundary_type) {
			case AtkTextBoundary.CHAR :
				return AccessibleText.CHARACTER;
			case AtkTextBoundary.WORD_START :
			case AtkTextBoundary.WORD_END :
				return AccessibleText.WORD;
			case AtkTextBoundary.SENTENCE_START :
			case AtkTextBoundary.SENTENCE_END :
				return AccessibleText.SENTENCE;
			case AtkTextBoundary.LINE_START :
			case AtkTextBoundary.LINE_END :
				return AccessibleExtendedText.LINE;
			default :
				return -1;
		}
	}

	private int getNextWordStart (int offset, String str) {
		BreakIterator words = BreakIterator.getWordInstance();
		words.setText(str);
		int start = words.following(offset);
		int end = words.next();

		while (end != BreakIterator.DONE) {
			for (int i = start; i < end; i++) {
				if (Character.isLetter(str.codePointAt(i))) {
					return start;
				}
			}

			start = end;
			end = words.next();
		}

		return BreakIterator.DONE;
	}

	private int getNextWordEnd (int offset, String str) {
		int start = getNextWordStart(offset, str);

		BreakIterator words = BreakIterator.getWordInstance();
		words.setText(str);
		int next = words.following(offset);

		if (start == next) {
			return words.following(start);
		} else {
			return next;
		}
	}

	private int getPreviousWordStart (int offset, String str) {
		BreakIterator words = BreakIterator.getWordInstance();
		words.setText(str);
		int start = words.preceding(offset);
		int end = words.next();

		while (start != BreakIterator.DONE) {
			for (int i = start; i < end; i++) {
				if (Character.isLetter(str.codePointAt(i))) {
					return start;
				}
			}

			end = start;
			start = words.preceding(end);
		}

		return BreakIterator.DONE;
	}

	private int getPreviousWordEnd (int offset, String str) {
		int start = getPreviousWordStart(offset, str);

		BreakIterator words = BreakIterator.getWordInstance();
		words.setText(str);
		int pre = words.preceding(offset);

		if (start == pre) {
			return words.preceding(start);
		} else {
			return pre;
		}
	}

	private int getNextSentenceStart (int offset, String str) {
		BreakIterator sentences = BreakIterator.getSentenceInstance();
		sentences.setText(str);
		int start = sentences.following(offset);

		return start;
	}

	private int getNextSentenceEnd (int offset, String str) {
		int start = getNextSentenceStart(offset, str);
		if (start == BreakIterator.DONE) {
			return str.length();
		}

		int index = start;
		do {
			index --;
		} while (index >= 0 && Character.isWhitespace(str.charAt(index)));

		index ++;
		if (index < offset) {
			start = getNextSentenceStart(start, str);
			if (start == BreakIterator.DONE) {
				return str.length();
			}

			index = start;
			do {
				index --;
			} while (index >= 0 && Character.isWhitespace(str.charAt(index)));

			index ++;
		}

		return index;
	}

	private int getPreviousSentenceStart (int offset, String str) {
		BreakIterator sentences = BreakIterator.getSentenceInstance();
		sentences.setText(str);
		int start = sentences.preceding(offset);

		return start;
	}

	private int getPreviousSentenceEnd (int offset, String str) {
		int start = getPreviousSentenceStart(offset, str);
		if (start == BreakIterator.DONE) {
			return 0;
		}

		int end = getNextSentenceEnd(start, str);
		if (offset < end) {
			start = getPreviousSentenceStart(start, str);
			if (start == BreakIterator.DONE) {
				return 0;
			}

			end = getNextSentenceEnd(start, str);
		}

		return end;
	}

	private StringSequence private_get_text_at_offset (int offset,
			int boundary_type) {
		int char_count = get_character_count();
		if (offset < 0 || offset >= char_count) {
			return null;
		}

		switch (boundary_type) {
			case AtkTextBoundary.CHAR :
			{
				String str = get_text(offset, offset+1);
				return new StringSequence(str, offset, offset+1);
			}
			case AtkTextBoundary.WORD_START :
			{
				String s = get_text(0, get_character_count());
				int start = getPreviousWordStart(offset, s);
				if (start == BreakIterator.DONE) {
					start = 0;
				}

				int end = getNextWordStart(offset, s);
				if (end == BreakIterator.DONE) {
					end = s.length();
				}

				String str = get_text(start, end);
				return new StringSequence(str, start, end);
			}
			case AtkTextBoundary.WORD_END :
			{
				String s = get_text(0, get_character_count());
				int start = getPreviousWordEnd(offset, s);
				if (start == BreakIterator.DONE) {
					start = 0;
				}

				int end = getNextWordEnd(offset, s);
				if (end == BreakIterator.DONE) {
					end = s.length();
				}

				String str = get_text(start, end);
				return new StringSequence(str, start, end);
			}
			case AtkTextBoundary.SENTENCE_START :
			{
				String s = get_text(0, get_character_count());
				int start = getPreviousSentenceStart(offset, s);
				if (start == BreakIterator.DONE) {
					start = 0;
				}

				int end = getNextSentenceStart(offset, s);
				if (end == BreakIterator.DONE) {
					end = s.length();
				}

				String str = get_text(start, end);
				return new StringSequence(str, start, end);
			}
			case AtkTextBoundary.SENTENCE_END :
			{
				String s = get_text(0, get_character_count());
				int start = getPreviousSentenceEnd(offset, s);
				if (start == BreakIterator.DONE) {
					start = 0;
				}

				int end = getNextSentenceEnd(offset, s);
				if (end == BreakIterator.DONE) {
					end = s.length();
				}

				String str = get_text(start, end);
				return new StringSequence(str, start, end);
			}
			case AtkTextBoundary.LINE_START :
			case AtkTextBoundary.LINE_END :
			{
				BreakIterator lines = BreakIterator.getLineInstance();
				String s = get_text(0, get_character_count());
				lines.setText(s);

				int start = lines.preceding(offset);
				if (start == BreakIterator.DONE) {
					start = 0;
				}

				int end = lines.following(offset);
				if (end == BreakIterator.DONE) {
					end = s.length();
				}

				String str = get_text(start, end);
				return new StringSequence(str, start, end);
			}
			default :
			{
				return null;
			}
		}
	}
}

