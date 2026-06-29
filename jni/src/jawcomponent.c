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

#include "jawimpl.h"
#include "jawutil.h"
#include <atk/atk.h>
#include <glib-object.h>
#include <glib.h>

/**
 * (From Atk documentation)
 *
 * AtkComponent:
 *
 * The ATK interface provided by UI components
 * which occupy a physical area on the screen.
 * which the user can activate/interact with.
 *
 * #AtkComponent should be implemented by most if not all UI elements
 * with an actual on-screen presence, i.e. components which can be
 * said to have a screen-coordinate bounding box.  Virtually all
 * widgets will need to have #AtkComponent implementations provided
 * for their corresponding #AtkObject class.  In short, only UI
 * elements which are *not* GUI elements will omit this ATK interface.
 *
 * A possible exception might be textual information with a
 * transparent background, in which case text glyph bounding box
 * information is provided by #AtkText.
 */

static gboolean jaw_component_contains (AtkComponent *component,
                                        gint x,
                                        gint y,
                                        AtkCoordType coord_type);

static AtkObject *jaw_component_ref_accessible_at_point (AtkComponent *component,
                                                         gint x,
                                                         gint y,
                                                         AtkCoordType coord_type);

static void jaw_component_get_extents (AtkComponent *component,
                                       gint *x,
                                       gint *y,
                                       gint *width,
                                       gint *height,
                                       AtkCoordType coord_type);

static gboolean jaw_component_set_extents (AtkComponent *component,
                                           gint x,
                                           gint y,
                                           gint width,
                                           gint height,
                                           AtkCoordType coord_type);

static gboolean jaw_component_grab_focus (AtkComponent *component);
static AtkLayer jaw_component_get_layer (AtkComponent *component);
/*static gin jaw_component_get_mdi_zorder(AtkComponent		*component); */

typedef struct _ComponentData
{
  jobject atk_component;
} ComponentData;

#define JAW_GET_COMPONENT(component, def_ret) \
  JAW_GET_OBJ_IFACE (component, INTERFACE_COMPONENT, ComponentData, atk_component, jniEnv, atk_component, def_ret)

void
jaw_component_interface_init (AtkComponentIface *iface, gpointer data)
{
  JAW_DEBUG_ALL ("%p,%p", iface, data);
  // deprecated: iface->add_focus_handler
  iface->contains = jaw_component_contains;
  iface->ref_accessible_at_point = jaw_component_ref_accessible_at_point;
  iface->get_extents = jaw_component_get_extents;
  // done by atk: iface->get_position
  // done by atk: iface->get_size
  iface->grab_focus = jaw_component_grab_focus;
  // deprecated: iface->remove_focus_handler
  iface->set_extents = jaw_component_set_extents;
  // TODO: iface->set_position similar to set_extents
  // TODO: iface->set_size similar to set_extents
  iface->get_layer = jaw_component_get_layer;
  iface->get_mdi_zorder = NULL; /* TODO: jaw_component_get_mdi_zorder;*/
  // TODO: missing java support for iface->get_alpha
  // TODO: missing java support for iface->scroll_to
  // TODO: missing java support for iface->scroll_to_point
}

/**
 * jaw_component_data_init:
 * @ac: a Java AccessibleContext object
 *
 * Initializes component interface data for an AccessibleContext.
 *
 * Explicitly manages a JNI local reference frame using
 * PushLocalFrame/PopLocalFrame; all local references are released
 * before the function returns.
 *
 * Returns: (transfer full): ComponentData pointer, or %NULL on error
 */

gpointer
jaw_component_data_init (jobject ac)
{
  JAW_DEBUG_ALL ("%p", ac);
  ComponentData *data = g_new0 (ComponentData, 1);

  JNIEnv *jniEnv = jaw_util_get_jni_env ();
  jclass classComponent = (*jniEnv)->FindClass (jniEnv,
                                                "org/GNOME/Accessibility/AtkComponent");
  jmethodID jmid = (*jniEnv)->GetStaticMethodID (jniEnv,
                                                 classComponent,
                                                 "create_atk_component",
                                                 "(Ljavax/accessibility/AccessibleContext;)Lorg/GNOME/Accessibility/AtkComponent;");

  jobject jatk_component = (*jniEnv)->CallStaticObjectMethod (jniEnv, classComponent, jmid, ac);
  data->atk_component = (*jniEnv)->NewGlobalRef (jniEnv, jatk_component);

  return data;
}

