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

#include <atk/atk.h>
#include <glib.h>
#include "jawimpl.h"
#include "jawutil.h"
#include "jawobject.h"

extern void jaw_editable_text_interface_init(AtkEditableTextIface*);
extern gpointer jaw_editable_text_data_init(jobject);
extern void jaw_editable_text_data_finalize(gpointer);

static void jaw_editable_text_set_text_contents(AtkEditableText *text,
                                                const gchar *string);
static void jaw_editable_text_insert_text(AtkEditableText *text,
                                          const gchar *string,
                                          gint length,
                                          gint *position);
static void jaw_editable_text_copy_text(AtkEditableText *text,
                                        gint start_pos,
                                        gint end_pos);
static void jaw_editable_text_cut_text(AtkEditableText *text,
                                       gint start_pos,
                                       gint end_pos);
static void jaw_editable_text_delete_text(AtkEditableText *text,
                                          gint start_pos,
                                          gint end_pos);
static void jaw_editable_text_paste_text(AtkEditableText *text,
                                         gint position);

static gboolean jaw_editable_text_set_run_attributes(AtkEditableText *text,
                                                                                      AtkAttributeSet  *attrib_set,
                                                                                      gint                      start_offset,
                                                                                      gint                 end_offset);

typedef struct _EditableTextData {
  jobject atk_editable_text;
}EditableTextData;

void
jaw_editable_text_interface_init (AtkEditableTextIface *iface)
{
  iface->set_text_contents = jaw_editable_text_set_text_contents;
  iface->insert_text = jaw_editable_text_insert_text;
  iface->copy_text = jaw_editable_text_copy_text;
  iface->cut_text = jaw_editable_text_cut_text;
  iface->delete_text = jaw_editable_text_delete_text;
  iface->paste_text = jaw_editable_text_paste_text;
  iface->set_run_attributes = jaw_editable_text_set_run_attributes;
}

gpointer
jaw_editable_text_data_init (jobject ac)
{
  EditableTextData *data = g_new0(EditableTextData, 1);

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classEditableText = (*jniEnv)->FindClass(jniEnv,
                                                  "org/GNOME/Accessibility/AtkEditableText");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classEditableText,
                                          "<init>",
                                          "(Ljavax/accessibility/AccessibleContext;)V");
  jobject jatk_editable_text = (*jniEnv)->NewObject(jniEnv,
                                                    classEditableText,
                                                    jmid,
                                                    ac);
  data->atk_editable_text = (*jniEnv)->NewGlobalRef(jniEnv,
                                                    jatk_editable_text);

  return data;
}

void
jaw_editable_text_data_finalize (gpointer p)
{
  EditableTextData *data = (EditableTextData*)p;
  JNIEnv *jniEnv = jaw_util_get_jni_env();

  if (data && data->atk_editable_text)
  {
    (*jniEnv)->DeleteGlobalRef(jniEnv, data->atk_editable_text);
    data->atk_editable_text = NULL;
  }
}

void
jaw_editable_text_set_text_contents (AtkEditableText *text,
                                     const gchar *string)
{
  JawObject *jaw_obj = JAW_OBJECT(text);
  EditableTextData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_EDITABLE_TEXT);
  jobject atk_editable_text = data->atk_editable_text;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classAtkEditableText = (*jniEnv)->FindClass(jniEnv,
                                                     "org/GNOME/Accessibility/AtkEditableText");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkEditableText,
                                          "set_text_contents",
                                          "(Ljava/lang/String;)V");

  jstring jstr = (*jniEnv)->NewStringUTF(jniEnv, string);
  (*jniEnv)->CallVoidMethod(jniEnv, atk_editable_text, jmid, jstr);
}

void
jaw_editable_text_insert_text (AtkEditableText *text,
                               const gchar *string,
                               gint length,
                               gint *position)
{
  JawObject *jaw_obj = JAW_OBJECT(text);
  EditableTextData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_EDITABLE_TEXT);
  jobject atk_editable_text = data->atk_editable_text;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classAtkEditableText = (*jniEnv)->FindClass(jniEnv,
                                                     "org/GNOME/Accessibility/AtkEditableText");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkEditableText,
                                         "insert_text",
                                         "(Ljava/lang/String;I)V");

  jstring jstr = (*jniEnv)->NewStringUTF(jniEnv, string);
  (*jniEnv)->CallVoidMethod(jniEnv,
                            atk_editable_text,
                            jmid, jstr,
                            (jint)*position);
  *position = *position + length;
  atk_text_set_caret_offset(ATK_TEXT(jaw_obj), *position);
}

