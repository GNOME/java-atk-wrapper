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

extern void jaw_text_interface_init(AtkTextIface*);
extern gpointer jaw_text_data_init(jobject);
extern void jaw_text_data_finalize(gpointer);

static gchar* jaw_text_get_text(AtkText *text,
                                gint start_offset,
                                gint end_offset);

static gunichar jaw_text_get_character_at_offset(AtkText *text, gint offset);

static gchar* jaw_text_get_text_at_offset(AtkText *text,
                                          gint offset,
                                          AtkTextBoundary boundary_type,
                                          gint *start_offset,
                                          gint *end_offset);

static gint jaw_text_get_caret_offset(AtkText *text);

static void jaw_text_get_character_extents(AtkText *text,
                                            gint offset,
                                            gint *x,
                                            gint *y,
                                            gint *width,
                                            gint *height,
                                            AtkCoordType coords);

static gint jaw_text_get_character_count(AtkText *text);

static gint jaw_text_get_offset_at_point(AtkText *text,
                                          gint x,
                                          gint y,
                                          AtkCoordType coords);
static void jaw_text_get_range_extents(AtkText *text,
                                        gint start_offset,
                                        gint end_offset,
                                        AtkCoordType coord_type,
                                        AtkTextRectangle *rect);

static gint jaw_text_get_n_selections(AtkText *text);

static gchar* jaw_text_get_selection(AtkText *text,
                                     gint selection_num,
                                     gint *start_offset,
                                     gint *end_offset);

static gboolean jaw_text_add_selection(AtkText *text,
                                       gint start_offset,
                                       gint end_offset);

static gboolean jaw_text_remove_selection(AtkText *text, gint selection_num);

static gboolean jaw_text_set_selection(AtkText *text,
                                       gint selection_num,
                                       gint start_offset,
                                       gint end_offset);

static gboolean jaw_text_set_caret_offset(AtkText *text, gint offset);

typedef struct _TextData {
  jobject atk_text;
  gchar* text;
  jstring jstrText;
}TextData;

void
jaw_text_interface_init (AtkTextIface *iface)
{
  iface->get_text = jaw_text_get_text;
  iface->get_character_at_offset = jaw_text_get_character_at_offset;
  iface->get_text_at_offset = jaw_text_get_text_at_offset;
  iface->get_caret_offset = jaw_text_get_caret_offset;
  iface->get_character_extents = jaw_text_get_character_extents;
  iface->get_character_count = jaw_text_get_character_count;
  iface->get_offset_at_point = jaw_text_get_offset_at_point;
  iface->get_range_extents = jaw_text_get_range_extents;
  iface->get_n_selections = jaw_text_get_n_selections;
  iface->get_selection = jaw_text_get_selection;
  iface->add_selection = jaw_text_add_selection;
  iface->remove_selection = jaw_text_remove_selection;
  iface->set_selection = jaw_text_set_selection;
  iface->set_caret_offset = jaw_text_set_caret_offset;
}

gpointer
jaw_text_data_init (jobject ac)
{
  TextData *data = g_new0(TextData, 1);

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classText = (*jniEnv)->FindClass(jniEnv,
                                          "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classText,
                                          "<init>",
                                          "(Ljavax/accessibility/AccessibleContext;)V");
  jobject jatk_text = (*jniEnv)->NewObject(jniEnv, classText, jmid, ac);
  data->atk_text = (*jniEnv)->NewGlobalRef(jniEnv, jatk_text);

  return data;
}

void
jaw_text_data_finalize (gpointer p)
{
  TextData *data = (TextData*)p;
  JNIEnv *jniEnv = jaw_util_get_jni_env();

  if (data && data->atk_text)
  {
    if (data->text != NULL)
    {
      (*jniEnv)->ReleaseStringUTFChars(jniEnv, data->jstrText, data->text);
      (*jniEnv)->DeleteGlobalRef(jniEnv, data->jstrText);
      data->jstrText = NULL;
      data->text = NULL;
    }

    (*jniEnv)->DeleteGlobalRef(jniEnv, data->atk_text);
    data->atk_text = NULL;
  }
}

