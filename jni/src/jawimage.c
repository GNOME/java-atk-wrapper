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
 * AtkImage:
 *
 * The ATK Interface implemented by components
 *  which expose image or pixmap content on-screen.
 *
 * #AtkImage should be implemented by #AtkObject subtypes on behalf of
 * components which display image/pixmap information onscreen, and
 * which provide information (other than just widget borders, etc.)
 * via that image content.  For instance, icons, buttons with icons,
 * toolbar elements, and image viewing panes typically should
 * implement #AtkImage.
 *
 * #AtkImage primarily provides two types of information: coordinate
 * information (useful for screen review mode of screenreaders, and
 * for use by onscreen magnifiers), and descriptive information.  The
 * descriptive information is provided for alternative, text-only
 * presentation of the most significant information present in the
 * image.
 */

static void jaw_image_get_image_position (AtkImage *image,
                                          gint *x,
                                          gint *y,
                                          AtkCoordType coord_type);
static const gchar *jaw_image_get_image_description (AtkImage *image);
static void jaw_image_get_image_size (AtkImage *image,
                                      gint *width,
                                      gint *height);

typedef struct _ImageData
{
  jobject atk_image;
  gchar *image_description;
  jstring jstrImageDescription;
} ImageData;

#define JAW_GET_IMAGE(image, def_ret) \
  JAW_GET_OBJ_IFACE (image, INTERFACE_IMAGE, ImageData, atk_image, jniEnv, atk_image, def_ret)

/**
 * AtkImageIface:
 * @get_image_position:
 * @get_image_description:
 * @get_image_size
 * @set_image_description:
 * @get_image_locale:
 **/

void
jaw_image_interface_init (AtkImageIface *iface, gpointer data)
{
  JAW_DEBUG_ALL ("%p, %p", iface, data);
  iface->get_image_position = jaw_image_get_image_position;
  iface->get_image_description = jaw_image_get_image_description;
  iface->get_image_size = jaw_image_get_image_size;
  iface->set_image_description = NULL; /* TODO */
                                       // TODO: iface->get_image_locale from AccessibleContext.getLocale()
}

/**
 * jaw_image_data_init:
 * @ac: a Java AccessibleContext object
 *
 * Initializes the image interface data for an accessible object.
 * Creates and returns an ImageData structure containing a global reference
 * to the Java AtkImage object.
 *
 * Explicitly manages a JNI local reference frame using
 * PushLocalFrame/PopLocalFrame; all local references are released
 * before the function returns.
 *
 * Returns: (nullable): pointer to ImageData or NULL on failure
 **/

gpointer
jaw_image_data_init (jobject ac)
{
  JAW_DEBUG_C ("%p", ac);
  ImageData *data = g_new0 (ImageData, 1);

  JNIEnv *jniEnv = jaw_util_get_jni_env ();
  jclass classImage = (*jniEnv)->FindClass (jniEnv, "org/GNOME/Accessibility/AtkImage");
  jmethodID jmid = (*jniEnv)->GetStaticMethodID (jniEnv, classImage, "createAtkImage", "(Ljavax/accessibility/AccessibleContext;)Lorg/GNOME/Accessibility/AtkImage;");
  jobject jatk_image = (*jniEnv)->CallStaticObjectMethod (jniEnv, classImage, jmid, ac);
  data->atk_image = (*jniEnv)->NewGlobalRef (jniEnv, jatk_image);

  return data;
}

/**
 * jaw_image_data_finalize:
 * @p: ImageData pointer to finalize
 *
 * Cleans up ImageData when the parent GObject is finalized.
 * Called from jaw_impl_finalize() when the object's reference count reaches
 * zero.
 */

void
jaw_image_data_finalize (gpointer p)
{
  JAW_DEBUG_ALL ("%p", p);
  ImageData *data = (ImageData *) p;
  JNIEnv *jniEnv = jaw_util_get_jni_env ();

  if (data && data->atk_image)
    {
      if (data->image_description != NULL)
        {
          (*jniEnv)->ReleaseStringUTFChars (jniEnv, data->jstrImageDescription, data->image_description);
          (*jniEnv)->DeleteGlobalRef (jniEnv, data->jstrImageDescription);
          data->jstrImageDescription = NULL;
          data->image_description = NULL;
        }

      (*jniEnv)->DeleteGlobalRef (jniEnv, data->atk_image);
      data->atk_image = NULL;
    }
}

/**
 * jaw_image_get_image_position:
 * @image: a #GObject instance that implements AtkImageIface
 * @x: (out) (optional): address of #gint to put x coordinate position;
 *otherwise, -1 if value cannot be obtained.
 * @y: (out) (optional): address of #gint to put y coordinate position;
 *otherwise, -1 if value cannot be obtained.
 * @coord_type: specifies whether the coordinates are relative to the screen
 * or to the components top level window
 *
 * Gets the position of the image in the form of a point specifying the
 * images top-left corner.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * If the position can not be obtained (e.g. missing support), x and y are set
 * to -1.
 **/