void
jaw_editable_text_copy_text (AtkEditableText *text,
                             gint start_pos,
                             gint end_pos)
{
  JawObject *jaw_obj = JAW_OBJECT(text);
  EditableTextData *data = jaw_object_get_interface_data(jaw_obj,
                                                         INTERFACE_EDITABLE_TEXT);
  jobject atk_editable_text = data->atk_editable_text;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classAtkEditableText = (*jniEnv)->FindClass(jniEnv,
                                                     "org/GNOME/Accessibility/AtkEditableText");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkEditableText,
                                          "copy_text",
                                          "(II)V");
  (*jniEnv)->CallVoidMethod(jniEnv,
                            atk_editable_text,
                            jmid,
                            (jint)start_pos,
                            (jint)end_pos);
}

void
jaw_editable_text_cut_text (AtkEditableText *text,
                            gint start_pos, gint end_pos)
{
  JawObject *jaw_obj = JAW_OBJECT(text);
  EditableTextData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_EDITABLE_TEXT);
  jobject atk_editable_text = data->atk_editable_text;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classAtkEditableText = (*jniEnv)->FindClass(jniEnv,
                                                     "org/GNOME/Accessibility/AtkEditableText");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkEditableText,
                                          "cut_text",
                                          "(II)V");
  (*jniEnv)->CallVoidMethod(jniEnv,
                            atk_editable_text,
                            jmid,
                            (jint)start_pos,
                            (jint)end_pos);
}

void
jaw_editable_text_delete_text (AtkEditableText *text,
                               gint start_pos,
                               gint end_pos)
{
  JawObject *jaw_obj = JAW_OBJECT(text);
  EditableTextData *data = jaw_object_get_interface_data(jaw_obj,
                                                         INTERFACE_EDITABLE_TEXT);
  jobject atk_editable_text = data->atk_editable_text;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classAtkEditableText = (*jniEnv)->FindClass(jniEnv,
                                                     "org/GNOME/Accessibility/AtkEditableText");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkEditableText,
                                          "delete_text",
                                          "(II)V");
  (*jniEnv)->CallVoidMethod(jniEnv,
                            atk_editable_text,
                            jmid,
                            (jint)start_pos,
                            (jint)end_pos);
}

void
jaw_editable_text_paste_text (AtkEditableText *text,
                              gint position)
{
  JawObject *jaw_obj = JAW_OBJECT(text);
  EditableTextData *data = jaw_object_get_interface_data(jaw_obj,
                                                         INTERFACE_EDITABLE_TEXT);
  jobject atk_editable_text = data->atk_editable_text;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classAtkEditableText = (*jniEnv)->FindClass(jniEnv,
                                                     "org/GNOME/Accessibility/AtkEditableText");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkEditableText,
                                          "paste_text",
                                          "(I)V");
  (*jniEnv)->CallVoidMethod(jniEnv,
                            atk_editable_text,
                            jmid,
                            (jint)position);
}

static gboolean
jaw_editable_text_set_run_attributes(AtkEditableText *text,
                                                           AtkAttributeSet  *attrib_set,
                                                           gint                      start_offset,
                                                           gint                      end_offset)
{
  JawObject *jaw_obj = JAW_OBJECT(text);
  EditableTextData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_EDITABLE_TEXT);
  jobject atk_editable_text = data->atk_editable_text;
  JNIEnv *env = jaw_util_get_jni_env();
  jclass classAtkEditableText = (*env)->FindClass(env, "org/GNOME/Accessibility/AtkEditableText");
  jmethodID jmid = (*env)->GetMethodID(env,
                                                                   classAtkEditableText,
                                                                   "setRunAttributes",
                                                                   "(Ljavax/swing/text/AttributeSet;II)Z");
  jboolean jresult = (*env)->CallBooleanMethod(env,
                                                                            atk_editable_text,
                                                                            jmid,
                                                                            (jobject)attrib_set,
                                                                            (jint)start_offset,
                                                                            (jint)end_offset);
  if (jresult == JNI_TRUE)
    return TRUE;

  return FALSE;
}