static gchar*
jaw_text_get_gtext_from_jstr (JNIEnv *jniEnv, TextData *data, jstring jstr)
{
  if (jstr == NULL)
  {
    return NULL;
  }

  gchar* tmp_text = (gchar*)(*jniEnv)->GetStringUTFChars(jniEnv, jstr, NULL);
  gchar* text = g_strdup(tmp_text);
  (*jniEnv)->ReleaseStringUTFChars(jniEnv, jstr, tmp_text);

  return text;
}

static gchar*
jaw_text_get_text (AtkText *text, gint start_offset, gint end_offset)
{
  JawObject *jaw_obj = JAW_OBJECT(text);
  TextData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TEXT);
  jobject atk_text = data->atk_text;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classAtkText = (*jniEnv)->FindClass(jniEnv,
                                             "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkText,
                                          "get_text",
                                          "(II)Ljava/lang/String;");

  jstring jstr = (*jniEnv)->CallObjectMethod(jniEnv,
                                             atk_text,
                                             jmid,
                                             (jint)start_offset,
                                             (jint)end_offset );

  return jaw_text_get_gtext_from_jstr(jniEnv, data, jstr);
}

static gunichar
jaw_text_get_character_at_offset (AtkText *text, gint offset)
{
  JawObject *jaw_obj = JAW_OBJECT(text);
  TextData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TEXT);
  jobject atk_text = data->atk_text;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classAtkText = (*jniEnv)->FindClass(jniEnv,
                                             "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkText,
                                          "get_character_at_offset", "(I)C");
  jchar jcharacter = (*jniEnv)->CallCharMethod(jniEnv,
                                               atk_text,
                                               jmid,
                                               (jint)offset );

  return (gunichar)jcharacter;
}

static gchar*
jaw_text_get_text_at_offset (AtkText *text,
                             gint offset,
                             AtkTextBoundary boundary_type,
                             gint *start_offset, gint *end_offset)
{
  JawObject *jaw_obj = JAW_OBJECT(text);
  TextData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TEXT);
  jobject atk_text = data->atk_text;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classAtkText = (*jniEnv)->FindClass(jniEnv,
                                             "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkText,
                                          "get_text_at_offset",
                                          "(II)Lorg/GNOME/Accessibility/AtkText$StringSequence;");
  jobject jStrSeq = (*jniEnv)->CallObjectMethod(jniEnv,
                                                atk_text,
                                                jmid,
                                                (jint)offset,
                                                (jint)boundary_type );

  if (jStrSeq == NULL)
  {
    return NULL;
  }

  jclass classStringSeq = (*jniEnv)->FindClass(jniEnv,
                                               "org/GNOME/Accessibility/AtkText$StringSequence");
  jfieldID jfidStr = (*jniEnv)->GetFieldID(jniEnv,
                                           classStringSeq,
                                           "str",
                                           "Ljava/lang/String;");
  jfieldID jfidStart = (*jniEnv)->GetFieldID(jniEnv,
                                             classStringSeq,
                                             "start_offset",
                                             "I");
  jfieldID jfidEnd = (*jniEnv)->GetFieldID(jniEnv,
                                           classStringSeq,
                                           "end_offset",
                                           "I");

  jstring jStr = (*jniEnv)->GetObjectField(jniEnv, jStrSeq, jfidStr);
  jint jStart = (*jniEnv)->GetIntField(jniEnv, jStrSeq, jfidStart);
  jint jEnd = (*jniEnv)->GetIntField(jniEnv, jStrSeq, jfidEnd);

  (*start_offset) = (gint)jStart;
  (*end_offset) = (gint)jEnd;

  return jaw_text_get_gtext_from_jstr(jniEnv, data, jStr);
}

