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

#include "jawimpl.h"
#include "jawutil.h"
#include <atk/atk.h>
#include <glib.h>

/**
 * (From Atk documentation)
 *
 * AtkText:
 *
 * The ATK interface implemented by components with text content.
 *
 * #AtkText should be implemented by #AtkObjects on behalf of widgets
 * that have text content which is either attributed or otherwise
 * non-trivial.  #AtkObjects whose text content is simple,
 * unattributed, and very brief may expose that content via
 * #atk_object_get_name instead; however if the text is editable,
 * multi-line, typically longer than three or four words, attributed,
 * selectable, or if the object already uses the 'name' ATK property
 * for other information, the #AtkText interface should be used to
 * expose the text content.  In the case of editable text content,
 * #AtkEditableText (a subtype of the #AtkText interface) should be
 * implemented instead.
 *
 *  #AtkText provides not only traversal facilities and change
 * notification for text content, but also caret tracking and glyph
 * bounding box calculations.  Note that the text strings are exposed
 * as UTF-8, and are therefore potentially multi-byte, and
 * caret-to-byte offset mapping makes no assumptions about the
 * character length; also bounding box glyph-to-offset mapping may be
 * complex for languages which use ligatures.
 */

static gchar *jaw_text_get_text (AtkText *text,
                                 gint start_offset,
                                 gint end_offset);

static gunichar jaw_text_get_character_at_offset (AtkText *text, gint offset);

static gchar *jaw_text_get_text_at_offset (AtkText *text,
                                           gint offset,
                                           AtkTextBoundary boundary_type,
                                           gint *start_offset,
                                           gint *end_offset);

static gchar *jaw_text_get_text_before_offset (AtkText *text,
                                               gint offset,
                                               AtkTextBoundary boundary_type,
                                               gint *start_offset,
                                               gint *end_offset);

static gchar *jaw_text_get_text_after_offset (AtkText *text,
                                              gint offset,
                                              AtkTextBoundary boundary_type,
                                              gint *start_offset,
                                              gint *end_offset);

static gint jaw_text_get_caret_offset (AtkText *text);

static void jaw_text_get_character_extents (AtkText *text,
                                            gint offset,
                                            gint *x,
                                            gint *y,
                                            gint *width,
                                            gint *height,
                                            AtkCoordType coords);

static gint jaw_text_get_character_count (AtkText *text);

static gint jaw_text_get_offset_at_point (AtkText *text,
                                          gint x,
                                          gint y,
                                          AtkCoordType coords);
static void jaw_text_get_range_extents (AtkText *text,
                                        gint start_offset,
                                        gint end_offset,
                                        AtkCoordType coord_type,
                                        AtkTextRectangle *rect);

static gint jaw_text_get_n_selections (AtkText *text);

static gchar *jaw_text_get_selection (AtkText *text,
                                      gint selection_num,
                                      gint *start_offset,
                                      gint *end_offset);

static gboolean jaw_text_add_selection (AtkText *text,
                                        gint start_offset,
                                        gint end_offset);

static gboolean jaw_text_remove_selection (AtkText *text, gint selection_num);

static gboolean jaw_text_set_selection (AtkText *text,
                                        gint selection_num,
                                        gint start_offset,
                                        gint end_offset);

static gboolean jaw_text_set_caret_offset (AtkText *text, gint offset);

typedef struct _TextData
{
  jobject atk_text;
  gchar *text;
  jstring jstrText;
} TextData;

#define JAW_GET_TEXT(text, def_ret) \
  JAW_GET_OBJ_IFACE (text, INTERFACE_TEXT, TextData, atk_text, jniEnv, atk_text, def_ret)