/**
 * jaw_component_data_finalize:
 * @p: ComponentData pointer to finalize
 *
 * Cleans up ComponentData when the parent GObject is finalized.
 * Called from jaw_impl_finalize() when the object's reference count reaches
 * zero.
 */

void
jaw_component_data_finalize (gpointer p)
{
  JAW_DEBUG_ALL ("%p", p);
  ComponentData *data = (ComponentData *) p;
  JNIEnv *jniEnv = jaw_util_get_jni_env ();

  if (data && data->atk_component)
    {
      (*jniEnv)->DeleteGlobalRef (jniEnv, data->atk_component);
      data->atk_component = NULL;
    }
}

/**
 * jaw_component_contains:
 * @component: the #AtkComponent
 * @x: x coordinate
 * @y: y coordinate
 * @coord_type: specifies whether the coordinates are relative to the screen
 * or to the components top level window
 *
 * Checks whether the specified point is within the extent of the @component.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * Returns: %TRUE or %FALSE indicating whether the specified point is within
 * the extent of the @component or not
 **/

static gboolean
jaw_component_contains (AtkComponent *component, gint x, gint y, AtkCoordType coord_type)
{
  JAW_DEBUG_C ("%p, %d, %d, %d", component, x, y, coord_type);
  JAW_GET_COMPONENT (component, FALSE);

  jclass classAtkComponent = (*jniEnv)->FindClass (jniEnv,
                                                   "org/GNOME/Accessibility/AtkComponent");

  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkComponent,
                                           "contains",
                                           "(III)Z");

  jboolean jcontains = (*jniEnv)->CallBooleanMethod (jniEnv,
                                                     atk_component,
                                                     jmid,
                                                     (jint) x,
                                                     (jint) y,
                                                     (jint) coord_type);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_component);

  return jcontains;
}

/**
 * jaw_component_ref_accessible_at_point:
 * @component: the #AtkComponent
 * @x: x coordinate
 * @y: y coordinate
 * @coord_type: specifies whether the coordinates are relative to the screen
 * or to the components top level window
 *
 * Gets a reference to the accessible child, if one exists, at the
 * coordinate point specified by @x and @y.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * Returns: (nullable) (transfer full): a reference to the accessible
 * child, if one exists
 **/

static AtkObject *
jaw_component_ref_accessible_at_point (AtkComponent *component, gint x, gint y, AtkCoordType coord_type)
{
  JAW_DEBUG_C ("%p, %d, %d, %d", component, x, y, coord_type);
  JAW_GET_COMPONENT (component, NULL);

  jclass classAtkComponent = (*jniEnv)->FindClass (jniEnv,
                                                   "org/GNOME/Accessibility/AtkComponent");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkComponent,
                                           "get_accessible_at_point",
                                           "(III)Ljavax/accessibility/AccessibleContext;");
  jobject child_ac = (*jniEnv)->CallObjectMethod (jniEnv,
                                                  atk_component,
                                                  jmid,
                                                  (jint) x,
                                                  (jint) y,
                                                  (jint) coord_type);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_component);

  JawImpl *jaw_impl = jaw_impl_get_instance_from_jaw (jniEnv, child_ac);

  if (jaw_impl)
    g_object_ref (G_OBJECT (jaw_impl));

  return ATK_OBJECT (jaw_impl);
}

/**
 * jaw_component_get_extents:
 * @component: an #AtkComponent
 * @x: (out) (optional): address of #gint to put x coordinate
 * @y: (out) (optional): address of #gint to put y coordinate
 * @width: (out) (optional): address of #gint to put width
 * @height: (out) (optional): address of #gint to put height
 * @coord_type: specifies whether the coordinates are relative to the screen
 * or to the components top level window
 *
 * Gets the rectangle which gives the extent of the @component.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * If the extent can not be obtained (e.g. a non-embedded plug or missing
 * support), all of x, y, width, height are set to -1.
 *
 **/