static gint
jaw_text_get_caret_offset (AtkText *text)
{
  JawObject *jaw_obj = JAW_OBJECT(text);
  TextData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TEXT);
  jobject atk_text = data->atk_text;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classAtkText = (*jniEnv)->FindClass(jniEnv,
                                             "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkText,
                                          "get_caret_offset",
                                          "()I");
  jint joffset = (*jniEnv)->CallIntMethod(jniEnv, atk_text, jmid);

  return (gint)joffset;
}

static void
jaw_text_get_character_extents (AtkText *text,
                                gint offset, gint *x, gint *y,
                                gint *width, gint *height,
                                AtkCoordType coords)
{
  JawObject *jaw_obj = JAW_OBJECT(text);
  TextData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TEXT);
  jobject atk_text = data->atk_text;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classAtkText = (*jniEnv)->FindClass(jniEnv,
                                             "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkText,
                                          "get_character_extents",
                                          "(II)Ljava/awt/Rectangle;");
  jobject jrect = (*jniEnv)->CallObjectMethod(jniEnv,
                                              atk_text,
                                              jmid,
                                              (jint)offset,
                                              (jint)coords);

  if (jrect == NULL)
  {
    return;
  }

  jaw_util_get_rect_info(jniEnv, jrect, x, y, width, height);
}

static gint
jaw_text_get_character_count (AtkText *text)
{
  JawObject *jaw_obj = JAW_OBJECT(text);
  TextData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TEXT);
  jobject atk_text = data->atk_text;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classAtkText = (*jniEnv)->FindClass(jniEnv,
                                             "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkText,
                                          "get_character_count",
                                          "()I");
  jint jcount = (*jniEnv)->CallIntMethod(jniEnv, atk_text, jmid);

  return (gint)jcount;
}

static gint
jaw_text_get_offset_at_point (AtkText *text, gint x, gint y, AtkCoordType coords)
{
  JawObject *jaw_obj = JAW_OBJECT(text);
  TextData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TEXT);
  jobject atk_text = data->atk_text;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classAtkText = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkText,
                                          "get_offset_at_point",
                                          "(III)I");
  jint joffset = (*jniEnv)->CallIntMethod(jniEnv,
                                          atk_text,
                                          jmid,
                                          (jint)x,
                                          (jint)y,
                                          (jint)coords);

  return (gint)joffset;
}

static void
jaw_text_get_range_extents (AtkText *text,
                            gint start_offset,
                            gint end_offset,
                            AtkCoordType coord_type,
                            AtkTextRectangle *rect)
{
  if (rect == NULL)
  {
    return;
  }

  JawObject *jaw_obj = JAW_OBJECT(text);
  TextData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TEXT);
  jobject atk_text = data->atk_text;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classAtkText = (*jniEnv)->FindClass(jniEnv,
                                             "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkText,
                                          "get_range_extents",
                                          "(III)Ljava/awt/Rectangle;");
  jobject jrect = (*jniEnv)->CallObjectMethod(jniEnv,
                                              atk_text,
                                              jmid,
                                              (jint)start_offset,
                                              (jint)end_offset,
                                              (jint)coord_type);

  if (!jrect)
  {
    return;
  }

  jaw_util_get_rect_info(jniEnv, jrect, &(rect->x), &(rect->y), &(rect->width), &(rect->height));
}

static gint
jaw_text_get_n_selections (AtkText *text)
{
  JawObject *jaw_obj = JAW_OBJECT(text);
  TextData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TEXT);
  jobject atk_text = data->atk_text;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classAtkText = (*jniEnv)->FindClass(jniEnv,
                                             "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkText,
                                          "get_n_selections",
                                          "()I");
  jint jselections = (*jniEnv)->CallIntMethod(jniEnv, atk_text, jmid);

  return (gint)jselections;
}