void
jaw_text_interface_init (AtkTextIface *iface, gpointer data)
{
  JAW_DEBUG_ALL ("%p, %p", iface, data);
  iface->get_text = jaw_text_get_text;
  iface->get_text_after_offset = jaw_text_get_text_after_offset;
  iface->get_text_at_offset = jaw_text_get_text_at_offset;
  iface->get_character_at_offset = jaw_text_get_character_at_offset;
  iface->get_text_before_offset = jaw_text_get_text_before_offset;
  iface->get_caret_offset = jaw_text_get_caret_offset;
  // TODO: iface->get_run_attributes by iterating getCharacterAttribute or using getTextSequenceAt with ATTRIBUTE_RUN
  // TODO: iface->get_default_attributes
  iface->get_character_extents = jaw_text_get_character_extents;
  iface->get_character_count = jaw_text_get_character_count;
  iface->get_offset_at_point = jaw_text_get_offset_at_point;
  iface->get_n_selections = jaw_text_get_n_selections;
  iface->get_selection = jaw_text_get_selection;
  iface->add_selection = jaw_text_add_selection;
  iface->remove_selection = jaw_text_remove_selection;
  iface->set_selection = jaw_text_set_selection;
  iface->set_caret_offset = jaw_text_set_caret_offset;

  iface->get_range_extents = jaw_text_get_range_extents;
  // TODO: iface->get_bounded_ranges from getTextBounds
  // TODO: iface->get_string_at_offset

  // TODO: missing java support for:
  // iface->scroll_substring_to
  // iface->scroll_substring_to_point
}

/**
 * jaw_text_data_init:
 * @ac: a Java AccessibleContext object
 *
 * Initializes the text interface data for an accessible object.
 * Creates and returns a TextData structure containing a global reference
 * to the Java AtkText object.
 *
 * Explicitly manages a JNI local reference frame using
 * PushLocalFrame/PopLocalFrame; all local references are released
 * before the function returns.
 *
 * Returns: (nullable): pointer to TextData or NULL on failure
 **/

gpointer
jaw_text_data_init (jobject ac)
{
  JAW_DEBUG_ALL ("%p", ac);
  TextData *data = g_new0 (TextData, 1);

  JNIEnv *jniEnv = jaw_util_get_jni_env ();
  jclass classText = (*jniEnv)->FindClass (jniEnv,
                                           "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetStaticMethodID (jniEnv,
                                                 classText,
                                                 "createAtkText",
                                                 "(Ljavax/accessibility/AccessibleContext;)Lorg/GNOME/Accessibility/AtkText;");
  jobject jatk_text = (*jniEnv)->CallStaticObjectMethod (jniEnv, classText, jmid, ac);
  data->atk_text = (*jniEnv)->NewGlobalRef (jniEnv, jatk_text);

  return data;
}

/**
 * jaw_text_data_finalize:
 * @p: TextData pointer to finalize
 *
 * Cleans up TextData when the parent GObject is finalized.
 * Called from jaw_impl_finalize() when the object's reference count reaches
 * zero.
 */

void
jaw_text_data_finalize (gpointer p)
{
  JAW_DEBUG_ALL ("%p", p);
  TextData *data = (TextData *) p;
  JNIEnv *jniEnv = jaw_util_get_jni_env ();

  if (data && data->atk_text)
    {
      if (data->text != NULL)
        {
          (*jniEnv)->ReleaseStringUTFChars (jniEnv, data->jstrText, data->text);
          (*jniEnv)->DeleteGlobalRef (jniEnv, data->jstrText);
          data->jstrText = NULL;
          data->text = NULL;
        }

      (*jniEnv)->DeleteGlobalRef (jniEnv, data->atk_text);
      data->atk_text = NULL;
    }
}

static gchar *
jaw_text_get_gtext_from_jstr (JNIEnv *jniEnv, jstring jstr)
{
  JAW_DEBUG_C ("%p, %p", jniEnv, jstr);
  if (jstr == NULL)
    {
      return NULL;
    }

  gchar *tmp_text = (gchar *) (*jniEnv)->GetStringUTFChars (jniEnv, jstr, NULL);
  gchar *text = g_strdup (tmp_text);
  (*jniEnv)->ReleaseStringUTFChars (jniEnv, jstr, tmp_text);

  return text;
}

/**
 * atk_text_get_text:
 * @text: an #AtkText
 * @start_offset: a starting character offset within @text
 * @end_offset: an ending character offset within @text, or -1 for the end of
 *the string.
 *
 * Gets the specified text.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * Returns: a newly allocated string containing the text from @start_offset up
 *          to, but not including @end_offset. Use g_free() to free the returned
 *          string.
 **/

