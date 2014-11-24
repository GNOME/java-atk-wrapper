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

extern void jaw_component_interface_init(AtkComponentIface*);
extern gpointer jaw_component_data_init(jobject);
extern void jaw_component_data_finalize(gpointer);

static gboolean jaw_component_contains(AtkComponent *component,
                                       gint         x,
                                       gint         y,
                                       AtkCoordType coord_type);

static AtkObject* jaw_component_ref_accessible_at_point(AtkComponent *component,
                                                        gint         x,
                                                        gint         y,
                                                        AtkCoordType coord_type);

static void jaw_component_get_extents(AtkComponent *component,
                                      gint         *x,
                                      gint         *y,
                                      gint         *width,
                                      gint         *height,
                                      AtkCoordType coord_type);

static gboolean jaw_component_set_extents(AtkComponent *component,
                                          gint         *x,
                                          gint         *y,
                                          gint         *width,
                                          gint         *height,
                                          AtkCoordType coord_type);

static gboolean jaw_component_grab_focus(AtkComponent *component);
static AtkLayer jaw_component_get_layer(AtkComponent *component);
/*static gint		jaw_component_get_mdi_zorder		(AtkComponent		*component);
static gdouble		jaw_component_get_alpha			(AtkComponent		*component);*/

typedef struct _ComponentData {
  jobject atk_component;
} ComponentData;

void
jaw_component_interface_init (AtkComponentIface *iface)
{
  iface->contains = jaw_component_contains;
  iface->ref_accessible_at_point = jaw_component_ref_accessible_at_point;
  iface->get_extents = jaw_component_get_extents;
  iface->grab_focus = jaw_component_grab_focus;
  iface->get_layer = jaw_component_get_layer;
  iface->get_mdi_zorder = NULL; /*jaw_component_get_mdi_zorder;*/
  iface->get_alpha = NULL; /*jaw_component_get_alpha;*/
  iface->set_extents = jaw_component_set_extents;
  iface->set_position = NULL;
  iface->set_size = NULL;
  iface->bounds_changed = NULL;
}

gpointer
jaw_component_data_init (jobject ac)
{
  ComponentData *data = g_new0(ComponentData, 1);

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classComponent = (*jniEnv)->FindClass(jniEnv,
                                               "org/GNOME/Accessibility/AtkComponent");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classComponent,
                                          "<init>",
                                          "(Ljavax/accessibility/AccessibleContext;)V");

  jobject jatk_component = (*jniEnv)->NewObject(jniEnv, classComponent, jmid, ac);
  data->atk_component = (*jniEnv)->NewGlobalRef(jniEnv, jatk_component);

  return data;
}

void
jaw_component_data_finalize (gpointer p)
{
  ComponentData *data = (ComponentData*)p;
  JNIEnv *jniEnv = jaw_util_get_jni_env();

  if (data && data->atk_component)
  {
    (*jniEnv)->DeleteGlobalRef(jniEnv, data->atk_component);
    data->atk_component = NULL;
  }
}

static void
coord_screen_to_local (JNIEnv *jniEnv, jobject jobj, gint *x, gint *y)
{
  jclass classAccessibleComponent = (*jniEnv)->FindClass(jniEnv,
                                                         "javax/accessibility/AccessibleComponent");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAccessibleComponent,
                                          "getLocationOnScreen",
                                          "()Ljava/awt/Point;");

  jobject jpoint = (*jniEnv)->CallObjectMethod(jniEnv, jobj, jmid);

  jclass classPoint = (*jniEnv)->FindClass(jniEnv, "java/awt/Point");
  jfieldID jfidX = (*jniEnv)->GetFieldID(jniEnv, classPoint, "x", "I");
  jfieldID jfidY = (*jniEnv)->GetFieldID(jniEnv, classPoint, "y", "I");
  jint jx = (*jniEnv)->GetIntField(jniEnv, jpoint, jfidX);
  jint jy = (*jniEnv)->GetIntField(jniEnv, jpoint, jfidY);

  (*x) = (*x) - (gint)jx;
  (*y) = (*y) - (gint)jy;
}

static gboolean
jaw_component_contains (AtkComponent *component, gint x, gint y, AtkCoordType coord_type)
{
  JawObject *jaw_obj = JAW_OBJECT(component);
  ComponentData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_COMPONENT);
  jobject atk_component = data->atk_component;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classAtkComponent = (*jniEnv)->FindClass(jniEnv,
                                                  "org/GNOME/Accessibility/AtkComponent");

  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkComponent,
                                          "contains",
                                          "(III)Z");

  jboolean jcontains = (*jniEnv)->CallBooleanMethod(jniEnv,
                                                    atk_component,
                                                    jmid,
                                                    (jint)x,
                                                    (jint)y,
                                                    (jint)coord_type);

  if (jcontains == JNI_TRUE)
  {
    return TRUE;
  } else {
    return FALSE;
  }
}