static void
jaw_image_get_image_position (AtkImage *image,
                              gint *x,
                              gint *y,
                              AtkCoordType coord_type)
{
  JAW_DEBUG_C ("%p, %p, %p, %d", image, x, y, coord_type);
  (*x) = -1;
  (*y) = -1;
  JAW_GET_IMAGE (image, );

  jclass classAtkImage = (*jniEnv)->FindClass (jniEnv, "org/GNOME/Accessibility/AtkImage");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv, classAtkImage, "get_image_position", "(I)Ljava/awt/Point;");
  jobject jpoint = (*jniEnv)->CallObjectMethod (jniEnv, atk_image, jmid, (jint) coord_type);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_image);

  if (jpoint == NULL)
    {
      JAW_DEBUG_I ("jpoint == NULL");
      return;
    }

  jclass classPoint = (*jniEnv)->FindClass (jniEnv, "java/awt/Point");
  jfieldID jfidX = (*jniEnv)->GetFieldID (jniEnv, classPoint, "x", "I");
  jfieldID jfidY = (*jniEnv)->GetFieldID (jniEnv, classPoint, "y", "I");
  jint jx = (*jniEnv)->GetIntField (jniEnv, jpoint, jfidX);
  jint jy = (*jniEnv)->GetIntField (jniEnv, jpoint, jfidY);

  (*x) = (gint) jx;
  (*y) = (gint) jy;
}

/**
 * jaw_image_get_image_description:
 * @image: a #GObject instance that implements AtkImageIface
 *
 * Get a textual description of this image.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * Returns: a string representing the image description or NULL
 **/

static const gchar *
jaw_image_get_image_description (AtkImage *image)
{
  JAW_DEBUG_C ("%p", image);
  JAW_GET_IMAGE (image, NULL);

  jclass classAtkImage = (*jniEnv)->FindClass (jniEnv, "org/GNOME/Accessibility/AtkImage");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv, classAtkImage, "get_image_description", "()Ljava/lang/String;");
  jstring jstr = (*jniEnv)->CallObjectMethod (jniEnv, atk_image, jmid);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_image);

  if (data->image_description != NULL)
    {
      (*jniEnv)->ReleaseStringUTFChars (jniEnv, data->jstrImageDescription, data->image_description);
      (*jniEnv)->DeleteGlobalRef (jniEnv, data->jstrImageDescription);
    }

  data->jstrImageDescription = (*jniEnv)->NewGlobalRef (jniEnv, jstr);
  data->image_description = (gchar *) (*jniEnv)->GetStringUTFChars (jniEnv, data->jstrImageDescription, NULL);

  return data->image_description;
}

/**
 * jaw_image_get_image_size:
 * @image: a #GObject instance that implements AtkImageIface
 * @width: (out) (optional): filled with the image width, or -1 if the value
 *cannot be obtained.
 * @height: (out) (optional): filled with the image height, or -1 if the value
 *cannot be obtained.
 *
 * Get the width and height in pixels for the specified image.
 * The values of @width and @height are returned as -1 if the
 * values cannot be obtained (for instance, if the object is not onscreen).
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * If the size can not be obtained (e.g. missing support), x and y are set
 * to -1.
 **/

static void
jaw_image_get_image_size (AtkImage *image, gint *width, gint *height)
{
  JAW_DEBUG_C ("%p, %p, %p", image, width, height);
  (*width) = -1;
  (*height) = -1;
  JAW_GET_IMAGE (image, );

  jclass classAtkImage = (*jniEnv)->FindClass (jniEnv, "org/GNOME/Accessibility/AtkImage");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv, classAtkImage, "get_image_size", "()Ljava/awt/Dimension;");
  jobject jdimension = (*jniEnv)->CallObjectMethod (jniEnv, atk_image, jmid);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_image);

  if (jdimension == NULL)
    {
      JAW_DEBUG_I ("jdimension == NULL");
      return;
    }

  jclass classDimension = (*jniEnv)->FindClass (jniEnv, "java/awt/Dimension");
  jfieldID jfidWidth = (*jniEnv)->GetFieldID (jniEnv, classDimension, "width", "I");
  jfieldID jfidHeight = (*jniEnv)->GetFieldID (jniEnv, classDimension, "height", "I");
  jint jwidth = (*jniEnv)->GetIntField (jniEnv, jdimension, jfidWidth);
  jint jheight = (*jniEnv)->GetIntField (jniEnv, jdimension, jfidHeight);

  (*width) = (gint) jwidth;
  (*height) = (gint) jheight;
}