static gchar *
jaw_text_get_text (AtkText *text, gint start_offset, gint end_offset)
{
  JAW_DEBUG_C ("%p, %d, %d", text, start_offset, end_offset);
  JAW_GET_TEXT (text, NULL);

  jclass classAtkText = (*jniEnv)->FindClass (jniEnv,
                                              "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkText,
                                           "get_text",
                                           "(II)Ljava/lang/String;");

  jstring jstr = (*jniEnv)->CallObjectMethod (jniEnv,
                                              atk_text,
                                              jmid,
                                              (jint) start_offset,
                                              (jint) end_offset);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_text);

  return jaw_text_get_gtext_from_jstr (jniEnv, jstr);
}

/**
 * jaw_text_get_character_at_offset:
 * @text: an #AtkText
 * @offset: a character offset within @text
 *
 * Gets the specified text.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * Returns: the character at @offset or 0 in the case of failure.
 **/

static gunichar
jaw_text_get_character_at_offset (AtkText *text, gint offset)
{
  JAW_DEBUG_C ("%p, %d", text, offset);
  JAW_GET_TEXT (text, 0);

  jclass classAtkText = (*jniEnv)->FindClass (jniEnv,
                                              "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkText,
                                           "get_character_at_offset", "(I)I");
  jint jcharacter = (*jniEnv)->CallIntMethod (jniEnv,
                                              atk_text,
                                              jmid,
                                              (jint) offset);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_text);

  return (gunichar) jcharacter;
}

static gchar *
jaw_text_get_gtext_from_string_seq (JNIEnv *jniEnv,
                                    jobject jStrSeq,
                                    gint *start_offset,
                                    gint *end_offset)
{
  jclass classStringSeq = (*jniEnv)->FindClass (jniEnv,
                                                "org/GNOME/Accessibility/AtkText$StringSequence");
  jfieldID jfidStr = (*jniEnv)->GetFieldID (jniEnv,
                                            classStringSeq,
                                            "str",
                                            "Ljava/lang/String;");
  jfieldID jfidStart = (*jniEnv)->GetFieldID (jniEnv,
                                              classStringSeq,
                                              "start_offset",
                                              "I");
  jfieldID jfidEnd = (*jniEnv)->GetFieldID (jniEnv,
                                            classStringSeq,
                                            "end_offset",
                                            "I");

  jstring jStr = (*jniEnv)->GetObjectField (jniEnv, jStrSeq, jfidStr);
  jint jStart = (*jniEnv)->GetIntField (jniEnv, jStrSeq, jfidStart);
  jint jEnd = (*jniEnv)->GetIntField (jniEnv, jStrSeq, jfidEnd);

  (*start_offset) = (gint) jStart;
  (*end_offset) = (gint) jEnd;

  return jaw_text_get_gtext_from_jstr (jniEnv, jStr);
}

/**
 * jaw_text_get_text_at_offset:
 * @text: an #AtkText
 * @offset: position
 * @boundary_type: An #AtkTextBoundary
 * @start_offset: (out): the starting character offset of the returned string
 * @end_offset: (out): the offset of the first character after the
 *              returned substring
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * Deprecated: This method is deprecated since ATK version
 * 2.9.4. Please use atk_text_get_string_at_offset() instead.
 *
 * Returns: a newly allocated string containing the text at @offset bounded
 *          by the specified @boundary_type. Use g_free() to free the returned
 *          string.
 **/

static gchar *
jaw_text_get_text_at_offset (AtkText *text,
                             gint offset,
                             AtkTextBoundary boundary_type,
                             gint *start_offset,
                             gint *end_offset)
{
  JAW_DEBUG_C ("%p, %d, %d, %p, %p", text, offset, boundary_type, start_offset, end_offset);
  JAW_GET_TEXT (text, NULL);

  jclass classAtkText = (*jniEnv)->FindClass (jniEnv,
                                              "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkText,
                                           "get_text_at_offset",
                                           "(II)Lorg/GNOME/Accessibility/AtkText$StringSequence;");
  jobject jStrSeq = (*jniEnv)->CallObjectMethod (jniEnv,
                                                 atk_text,
                                                 jmid,
                                                 (jint) offset,
                                                 (jint) boundary_type);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_text);

  if (jStrSeq == NULL)
    {
      return NULL;
    }

  return jaw_text_get_gtext_from_string_seq (jniEnv, jStrSeq, start_offset, end_offset);
}