static gchar*
jaw_text_get_selection (AtkText *text, gint selection_num, gint *start_offset, gint *end_offset)
{
  JawObject *jaw_obj = JAW_OBJECT(text);
  TextData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TEXT);
  jobject atk_text = data->atk_text;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classAtkText = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkText,
                                          "get_selection",
                                          "()Lorg/GNOME/Accessibility/AtkText$StringSequence;");
  jobject jStrSeq = (*jniEnv)->CallObjectMethod(jniEnv, atk_text, jmid);

  if (jStrSeq == NULL)
  {
    return NULL;
  }

  jclass classStringSeq = (*jniEnv)->FindClass(jniEnv,
                                               "org/GNOME/Accessibility/AtkText$StringSequence");
  jfieldID jfidStr = (*jniEnv)->GetFieldID(jniEnv,
                                           classStringSeq,
                                           "str",
                                           "Ljava/lang/String;");
  jfieldID jfidStart = (*jniEnv)->GetFieldID(jniEnv,
                                             classStringSeq,
                                             "start_offset",
                                             "I");
  jfieldID jfidEnd = (*jniEnv)->GetFieldID(jniEnv,
                                           classStringSeq,
                                           "end_offset",
                                           "I");

  jstring jStr = (*jniEnv)->GetObjectField(jniEnv, jStrSeq, jfidStr);
  *start_offset = (gint)(*jniEnv)->GetIntField(jniEnv, jStrSeq, jfidStart);
  *end_offset = (gint)(*jniEnv)->GetIntField(jniEnv, jStrSeq, jfidEnd);

  return jaw_text_get_gtext_from_jstr(jniEnv, data, jStr);
}

static gboolean
jaw_text_add_selection (AtkText *text, gint start_offset, gint end_offset)
{
  JawObject *jaw_obj = JAW_OBJECT(text);
  TextData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TEXT);
  jobject atk_text = data->atk_text;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classAtkText = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                         classAtkText,
                                         "add_selection",
                                         "(II)Z");
  jboolean jresult = (*jniEnv)->CallBooleanMethod(jniEnv,
                                                  atk_text,
                                                  jmid,
                                                  (jint)start_offset,
                                                  (jint)end_offset);

  if (jresult == JNI_TRUE)
  {
    return TRUE;
  } else {
    return FALSE;
  }
}

static gboolean
jaw_text_remove_selection (AtkText *text, gint selection_num)
{
  JawObject *jaw_obj = JAW_OBJECT(text);
  TextData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TEXT);
  jobject atk_text = data->atk_text;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classAtkText = (*jniEnv)->FindClass(jniEnv,
                                             "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkText,
                                          "remove_selection",
                                          "(I)Z");
  jboolean jresult = (*jniEnv)->CallBooleanMethod(jniEnv,
                                                  atk_text,
                                                  jmid,
                                                  (jint)selection_num);

  if (jresult == JNI_TRUE)
  {
    return TRUE;
  } else {
    return FALSE;
  }
}

static gboolean
jaw_text_set_selection (AtkText *text, gint selection_num, gint start_offset, gint end_offset)
{
  JawObject *jaw_obj = JAW_OBJECT(text);
  TextData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TEXT);
  jobject atk_text = data->atk_text;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classAtkText = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkText, "set_selection", "(III)Z");
  jboolean jresult = (*jniEnv)->CallBooleanMethod(jniEnv,
                                                  atk_text,
                                                  jmid,
                                                  (jint)selection_num,
                                                  (jint)start_offset,
                                                  (jint)end_offset);

  if (jresult == JNI_TRUE) {
    return TRUE;
  } else {
    return FALSE;
  }
}

static gboolean
jaw_text_set_caret_offset (AtkText *text, gint offset)
{
  JawObject *jaw_obj = JAW_OBJECT(text);
  TextData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_TEXT);
  jobject atk_text = data->atk_text;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classAtkText = (*jniEnv)->FindClass(jniEnv,
                                             "org/GNOME/Accessibility/AtkText");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkText,
                                          "set_caret_offset",
                                          "(I)Z");
  jboolean jresult = (*jniEnv)->CallBooleanMethod(jniEnv,
                                                  atk_text,
                                                  jmid,
                                                  (jint)offset);

  if (jresult == JNI_TRUE)
  {
    return TRUE;
  } else {
    return FALSE;
  }
}