static AtkObject*
jaw_component_ref_accessible_at_point (AtkComponent *component, gint x, gint y, AtkCoordType coord_type)
{
  JawObject *jaw_obj = JAW_OBJECT(component);
  ComponentData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_COMPONENT);
  jobject atk_component = data->atk_component;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classAtkComponent = (*jniEnv)->FindClass(jniEnv,
                                                  "org/GNOME/Accessibility/AtkComponent");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkComponent,
                                          "get_accessible_at_point",
                                          "(III)Ljavax/accessibility/AccessibleContext;");
  jobject child_ac = (*jniEnv)->CallObjectMethod(jniEnv,
                                                 atk_component,
                                                 jmid,
                                                 (jint)x,
                                                 (jint)y,
                                                 (jint)coord_type);

  JawImpl* jaw_impl = jaw_impl_get_instance( jniEnv, child_ac );

  g_object_ref( G_OBJECT(jaw_impl) );

  return ATK_OBJECT(jaw_impl);
}

static gboolean
jaw_component_set_extents (AtkComponent *component,
                           gint         *x,
                           gint         *y,
                           gint         *width,
                           gint         *height,
                           AtkCoordType coord_type)
{

  if (x == NULL || y == NULL || width == NULL || height == NULL)
  {
    return FALSE;
  }
  
  JawObject *jaw_obj = JAW_OBJECT(component);
  ComponentData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_COMPONENT);
  jobject atk_component = data->atk_component;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classAtkComponent = (*jniEnv)->FindClass(jniEnv,
                                                  "org/GNOME/Accessibility/AtkComponent");

  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkComponent,
                                          "set_extents",
                                          "()Ljava/awt/Rectangle;");

  jobject jrectangle = (*jniEnv)->CallObjectMethod(jniEnv, atk_component, jmid);

  if (jrectangle == NULL)
  {
    (*width) = 0;
    (*height) = 0;
    (*x) = 0;
    (*y) = 0;
    return FALSE;
  }

  jclass rectangle_class = (*jniEnv)->FindClass(jniEnv, "java/awt/Rectangle");
  jfieldID jfidWidth = (*jniEnv)->GetFieldID(jniEnv,
                                             rectangle_class,
                                             "width",
                                             "I");
  jfieldID jfidHeight = (*jniEnv)->GetFieldID(jniEnv,
                                              rectangle_class,
                                              "height",
                                              "I");
  jfieldID jfidX = (*jniEnv)->GetFieldID(jniEnv,
                                         rectangle_class,
                                         "x",
                                         "I");

  jfieldID jfidY = (*jniEnv)->GetFieldID(jniEnv,
                                         rectangle_class,
                                         "y",
                                         "I");

  jint jwidth = (*jniEnv)->GetIntField(jniEnv, rectangle_class, jfidWidth);
  jint jheight = (*jniEnv)->GetIntField(jniEnv, rectangle_class, jfidHeight);
  jint jx = (*jniEnv)->GetIntField(jniEnv, rectangle_class, jfidX);
  jint jy = (*jniEnv)->GetIntField(jniEnv, rectangle_class, jfidY);

  (*width) = (gint)jwidth;
  (*height) = (gint)jheight;
  (*x) = (gint)jx;
  (*y) = (gint)jy;

  return TRUE;
}

static void
jaw_component_get_extents (AtkComponent *component,
                           gint         *x,
                           gint         *y,
                           gint         *width,
                           gint         *height,
                           AtkCoordType coord_type)
{
  if (x == NULL || y == NULL || width == NULL || height == NULL)
  {
    return;
  }
  
  JawObject *jaw_obj = JAW_OBJECT(component);
  ComponentData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_COMPONENT);
  jobject atk_component = data->atk_component;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classAtkComponent = (*jniEnv)->FindClass(jniEnv,
                                                  "org/GNOME/Accessibility/AtkComponent");

  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkComponent,
                                          "get_extents",
                                          "()Ljava/awt/Rectangle;");

  jobject jrectangle = (*jniEnv)->CallObjectMethod(jniEnv, atk_component, jmid);

  if (jrectangle == NULL)
  {
    (*width) = 0;
    (*height) = 0;
    (*x) = 0;
    (*y) = 0;
    return;
  }
  return jaw_component_get_extents(component, x, y, width, height, coord_type);
}

static gboolean
jaw_component_grab_focus (AtkComponent *component)
{
  JawObject *jaw_obj = JAW_OBJECT(component);
  ComponentData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_COMPONENT);
  jobject atk_component = data->atk_component;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classAtkComponent = (*jniEnv)->FindClass(jniEnv,
                                                  "org/GNOME/Accessibility/AtkComponent");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkComponent,
                                          "grab_focus",
                                          "()Z");
  jboolean jresult = (*jniEnv)->CallBooleanMethod(jniEnv, atk_component, jmid);

  if (jresult == JNI_TRUE)
  {
    return TRUE;
  }

  return FALSE;
}

static AtkLayer
jaw_component_get_layer (AtkComponent *component)
{
  JawObject *jaw_obj = JAW_OBJECT(component);
  ComponentData *data = jaw_object_get_interface_data(jaw_obj,
                                                      INTERFACE_COMPONENT);
  jobject atk_component = data->atk_component;

  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classAtkComponent = (*jniEnv)->FindClass(jniEnv,
                                                  "org/GNOME/Accessibility/AtkComponent");
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAtkComponent,
                                          "get_layer",
                                          "()I");

  jint jlayer = (*jniEnv)->CallIntMethod(jniEnv, atk_component, jmid);

  return (AtkLayer)jlayer;
}