static gchar *
jaw_text_get_text_before_offset (AtkText *text,
                                 gint offset,
                                 AtkTextBoundary boundary_type,
                                 gint *start_offset,
                                 gint *end_offset)
{
  JAW_DEBUG_C ("%p, %d, %d, %p, %p", text, offset, boundary_type, start_offset, end_offset);
  JAW_GET_TEXT (text, NULL);

  jclass classAtkText = (*jniEnv)->FindClass (jniEnv,
                                              "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkText,
                                           "get_text_before_offset",
                                           "(II)Lorg/GNOME/Accessibility/AtkText$StringSequence;");
  jobject jStrSeq = (*jniEnv)->CallObjectMethod (jniEnv,
                                                 atk_text,
                                                 jmid,
                                                 (jint) offset,
                                                 (jint) boundary_type);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_text);

  if (jStrSeq == NULL)
    {
      return NULL;
    }

  return jaw_text_get_gtext_from_string_seq (jniEnv, jStrSeq, start_offset, end_offset);
}

static gchar *
jaw_text_get_text_after_offset (AtkText *text,
                                gint offset,
                                AtkTextBoundary boundary_type,
                                gint *start_offset,
                                gint *end_offset)
{
  JAW_DEBUG_C ("%p, %d, %d, %p, %p", text, offset, boundary_type, start_offset, end_offset);
  JAW_GET_TEXT (text, NULL);

  jclass classAtkText = (*jniEnv)->FindClass (jniEnv,
                                              "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkText,
                                           "get_text_after_offset",
                                           "(II)Lorg/GNOME/Accessibility/AtkText$StringSequence;");
  jobject jStrSeq = (*jniEnv)->CallObjectMethod (jniEnv,
                                                 atk_text,
                                                 jmid,
                                                 (jint) offset,
                                                 (jint) boundary_type);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_text);

  if (jStrSeq == NULL)
    {
      return NULL;
    }

  return jaw_text_get_gtext_from_string_seq (jniEnv, jStrSeq, start_offset, end_offset);
}

/**
 * jaw_text_get_caret_offset:
 * @text: an #AtkText
 *
 * Gets the offset of the position of the caret (cursor).
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * Returns: the character offset of the position of the caret or -1 if
 *          the caret is not located inside the element or in the case of
 *          any other failure.
 **/

static gint
jaw_text_get_caret_offset (AtkText *text)
{
  JAW_DEBUG_C ("%p", text);
  JAW_GET_TEXT (text, -1);

  jclass classAtkText = (*jniEnv)->FindClass (jniEnv,
                                              "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkText,
                                           "get_caret_offset",
                                           "()I");
  jint joffset = (*jniEnv)->CallIntMethod (jniEnv, atk_text, jmid);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_text);

  return (gint) joffset;
}

/**
 * jaw_text_get_character_extents:
 * @text: an #AtkText
 * @offset: The offset of the text character for which bounding information is
 *required.
 * @x: (out) (optional): Pointer for the x coordinate of the bounding box
 * @y: (out) (optional): Pointer for the y coordinate of the bounding box
 * @width: (out) (optional): Pointer for the width of the bounding box
 * @height: (out) (optional): Pointer for the height of the bounding box
 * @coords: specify whether coordinates are relative to the screen or widget
 *window
 *
 * If the extent can not be obtained (e.g. missing support), all of x, y, width,
 * height are set to -1.
 *
 * Get the bounding box containing the glyph representing the character at
 *     a particular text offset.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 **/

