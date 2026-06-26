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
import java.awt.*;
import java.lang.ref.WeakReference;
import java.text.BreakIterator;

/**
 * The ATK Text interface implementation for Java accessibility.
 * <p>
 * This class provides a bridge between Java's {@link AccessibleText}
 * interface and the ATK (Accessibility Toolkit) text interface used by
 * assistive technologies.
 * <p>
 * <strong>Offset conventions:</strong> ATK uses "character offsets" in the
 * exposed UTF-8 text stream. Across the JNI boundary we standardize on Unicode
 * code point offsets. Java Swing text component indices, however, are typically
 * UTF-16 indices. Therefore, all offsets received from native code are treated
 * as code point offsets and converted to UTF-16 indices before calling into
 * {@link AccessibleText}.
 */
public class AtkText {

    private final WeakReference<AccessibleContext> accessibleContextWeakRef;
    private final WeakReference<AccessibleText> accessibleTextWeakRef;
    private final WeakReference<AccessibleEditableText> accessibleEditableTextWeakRef;

    public AtkText(AccessibleContext ac) {
        super();
        this.accessibleContextWeakRef = new WeakReference<AccessibleContext>(ac);
        this.accessibleTextWeakRef = new WeakReference<AccessibleText>(ac.getAccessibleText());
        this.accessibleEditableTextWeakRef =
                new WeakReference<AccessibleEditableText>(ac.getAccessibleEditableText());
    }

    public static int getRightStart(int start) {
        if (start < 0)
            return 0;
        return start;
    }

    public static int getRightEnd(int start, int end, int count) {
        if (end < -1)
            return start;
        else if (end > count || end == -1)
            return count;
        else
            return end;
    }

    private int getPartTypeFromBoundary(int boundary_type) {
        switch (boundary_type) {
            case AtkTextBoundary.CHAR:
                return AccessibleText.CHARACTER;
            case AtkTextBoundary.WORD_START:
            case AtkTextBoundary.WORD_END:
                return AccessibleText.WORD;
            case AtkTextBoundary.SENTENCE_START:
            case AtkTextBoundary.SENTENCE_END:
                return AccessibleText.SENTENCE;
            case AtkTextBoundary.LINE_START:
            case AtkTextBoundary.LINE_END:
                return AccessibleExtendedText.LINE;
            default:
                return -1;
        }
    }

