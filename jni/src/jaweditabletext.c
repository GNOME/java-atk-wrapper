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
#include "jawobject.h"
#include "jawutil.h"
#include <atk/atk.h>
#include <glib.h>

/**
 * (From Atk documentation)
 *
 * AtkEditableText:
 *
 * The ATK interface implemented by components containing user-editable text
 * content.
 *
 * #AtkEditableText should be implemented by UI components which
 * contain text which the user can edit, via the #AtkObject
 * corresponding to that component (see #AtkObject).
 *
 * #AtkEditableText is a subclass of #AtkText, and as such, an object
 * which implements #AtkEditableText is by definition an #AtkText
 * implementor as well.
 *
 * See [iface@AtkText]
 */

static void jaw_editable_text_set_text_contents (AtkEditableText *text,
                                                 const gchar *string);
static void jaw_editable_text_insert_text (AtkEditableText *text,
                                           const gchar *string,
                                           gint length,
                                           gint *position);
static void jaw_editable_text_copy_text (AtkEditableText *text,
                                         gint start_pos,
                                         gint end_pos);
static void jaw_editable_text_cut_text (AtkEditableText *text,
                                        gint start_pos,
                                        gint end_pos);
static void jaw_editable_text_delete_text (AtkEditableText *text,
                                           gint start_pos,
                                           gint end_pos);
static void jaw_editable_text_paste_text (AtkEditableText *text,
                                          gint position);

static gboolean jaw_editable_text_set_run_attributes (AtkEditableText *text,
                                                      AtkAttributeSet *attrib_set,
                                                      gint start_offset,
                                                      gint end_offset);

typedef struct _EditableTextData
{
  jobject atk_editable_text;
} EditableTextData;

#define JAW_GET_EDITABLETEXT(text, def_ret) \
  JAW_GET_OBJ_IFACE (text, INTERFACE_EDITABLE_TEXT, EditableTextData, atk_editable_text, jniEnv, atk_editable_text, def_ret)

/**
 * AtkEditableTextIface:
 * @set_run_attributes:
 * @set_text_contents:
 * @copy_text:
 * @cut_text:
 * @delete_text:
 * @paste_text:
 **/

void
jaw_editable_text_interface_init (AtkEditableTextIface *iface, gpointer data)
{
  JAW_DEBUG_ALL ("%p,%p", iface, data);
  iface->set_run_attributes = jaw_editable_text_set_run_attributes;
  iface->set_text_contents = jaw_editable_text_set_text_contents;
  iface->insert_text = jaw_editable_text_insert_text;
  iface->copy_text = jaw_editable_text_copy_text;
  iface->cut_text = jaw_editable_text_cut_text;
  iface->delete_text = jaw_editable_text_delete_text;
  iface->paste_text = jaw_editable_text_paste_text;
}

/**
 * jaw_editable_text_data_init:
 * @ac: a Java AccessibleContext object
 *
 * Initializes editable text interface data for an AccessibleContext.
 *
 * Explicitly manages a JNI local reference frame using
 * PushLocalFrame/PopLocalFrame; all local references are released
 * before the function returns.
 *
 * Returns: (transfer full): EditableTextData pointer, or %NULL on error
 */

gpointer
jaw_editable_text_data_init (jobject ac)
{
  JAW_DEBUG_ALL ("%p", ac);
  EditableTextData *data = g_new0 (EditableTextData, 1);

  JNIEnv *jniEnv = jaw_util_get_jni_env ();
  jclass classEditableText = (*jniEnv)->FindClass (jniEnv,
                                                   "org/GNOME/Accessibility/AtkEditableText");
  jmethodID jmid = (*jniEnv)->GetStaticMethodID (jniEnv,
                                                 classEditableText,
                                                 "createAtkEditableText",
                                                 "(Ljavax/accessibility/AccessibleContext;)Lorg/GNOME/Accessibility/AtkEditableText;");
  jobject jatk_editable_text = (*jniEnv)->CallStaticObjectMethod (jniEnv,
                                                                  classEditableText,
                                                                  jmid,
                                                                  ac);
  data->atk_editable_text = (*jniEnv)->NewGlobalRef (jniEnv,
                                                     jatk_editable_text);

  return data;
}

/**
 * jaw_editable_text_data_finalize:
 * @p: EditableTextData pointer to finalize
 *
 * Cleans up EditableTextData when the parent GObject is finalized.
 * Called from jaw_impl_finalize() when the object's reference count reaches
 * zero.
 */

void
jaw_editable_text_data_finalize (gpointer p)
{
  JAW_DEBUG_ALL ("%p", p);
  EditableTextData *data = (EditableTextData *) p;
  JNIEnv *jniEnv = jaw_util_get_jni_env ();

  if (data && data->atk_editable_text)
    {
      (*jniEnv)->DeleteGlobalRef (jniEnv, data->atk_editable_text);
      data->atk_editable_text = NULL;
    }
}

/**
 * jaw_editable_text_set_text_contents:
 * @text: an #AtkEditableText
 * @string: string to set for text contents of @text
 *
 * Sets the entire contents of @text to the specified string.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 */

void
jaw_editable_text_set_text_contents (AtkEditableText *text,
                                     const gchar *string)
{
  JAW_DEBUG_C ("%p, %s", text, string);
  JAW_GET_EDITABLETEXT (text, );

  jclass classAtkEditableText = (*jniEnv)->FindClass (jniEnv,
                                                      "org/GNOME/Accessibility/AtkEditableText");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkEditableText,
                                           "set_text_contents",
                                           "(Ljava/lang/String;)V");

  jstring jstr = (*jniEnv)->NewStringUTF (jniEnv, string);
  (*jniEnv)->CallVoidMethod (jniEnv, atk_editable_text, jmid, jstr);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_editable_text);
}