static void
jaw_text_get_character_extents (AtkText *text,
                                gint offset,
                                gint *x,
                                gint *y,
                                gint *width,
                                gint *height,
                                AtkCoordType coords)
{
  JAW_DEBUG_C ("%p, %d, %p, %p, %p, %p, %d", text, offset, x, y, width, height, coords);
  *x = -1;
  *y = -1;
  *width = -1;
  *height = -1;
  JAW_GET_TEXT (text, );

  jclass classAtkText = (*jniEnv)->FindClass (jniEnv,
                                              "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkText,
                                           "get_character_extents",
                                           "(II)Ljava/awt/Rectangle;");
  jobject jrect = (*jniEnv)->CallObjectMethod (jniEnv,
                                               atk_text,
                                               jmid,
                                               (jint) offset,
                                               (jint) coords);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_text);

  if (jrect == NULL)
    {
      JAW_DEBUG_I ("jrect == NULL");
      return;
    }

  jaw_util_get_rect_info (jniEnv, jrect, x, y, width, height);
}

/**
 * jaw_text_get_character_count:
 * @text: an #AtkText
 *
 * Gets the character count.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * Returns: the number of characters or -1 in case of failure.
 **/

static gint
jaw_text_get_character_count (AtkText *text)
{
  JAW_DEBUG_C ("%p", text);
  JAW_GET_TEXT (text, -1);

  jclass classAtkText = (*jniEnv)->FindClass (jniEnv,
                                              "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkText,
                                           "get_character_count",
                                           "()I");
  jint jcount = (*jniEnv)->CallIntMethod (jniEnv, atk_text, jmid);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_text);

  return (gint) jcount;
}

/**
 * jaw_text_get_offset_at_point:
 * @text: an #AtkText
 * @x: screen x-position of character
 * @y: screen y-position of character
 * @coords: specify whether coordinates are relative to the screen or
 * widget window
 *
 * Gets the offset of the character located at coordinates @x and @y. @x and @y
 * are interpreted as being relative to the screen or this widget's window
 * depending on @coords.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * Returns: the offset to the character which is located at  the specified
 *          @x and @y coordinates of -1 in case of failure.
 **/

static gint
jaw_text_get_offset_at_point (AtkText *text, gint x, gint y, AtkCoordType coords)
{
  JAW_DEBUG_C ("%p, %d, %d, %d", text, x, y, coords);
  JAW_GET_TEXT (text, 0);

  jclass classAtkText = (*jniEnv)->FindClass (jniEnv, "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkText,
                                           "get_offset_at_point",
                                           "(III)I");
  jint joffset = (*jniEnv)->CallIntMethod (jniEnv,
                                           atk_text,
                                           jmid,
                                           (jint) x,
                                           (jint) y,
                                           (jint) coords);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_text);

  return (gint) joffset;
}

/**
 * jaw_text_get_range_extents:
 * @text: an #AtkText
 * @start_offset: The offset of the first text character for which boundary
 *        information is required.
 * @end_offset: The offset of the text character after the last character
 *        for which boundary information is required.
 * @coord_type: Specify whether coordinates are relative to the screen or widget
 *window.
 * @rect: (out): A pointer to a AtkTextRectangle which is filled in by this
 *function.
 *
 * Get the bounding box for text within the specified range.
 *
 * If the extents can not be obtained (e.g. or missing support), the rectangle
 * fields are set to -1.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * In Atk Since: 1.3
 **/

static void
jaw_text_get_range_extents (AtkText *text,
                            gint start_offset,
                            gint end_offset,
                            AtkCoordType coord_type,
                            AtkTextRectangle *rect)
{
  JAW_DEBUG_C ("%p, %d, %d, %d, %p", text, start_offset, end_offset, coord_type, rect);
  if (rect == NULL)
    {
      return;
    }
  rect->x = -1;
  rect->y = -1;
  rect->width = -1;
  rect->height = -1;

  JAW_GET_TEXT (text, );

  jclass classAtkText = (*jniEnv)->FindClass (jniEnv,
                                              "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkText,
                                           "get_range_extents",
                                           "(III)Ljava/awt/Rectangle;");
  jobject jrect = (*jniEnv)->CallObjectMethod (jniEnv,
                                               atk_text,
                                               jmid,
                                               (jint) start_offset,
                                               (jint) end_offset,
                                               (jint) coord_type);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_text);

  if (!jrect)
    {
      return;
    }

  jaw_util_get_rect_info (jniEnv, jrect, &(rect->x), &(rect->y), &(rect->width), &(rect->height));
}