    private int getNextWordStart(int offset, String str) {
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

    private int getNextWordEnd(int offset, String str) {
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

    /**
     * Gets the start position of the previous word before the given UTF-16 index.
     *
     * @param utf16Index The UTF-16 character index within the text
     * @param text       The full text to search within
     * @return The UTF-16 start position of the previous word, or {@link BreakIterator#DONE}
     * if no previous word is found or if no word segments contain letters or digits
     */
    private int getPreviousWordStart(int utf16Index, String text) {
        BreakIterator words = BreakIterator.getWordInstance();
        words.setText(text);
        int start = words.preceding(utf16Index);
        int end = words.next();

        while (start != BreakIterator.DONE) {
            for (int i = start; i < end; i++) {
                if (Character.isLetter(text.codePointAt(i))) {
                    return start;
                }
            }

            end = start;
            start = words.preceding(end);
        }

        return BreakIterator.DONE;
    }

    private int getPreviousWordEnd(int offset, String str) {
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

    private int getNextSentenceStart(int offset, String str) {
        BreakIterator sentences = BreakIterator.getSentenceInstance();
        sentences.setText(str);
        int start = sentences.following(offset);

        return start;
    }

    private int getNextSentenceEnd(int offset, String str) {
        int start = getNextSentenceStart(offset, str);
        if (start == BreakIterator.DONE) {
            return str.length();
        }

        int index = start;
        do {
            index--;
        } while (index >= 0 && Character.isWhitespace(str.charAt(index)));

        index++;
        if (index < offset) {
            start = getNextSentenceStart(start, str);
            if (start == BreakIterator.DONE) {
                return str.length();
            }

            index = start;
            do {
                index--;
            } while (index >= 0 && Character.isWhitespace(str.charAt(index)));

            index++;
        }

        return index;
    }

    /**
     * Gets the start position of the previous sentence before the given offset.
     *
     * @param utf16Index The UTF-16 character index within the text
     * @param text       The full text to search within
     * @return The start position of the previous sentence, or BreakIterator.DONE if not found
     */
    private int getPreviousSentenceStart(int utf16Index, String text) {
        BreakIterator sentences = BreakIterator.getSentenceInstance();
        sentences.setText(text);
        int start = sentences.preceding(utf16Index);

        return start;
    }

    private int getPreviousSentenceEnd(int offset, String str) {
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

    private int getNextLineStart(int offset, String str) {
        int max = str.length();
        while (offset < max) {
            if (str.charAt(offset) == '\n')
                return offset + 1;
            offset += 1;
        }
        return offset;
    }

    private int getPreviousLineStart(int offset, String str) {
        offset -= 2;
        while (offset >= 0) {
            if (str.charAt(offset) == '\n')
                return offset + 1;
            offset -= 1;
        }
        return 0;
    }

    private int getNextLineEnd(int offset, String str) {
        int max = str.length();
        offset += 1;
        while (offset < max) {
            if (str.charAt(offset) == '\n')
                return offset;
            offset += 1;
        }
        return offset;
    }

    private int getPreviousLineEnd(int offset, String str) {
        offset -= 1;
        while (offset >= 0) {
            if (str.charAt(offset) == '\n')
                return offset;
            offset -= 1;
        }
        return 0;
    }

    private StringSequence private_get_text_at_offset(int offset,
                                                      int boundary_type) {
        int char_count = get_character_count();
        if (offset < 0 || offset > char_count) {
            return null;
        }

        switch (boundary_type) {
            case AtkTextBoundary.CHAR: {
                if (offset == char_count)
                    return null;
                String str = get_text(offset, offset + 1);
                return new StringSequence(str, offset, offset + 1);
            }
            case AtkTextBoundary.WORD_START: {
                if (offset == char_count)
                    return new StringSequence("", char_count, char_count);

                String s = get_text(0, char_count);
                int start = getPreviousWordStart(offset + 1, s);
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
            case AtkTextBoundary.WORD_END: {
                if (offset == 0)
                    return new StringSequence("", 0, 0);

                String s = get_text(0, char_count);
                int start = getPreviousWordEnd(offset, s);
                if (start == BreakIterator.DONE) {
                    start = 0;
                }

                int end = getNextWordEnd(offset - 1, s);
                if (end == BreakIterator.DONE) {
                    end = s.length();
                }

                String str = get_text(start, end);
                return new StringSequence(str, start, end);
            }
            case AtkTextBoundary.SENTENCE_START: {
                if (offset == char_count)
                    return new StringSequence("", char_count, char_count);

                String s = get_text(0, char_count);
                int start = getPreviousSentenceStart(offset + 1, s);
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
            case AtkTextBoundary.SENTENCE_END: {
                if (offset == 0)
                    return new StringSequence("", 0, 0);

                String s = get_text(0, char_count);
                int start = getPreviousSentenceEnd(offset, s);
                if (start == BreakIterator.DONE) {
                    start = 0;
                }

                int end = getNextSentenceEnd(offset - 1, s);
                if (end == BreakIterator.DONE) {
                    end = s.length();
                }

                String str = get_text(start, end);
                return new StringSequence(str, start, end);
            }
            case AtkTextBoundary.LINE_START: {
                if (offset == char_count)
                    return new StringSequence("", char_count, char_count);

                String s = get_text(0, char_count);
                int start = getPreviousLineStart(offset + 1, s);
                int end = getNextLineStart(offset, s);

                String str = get_text(start, end);
                return new StringSequence(str, start, end);
            }
            case AtkTextBoundary.LINE_END: {
                String s = get_text(0, char_count);
                int start = getPreviousLineEnd(offset, s);
                int end = getNextLineEnd(offset - 1, s);

                String str = get_text(start, end);
                return new StringSequence(str, start, end);
            }
            default: {
                return null;
            }
        }
    }

    /**
     * The ATK Text interface implementation for Java accessibility.
     * <p>
     * This class provides a bridge between Java's {@link AccessibleText}
     * interface and the ATK (Accessibility Toolkit) text interface used by
     * assistive technologies.
     * <p>
     * <strong>Offset conventions:</strong> ATK uses "character offsets" in the
     * exposed UTF-8 text stream. Across the JNI boundary we standardize on Unicode
     * code point offsets. Java Swing text component indices, however, are typically
     * UTF-16 indices. Therefore, all offsets received from native code are treated
     * as code point offsets and converted to UTF-16 indices before calling into
     * {@link AccessibleText}.
     */
    public class StringSequence {

        public final String str;
        public final int start_offset, end_offset;

        public StringSequence(String str, int start_offset, int end_offset) {
            this.str = str;
            this.start_offset = start_offset;
            this.end_offset = end_offset;
        }
    }

    // JNI upcalls section

    /**
     * Factory method to create an AtkText instance from an AccessibleContext.
     * Called from native code via JNI.
     *
     * @param ac the AccessibleContext to wrap
     * @return a new AtkText instance, or null if creation fails
     */
    private static AtkText createAtkText(AccessibleContext ac) {
        return AtkUtil.invokeInSwing(() -> {
            return new AtkText(ac);
        }, null);
    }

    /* Return string from start, up to, but not including end */

    /**
     * Gets the specified text from start to end offset.
     * Called from native code via JNI.
     *
     * @param startCodePointIndex a starting code point offset within the text
     * @param endCodePointIndex   an ending code point offset within the text, or -1 for the end of the string
     * @return a string containing the text from start up to, but not including end, or null if retrieval fails
     */
    private String get_text(int startCodePointIndex, int endCodePointIndex) {
        AccessibleText accessibleText = accessibleTextWeakRef.get();
        if (accessibleText == null)
            return null;

        return AtkUtil.invokeInSwing(() -> {
            final int rightStart = getRightStart(startCodePointIndex);
            final int rightEnd =
                    getRightEnd(
                            startCodePointIndex,
                            endCodePointIndex,
                            accessibleText.getCharCount());

            if (accessibleText instanceof AccessibleExtendedText acc_ext_text) {
                return acc_ext_text.getTextRange(rightStart, rightEnd);
            }
            StringBuffer buf = new StringBuffer();
            for (int i = rightStart; i <= rightEnd - 1; i++) {
                String str = accessibleText.getAtIndex(AccessibleText.CHARACTER, i);
                buf.append(str);
            }
            return buf.toString();
        }, null);
    }

    /**
     * Gets the character (Unicode code point) at the specified offset.
     * Called from native code via JNI.
     *
     * @param codePointOffset a code point offset within the text
     * @return the Unicode code point at the specified offset, or 0 in case of failure
     */
    private char get_character_at_offset(int codePointOffset) {
        AccessibleText accessibleText = accessibleTextWeakRef.get();
        if (accessibleText == null)
            return ' ';

        return AtkUtil.invokeInSwing(() -> {
            String str = accessibleText.getAtIndex(AccessibleText.CHARACTER, codePointOffset);
            if (str == null || str.length() == 0)
                return ' ';
            return str.charAt(0);
        }, ' ');
    }

    private StringSequence get_text_at_offset(int codePointOffset, int boundaryType) {
        AccessibleText accessibleText = accessibleTextWeakRef.get();
        if (accessibleText == null)
            return null;

        return AtkUtil.invokeInSwing(() -> {
            if (false) {
                // FIXME: this is not using start/end boundaries
                AccessibleExtendedText acc_ext_text = (AccessibleExtendedText)accessibleText;
                int part = getPartTypeFromBoundary(boundaryType);
                if (part == -1)
                    return null;
                AccessibleTextSequence seq = acc_ext_text.getTextSequenceAt(part, codePointOffset);
                if (seq == null)
                    return null;
                return new StringSequence(seq.text, seq.startIndex, seq.endIndex + 1);
            } else {
                return private_get_text_at_offset(codePointOffset, boundaryType);
            }
        }, null);
    }

    private StringSequence get_text_before_offset(int offset, int boundary_type) {
        AccessibleText accessibleText = accessibleTextWeakRef.get();
        if (accessibleText == null)
            return null;

        return AtkUtil.invokeInSwing(() -> {
            if (false) {
                // FIXME: this is not using start/end boundaries
                AccessibleExtendedText acc_ext_text = (AccessibleExtendedText)accessibleText;
                int part = getPartTypeFromBoundary(boundary_type);
                if (part == -1)
                    return null;
                AccessibleTextSequence seq = acc_ext_text.getTextSequenceBefore(part, offset);
                if (seq == null)
                    return null;
                return new StringSequence(seq.text, seq.startIndex, seq.endIndex + 1);
            } else {
                StringSequence seq = private_get_text_at_offset(offset, boundary_type);
                if (seq == null)
                    return null;
                return private_get_text_at_offset(seq.start_offset - 1, boundary_type);
            }
        }, null);
    }

    private StringSequence get_text_after_offset(int offset, int boundary_type) {
        AccessibleText accessibleText = accessibleTextWeakRef.get();
        if (accessibleText == null)
            return null;

        return AtkUtil.invokeInSwing(() -> {
            if (false) {
                // FIXME: this is not using start/end boundaries
                AccessibleExtendedText acc_ext_text = (AccessibleExtendedText)accessibleText;
                int part = getPartTypeFromBoundary(boundary_type);
                if (part == -1)
                    return null;
                AccessibleTextSequence seq = acc_ext_text.getTextSequenceAfter(part, offset);
                if (seq == null)
                    return null;
                return new StringSequence(seq.text, seq.startIndex, seq.endIndex + 1);
            } else {
                StringSequence seq = private_get_text_at_offset(offset, boundary_type);
                if (seq == null)
                    return null;
                return private_get_text_at_offset(seq.end_offset, boundary_type);
            }
        }, null);
    }

    /**
     * Gets the offset of the position of the caret (cursor).
     * Called from native code via JNI.
     *
     * @return the character offset of the position of the caret, or -1 if the caret is not located
     * inside the element or in the case of any other failure
     */
    private int get_caret_offset() {
        AccessibleText accessibleText = accessibleTextWeakRef.get();
        if (accessibleText == null)
            return 0;

        return AtkUtil.invokeInSwing(() -> {
            return accessibleText.getCaretPosition();
        }, 0);
    }

    /**
     * Gets the bounding box containing the glyph representing the character at a particular text offset.
     * Called from native code via JNI.
     *
     * @param codePointOffset the code point offset of the text character for which bounding information is required
     * @param coordType       specifies whether coordinates are relative to the screen or widget window
     * @return a Rectangle containing the bounding box (x, y, width, height), or null if the extent
     * cannot be obtained. Returns null if all coordinates are set to -1.
     */
    private Rectangle get_character_extents(int codePointOffset, int coordType) {
        AccessibleContext accessibleContext = accessibleContextWeakRef.get();
        if (accessibleContext == null)
            return null;
        AccessibleText accessibleText = accessibleTextWeakRef.get();
        if (accessibleText == null)
            return null;

        return AtkUtil.invokeInSwing(() -> {
            Rectangle rect = accessibleText.getCharacterBounds(codePointOffset);
            if (rect == null)
                return null;
            AccessibleComponent component = accessibleContext.getAccessibleComponent();
            if (component == null)
                return null;
            Point p = AtkComponent.getComponentOrigin(accessibleContext, component, coordType);
            rect.x += p.x;
            rect.y += p.y;
            return rect;
        }, null);
    }

    /**
     * Gets the character count.
     * Called from native code via JNI.
     *
     * @return the number of characters, or -1 in case of failure
     */
    private int get_character_count() {
        AccessibleText accessibleText = accessibleTextWeakRef.get();
        if (accessibleText == null)
            return 0;

        return AtkUtil.invokeInSwing(() -> {
            return accessibleText.getCharCount();
        }, 0);
    }

    /**
     * Gets the offset of the character located at coordinates @x and @y. @x and @y
     * are interpreted as being relative to the screen or this widget's window
     * depending on @coords.
     *
     * @param x         int screen x-position of character
     * @param y         int screen y-position of character
     * @param coordType int specify whether coordinates are relative to the screen or
     *                  widget window
     * @return the offset to the character which is located at the specified
     * @x and @y coordinates or -1 in case of failure.
     */
    private int get_offset_at_point(int x, int y, int coordType) {
        AccessibleContext accessibleContext = accessibleContextWeakRef.get();
        if (accessibleContext == null)
            return -1;
        AccessibleText accessibleText = accessibleTextWeakRef.get();
        if (accessibleText == null)
            return -1;

        return AtkUtil.invokeInSwing(() -> {
            AccessibleComponent component = accessibleContext.getAccessibleComponent();
            if (component == null)
                return -1;
            Point p = AtkComponent.getComponentOrigin(accessibleContext, component, coordType);
            return accessibleText.getIndexAtPoint(new Point(x - p.x, y - p.y));
        }, -1);
    }

    /**
     * Gets the bounding box for text within the specified range.
     * Called from native code via JNI.
     *
     * @param startCodePointIndex the code point offset of the first text character for which boundary information is required
     * @param endCodePointIndex   the code point offset of the text character after the last character for which boundary information is required
     * @param coordType           specifies whether coordinates are relative to the screen or widget window
     * @return a Rectangle filled in with the bounding box, or null if the extents cannot be obtained.
     * Returns null if all rectangle fields are set to -1.
     */
    private Rectangle get_range_extents(
            int startCodePointIndex, int endCodePointIndex, int coordType) {
        AccessibleContext accessibleContext = accessibleContextWeakRef.get();
        if (accessibleContext == null)
            return null;
        AccessibleText accessibleText = accessibleTextWeakRef.get();
        if (accessibleText == null)
            return null;

        return AtkUtil.invokeInSwing(() -> {
            if (accessibleText instanceof AccessibleExtendedText acc_ext_text) {
                final int rightStart = getRightStart(startCodePointIndex);
                final int rightEnd =
                        getRightEnd(
                                startCodePointIndex,
                                endCodePointIndex,
                                accessibleText.getCharCount());

                Rectangle rect = acc_ext_text.getTextBounds(rightStart, rightEnd);
                if (rect == null)
                    return null;
                AccessibleComponent component = accessibleContext.getAccessibleComponent();
                if (component == null)
                    return null;
                Point p = AtkComponent.getComponentOrigin(accessibleContext, component, coordType);
                rect.x += p.x;
                rect.y += p.y;
                return rect;
            }
            return null;
        }, null);
    }

    /**
     * Gets the number of selected regions.
     * Called from native code via JNI.
     *
     * @return The number of selected regions, or -1 in the case of failure.
     */
    private int get_n_selections() {
        AccessibleText accessibleText = accessibleTextWeakRef.get();
        if (accessibleText == null)
            return 0;

        return AtkUtil.invokeInSwing(() -> {
            String str = accessibleText.getSelectedText();
            if (str != null && str.length() > 0)
                return 1;
            return 0;
        }, 0);
    }

    /**
     * Gets the text from the specified selection.
     * Called from native code via JNI.
     *
     * @return a StringSequence containing the selected text and its start and end code point offsets,
     * or null if there is no selection or retrieval fails
     */
    private StringSequence get_selection() {
        AccessibleText accessibleText = accessibleTextWeakRef.get();
        if (accessibleText == null)
            return null;

        return AtkUtil.invokeInSwing(() -> {
            int start = accessibleText.getSelectionStart();
            int end = accessibleText.getSelectionEnd();
            String text = accessibleText.getSelectedText();
            if (text == null)
                return null;
            return new StringSequence(text, start, end);
        }, null);
    }

    /**
     * Adds a selection bounded by the specified offsets.
     * Called from native code via JNI.
     *
     * @param startCodePointIndex the starting code point offset of the selected region
     * @param endCodePointIndex   the code point offset of the first character after the selected region
     * @return true if successful, false otherwise. Note that Java AccessibleText only supports
     * a single selection, so this will return false if a selection already exists.
     */
    private boolean add_selection(int startCodePointIndex, int endCodePointIndex) {
        AccessibleText accessibleText = accessibleTextWeakRef.get();
        if (accessibleText == null)
            return false;
        AccessibleEditableText accessibleEditableText = accessibleEditableTextWeakRef.get();
        if (accessibleEditableText == null)
            return false;

        return AtkUtil.invokeInSwing(() -> {
            if (accessibleEditableText == null || get_n_selections() > 0)
                return false;

            final int rightStart = getRightStart(startCodePointIndex);
            final int rightEnd =
                    getRightEnd(
                            startCodePointIndex,
                            endCodePointIndex,
                            accessibleText.getCharCount());

            return set_selection(0, rightStart, rightEnd);
        }, false);
    }

    /**
     * Removes the specified selection.
     * Called from native code via JNI.
     *
     * @param selectionNum the selection number. The selected regions are assigned numbers
     *                     that correspond to how far the region is from the start of the text.
     *                     Since Java only supports a single selection, only 0 is valid.
     * @return true if successful, false otherwise
     */
    private boolean remove_selection(int selectionNum) {
        AccessibleEditableText accessibleEditableText = accessibleEditableTextWeakRef.get();
        if (accessibleEditableText == null)
            return false;

        return AtkUtil.invokeInSwing(() -> {
            if (accessibleEditableText == null || selectionNum > 0)
                return false;
            accessibleEditableText.selectText(0, 0);
            return true;
        }, false);
    }

    /**
     * Changes the start and end offset of the specified selection.
     * Called from native code via JNI.
     *
     * @param selectionNum        the selection number. The selected regions are assigned numbers
     *                            that correspond to how far the region is from the start of the text.
     *                            Since Java only supports a single selection, only 0 is valid.
     * @param startCodePointIndex the new starting code point offset of the selection
     * @param endCodePointIndex   the new end code point offset (offset immediately past) of the selection
     * @return true if successful, false otherwise
     */
    private boolean set_selection(
            int selectionNum, int startCodePointIndex, int endCodePointIndex) {
        AccessibleText accessibleText = accessibleTextWeakRef.get();
        if (accessibleText == null)
            return false;
        AccessibleEditableText accessibleEditableText = accessibleEditableTextWeakRef.get();
        if (accessibleEditableText == null)
            return false;

        return AtkUtil.invokeInSwing(() -> {
            if (accessibleEditableText == null || selectionNum > 0)
                return false;

            final int rightStart = getRightStart(startCodePointIndex);
            final int rightEnd =
                    getRightEnd(
                            startCodePointIndex,
                            endCodePointIndex,
                            accessibleText.getCharCount());

            accessibleEditableText.selectText(rightStart, rightEnd);
            return true;
        }, false);
    }

    /**
     * Sets the caret (cursor) position to the specified offset.
     * Called from native code via JNI.
     *
     * @param codePointOffset the code point offset of the new caret position
     * @return true if successful, false otherwise
     */
    private boolean set_caret_offset(int codePointOffset) {
        AccessibleText accessibleText = accessibleTextWeakRef.get();
        if (accessibleText == null)
            return false;
        AccessibleEditableText accessibleEditableText = accessibleEditableTextWeakRef.get();
        if (accessibleEditableText == null)
            return false;

        return AtkUtil.invokeInSwing(() -> {
            if (accessibleEditableText != null) {
                final int rightOffset =
                        getRightEnd(0, codePointOffset, accessibleText.getCharCount());
                accessibleEditableText.selectText(codePointOffset, codePointOffset);
                return true;
            }
            return false;
        }, false);
    }


}