static void
jaw_component_get_extents (AtkComponent *component,
                           gint *x,
                           gint *y,
                           gint *width,
                           gint *height,
                           AtkCoordType coord_type)
{
  JAW_DEBUG_C ("%p, %p, %p, %p, %p, %d", component, x, y, width, height, coord_type);
  if (x == NULL || y == NULL || width == NULL || height == NULL)
    return;

  (*x) = -1;
  (*y) = -1;
  (*width) = -1;
  (*height) = -1;

  if (component == NULL)
    return;

  JAW_GET_COMPONENT (component, );

  jclass classAtkComponent = (*jniEnv)->FindClass (jniEnv,
                                                   "org/GNOME/Accessibility/AtkComponent");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkComponent,
                                           "get_extents",
                                           "(I)Ljava/awt/Rectangle;");

  jobject jrectangle = (*jniEnv)->CallObjectMethod (jniEnv, atk_component, jmid, (jint) coord_type);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_component);

  if (jrectangle == NULL)
    {
      JAW_DEBUG_I ("jrectangle == NULL");
      return;
    }

  jclass classRectangle = (*jniEnv)->FindClass (jniEnv, "java/awt/Rectangle");
  jfieldID jfidX = (*jniEnv)->GetFieldID (jniEnv, classRectangle, "x", "I");
  jfieldID jfidY = (*jniEnv)->GetFieldID (jniEnv, classRectangle, "y", "I");
  jfieldID jfidW = (*jniEnv)->GetFieldID (jniEnv, classRectangle, "width", "I");
  jfieldID jfidH = (*jniEnv)->GetFieldID (jniEnv, classRectangle, "height", "I");
  (*x) = (gint) (*jniEnv)->GetIntField (jniEnv, jrectangle, jfidX);
  (*y) = (gint) (*jniEnv)->GetIntField (jniEnv, jrectangle, jfidY);
  (*width) = (gint) (*jniEnv)->GetIntField (jniEnv, jrectangle, jfidW);
  (*height) = (gint) (*jniEnv)->GetIntField (jniEnv, jrectangle, jfidH);
}

/**
 * jaw_component_set_extents:
 * @component: an #AtkComponent
 * @x: x coordinate
 * @y: y coordinate
 * @width: width to set for @component
 * @height: height to set for @component
 * @coord_type: specifies whether the coordinates are relative to the screen
 * or to the components top level window
 *
 * Sets the extents of @component.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * Returns: %TRUE or %FALSE whether the extents were set or not
 **/

static gboolean
jaw_component_set_extents (AtkComponent *component,
                           gint x,
                           gint y,
                           gint width,
                           gint height,
                           AtkCoordType coord_type)
{
  JAW_DEBUG_C ("%p, %d, %d, %d, %d, %d", component, x, y, width, height, coord_type);
  JAW_GET_COMPONENT (component, FALSE);

  jclass classAtkComponent = (*jniEnv)->FindClass (jniEnv, "org/GNOME/Accessibility/AtkComponent");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv, classAtkComponent, "set_extents", "(IIIII)Z");
  jboolean assigned = (*jniEnv)->CallBooleanMethod (jniEnv, atk_component, jmid, (jint) x, (jint) y, (jint) width, (jint) height, (jint) coord_type);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_component);
  return assigned;
}

/**
 * jaw_component_grab_focus:
 * @component: an #AtkComponent
 *
 * Grabs focus for this @component.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * Returns: %TRUE if successful, %FALSE otherwise.
 **/

static gboolean
jaw_component_grab_focus (AtkComponent *component)
{
  JAW_DEBUG_C ("%p", component);
  JAW_GET_COMPONENT (component, FALSE);

  jclass classAtkComponent = (*jniEnv)->FindClass (jniEnv,
                                                   "org/GNOME/Accessibility/AtkComponent");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkComponent,
                                           "grab_focus",
                                           "()Z");
  jboolean jresult = (*jniEnv)->CallBooleanMethod (jniEnv, atk_component, jmid);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_component);
  return jresult;
}

/**
 * jaw_component_get_layer:
 * @component: an #AtkComponent
 *
 * Gets the layer of the component.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * Returns: an #AtkLayer which is the layer of the component, ATK_LAYER_INVALID
 * if an error occurred.
 **/

static AtkLayer
jaw_component_get_layer (AtkComponent *component)
{
  JAW_DEBUG_C ("%p", component);
  JAW_GET_COMPONENT (component, 0);

  jclass classAtkComponent = (*jniEnv)->FindClass (jniEnv,
                                                   "org/GNOME/Accessibility/AtkComponent");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv,
                                           classAtkComponent,
                                           "get_layer",
                                           "()I");

  jint jlayer = (*jniEnv)->CallIntMethod (jniEnv, atk_component, jmid);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_component);

  return (AtkLayer) jlayer;
}