/**
 * jaw_text_get_n_selections:
 * @text: an #AtkText
 *
 * Gets the number of selected regions.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * Returns: The number of selected regions, or -1 in the case of failure.
 **/

static gint
jaw_text_get_n_selections (AtkText *text)
{
  JAW_DEBUG_C ("%p", text);
  JAW_GET_TEXT (text, -1);

  jclass classAtkText = (*jniEnv)->FindClass (jniEnv,
                                              "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkText,
                                           "get_n_selections",
                                           "()I");
  jint jselections = (*jniEnv)->CallIntMethod (jniEnv, atk_text, jmid);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_text);

  return (gint) jselections;
}

/**
 * jaw_text_get_selection:
 * @text: an #AtkText
 * @selection_num: The selection number.  The selected regions are
 * assigned numbers that correspond to how far the region is from the
 * start of the text.  The selected region closest to the beginning
 * of the text region is assigned the number 0, etc.  Note that adding,
 * moving or deleting a selected region can change the numbering.
 * @start_offset: (out): passes back the starting character offset of the
 *selected region
 * @end_offset: (out): passes back the ending character offset (offset
 *immediately past) of the selected region
 *
 * Gets the text from the specified selection.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * Returns: a newly allocated string containing the selected text. Use g_free()
 *          to free the returned string.
 **/

static gchar *
jaw_text_get_selection (AtkText *text, gint selection_num, gint *start_offset, gint *end_offset)
{
  JAW_DEBUG_C ("%p, %d, %p, %p", text, selection_num, start_offset, end_offset);
  JAW_GET_TEXT (text, NULL);

  jclass classAtkText = (*jniEnv)->FindClass (jniEnv, "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkText,
                                           "get_selection",
                                           "()Lorg/GNOME/Accessibility/AtkText$StringSequence;");
  jobject jStrSeq = (*jniEnv)->CallObjectMethod (jniEnv, atk_text, jmid);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_text);

  if (jStrSeq == NULL)
    {
      return NULL;
    }

  jclass classStringSeq = (*jniEnv)->FindClass (jniEnv,
                                                "org/GNOME/Accessibility/AtkText$StringSequence");
  jfieldID jfidStr = (*jniEnv)->GetFieldID (jniEnv,
                                            classStringSeq,
                                            "str",
                                            "Ljava/lang/String;");
  jfieldID jfidStart = (*jniEnv)->GetFieldID (jniEnv,
                                              classStringSeq,
                                              "start_offset",
                                              "I");
  jfieldID jfidEnd = (*jniEnv)->GetFieldID (jniEnv,
                                            classStringSeq,
                                            "end_offset",
                                            "I");

  jstring jStr = (*jniEnv)->GetObjectField (jniEnv, jStrSeq, jfidStr);
  *start_offset = (gint) (*jniEnv)->GetIntField (jniEnv, jStrSeq, jfidStart);
  *end_offset = (gint) (*jniEnv)->GetIntField (jniEnv, jStrSeq, jfidEnd);

  return jaw_text_get_gtext_from_jstr (jniEnv, jStr);
}

/**
 * jaw_text_add_selection:
 * @text: an #AtkText
 * @start_offset: the starting character offset of the selected region
 * @end_offset: the offset of the first character after the selected region.
 *
 * Adds a selection bounded by the specified offsets.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * Returns: %TRUE if successful, %FALSE otherwise
 **/

static gboolean
jaw_text_add_selection (AtkText *text, gint start_offset, gint end_offset)
{
  JAW_DEBUG_C ("%p, %d, %d", text, start_offset, end_offset);
  JAW_GET_TEXT (text, FALSE);

  jclass classAtkText = (*jniEnv)->FindClass (jniEnv, "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkText,
                                           "add_selection",
                                           "(II)Z");
  jboolean jresult = (*jniEnv)->CallBooleanMethod (jniEnv,
                                                   atk_text,
                                                   jmid,
                                                   (jint) start_offset,
                                                   (jint) end_offset);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_text);

  return jresult;
}