/**
 * jaw_editable_text_insert_text:
 * @text: an #AtkEditableText
 * @string: the text to insert
 * @length: the length of text to insert, in bytes. If len is negative, then the
 * string is nul-terminated.
 * @position: (inout): The caller initializes this to the position at which to
 *   insert the text. After the call it points at the position after the newly
 *   inserted text.
 *
 * Inserts text at a given position.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 */

void
jaw_editable_text_insert_text (AtkEditableText *text,
                               const gchar *string,
                               gint length,
                               gint *position)
{
  JAW_DEBUG_C ("%p, %s, %d, %p", text, string, length, position);
  JAW_GET_EDITABLETEXT (text, );

  jclass classAtkEditableText = (*jniEnv)->FindClass (jniEnv,
                                                      "org/GNOME/Accessibility/AtkEditableText");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkEditableText,
                                           "insert_text",
                                           "(Ljava/lang/String;I)V");

  jstring jstr = (*jniEnv)->NewStringUTF (jniEnv, string);
  (*jniEnv)->CallVoidMethod (jniEnv,
                             atk_editable_text,
                             jmid, jstr,
                             (jint) *position);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_editable_text);
  *position = *position + length;
  atk_text_set_caret_offset (ATK_TEXT (jaw_obj), *position);
}

/**
 * jaw_editable_text_copy_text:
 * @text: an #AtkEditableText
 * @start_pos: start position
 * @end_pos: end position
 *
 * Copies text from @start_pos up to, but not including @end_pos to the
 * clipboard.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 */

void
jaw_editable_text_copy_text (AtkEditableText *text,
                             gint start_pos,
                             gint end_pos)
{
  JAW_DEBUG_C ("%p, %d, %d", text, start_pos, end_pos);
  JAW_GET_EDITABLETEXT (text, );

  jclass classAtkEditableText = (*jniEnv)->FindClass (jniEnv,
                                                      "org/GNOME/Accessibility/AtkEditableText");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkEditableText,
                                           "copy_text",
                                           "(II)V");
  (*jniEnv)->CallVoidMethod (jniEnv,
                             atk_editable_text,
                             jmid,
                             (jint) start_pos,
                             (jint) end_pos);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_editable_text);
}

/**
 * jaw_editable_text_cut_text:
 * @text: an #AtkEditableText
 * @start_pos: start position
 * @end_pos: end position
 *
 * Copies text from @start_pos up to, but not including @end_pos to the
 * clipboard and then deletes the text from the widget.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 */

void
jaw_editable_text_cut_text (AtkEditableText *text,
                            gint start_pos,
                            gint end_pos)
{
  JAW_DEBUG_C ("%p, %d, %d", text, start_pos, end_pos);
  JAW_GET_EDITABLETEXT (text, );

  jclass classAtkEditableText = (*jniEnv)->FindClass (jniEnv,
                                                      "org/GNOME/Accessibility/AtkEditableText");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkEditableText,
                                           "cut_text",
                                           "(II)V");
  (*jniEnv)->CallVoidMethod (jniEnv,
                             atk_editable_text,
                             jmid,
                             (jint) start_pos,
                             (jint) end_pos);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_editable_text);
}

/**
 * jaw_editable_text_delete_text:
 * @text: an #AtkEditableText
 * @start_pos: start position
 * @end_pos: end position
 *
 * Deletes text from @start_pos up to, but not including @end_pos.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 */

void
jaw_editable_text_delete_text (AtkEditableText *text,
                               gint start_pos,
                               gint end_pos)
{
  JAW_DEBUG_C ("%p, %d, %d", text, start_pos, end_pos);
  JAW_GET_EDITABLETEXT (text, );

  jclass classAtkEditableText = (*jniEnv)->FindClass (jniEnv,
                                                      "org/GNOME/Accessibility/AtkEditableText");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkEditableText,
                                           "delete_text",
                                           "(II)V");
  (*jniEnv)->CallVoidMethod (jniEnv,
                             atk_editable_text,
                             jmid,
                             (jint) start_pos,
                             (jint) end_pos);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_editable_text);
}

/**
 * jaw_editable_text_paste_text:
 * @text: an #AtkEditableText
 * @position: position to paste
 *
 * Pastes text from the clipboard to the specified @position.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 */

void
jaw_editable_text_paste_text (AtkEditableText *text,
                              gint position)
{
  JAW_DEBUG_C ("%p, %d", text, position);
  JAW_GET_EDITABLETEXT (text, );

  jclass classAtkEditableText = (*jniEnv)->FindClass (jniEnv,
                                                      "org/GNOME/Accessibility/AtkEditableText");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkEditableText,
                                           "paste_text",
                                           "(I)V");
  (*jniEnv)->CallVoidMethod (jniEnv,
                             atk_editable_text,
                             jmid,
                             (jint) position);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_editable_text);
}

static gboolean
jaw_editable_text_set_run_attributes (AtkEditableText *text,
                                      AtkAttributeSet *attrib_set,
                                      gint start_offset,
                                      gint end_offset)
{
  JAW_DEBUG_C ("%p, %p, %d, %d", text, attrib_set, start_offset, end_offset);
  JAW_GET_EDITABLETEXT (text, FALSE);

  jclass classAtkEditableText = (*jniEnv)->FindClass (jniEnv, "org/GNOME/Accessibility/AtkEditableText");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkEditableText,
                                           "setRunAttributes",
                                           "(Ljavax/swing/text/AttributeSet;II)Z");
  jboolean jresult = (*jniEnv)->CallBooleanMethod (jniEnv,
                                                   atk_editable_text,
                                                   jmid,
                                                   (jobject) attrib_set,
                                                   (jint) start_offset,
                                                   (jint) end_offset);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_editable_text);
  return jresult;
}