/**
 * jaw_text_remove_selection:
 * @text: an #AtkText
 * @selection_num: The selection number.  The selected regions are
 * assigned numbers that correspond to how far the region is from the
 * start of the text.  The selected region closest to the beginning
 * of the text region is assigned the number 0, etc.  Note that adding,
 * moving or deleting a selected region can change the numbering.
 *
 * Removes the specified selection.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * Returns: %TRUE if successful, %FALSE otherwise
 **/

static gboolean
jaw_text_remove_selection (AtkText *text, gint selection_num)
{
  JAW_DEBUG_C ("%p, %d", text, selection_num);
  JAW_GET_TEXT (text, FALSE);

  jclass classAtkText = (*jniEnv)->FindClass (jniEnv,
                                              "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkText,
                                           "remove_selection",
                                           "(I)Z");
  jboolean jresult = (*jniEnv)->CallBooleanMethod (jniEnv,
                                                   atk_text,
                                                   jmid,
                                                   (jint) selection_num);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_text);

  return jresult;
}

/**
 * jaw_text_set_selection:
 * @text: an #AtkText
 * @selection_num: The selection number.  The selected regions are
 * assigned numbers that correspond to how far the region is from the
 * start of the text.  The selected region closest to the beginning
 * of the text region is assigned the number 0, etc.  Note that adding,
 * moving or deleting a selected region can change the numbering.
 * @start_offset: the new starting character offset of the selection
 * @end_offset: the new end position of (e.g. offset immediately past)
 * the selection
 *
 * Changes the start and end offset of the specified selection.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * Returns: %TRUE if successful, %FALSE otherwise
 **/

static gboolean
jaw_text_set_selection (AtkText *text, gint selection_num, gint start_offset, gint end_offset)
{
  JAW_DEBUG_C ("%p, %d, %d, %d", text, selection_num, start_offset, end_offset);
  JAW_GET_TEXT (text, FALSE);

  jclass classAtkText = (*jniEnv)->FindClass (jniEnv, "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv, classAtkText, "set_selection", "(III)Z");
  jboolean jresult = (*jniEnv)->CallBooleanMethod (jniEnv,
                                                   atk_text,
                                                   jmid,
                                                   (jint) selection_num,
                                                   (jint) start_offset,
                                                   (jint) end_offset);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_text);

  return jresult;
}

/**
 * jaw_text_set_caret_offset:
 * @text: an #AtkText
 * @offset: the character offset of the new caret position
 *
 * Sets the caret (cursor) position to the specified @offset.
 *
 * In the case of rich-text content, this method should either grab focus
 * or move the sequential focus navigation starting point (if the application
 * supports this concept) as if the user had clicked on the new caret position.
 * Typically, this means that the target of this operation is the node
 *containing the new caret position or one of its ancestors. In other words,
 *after this method is called, if the user advances focus, it should move to the
 *first focusable node following the new caret position.
 *
 * Calling this method should also scroll the application viewport in a way
 * that matches the behavior of the application's typical caret motion or tab
 * navigation as closely as possible. This also means that if the application's
 * caret motion or focus navigation does not trigger a scroll operation, this
 * method should not trigger one either. If the application does not have a
 *caret motion or focus navigation operation, this method should try to scroll
 *the new caret position into view while minimizing unnecessary scroll motion.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * Returns: %TRUE if successful, %FALSE otherwise.
 **/

static gboolean
jaw_text_set_caret_offset (AtkText *text, gint offset)
{
  JAW_DEBUG_C ("%p, %d", text, offset);
  JAW_GET_TEXT (text, FALSE);

  jclass classAtkText = (*jniEnv)->FindClass (jniEnv,
                                              "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkText,
                                           "set_caret_offset",
                                           "(I)Z");
  jboolean jresult = (*jniEnv)->CallBooleanMethod (jniEnv,
                                                   atk_text,
                                                   jmid,
                                                   (jint) offset);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_text);

  return jresult;
}
