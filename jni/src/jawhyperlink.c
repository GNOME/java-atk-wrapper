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

#include "jawhyperlink.h"
#include "jawimpl.h"
#include "jawutil.h"
#include <glib.h>

/**
 * (From Atk documentation)
 *
 * AtkHyperlink:
 *
 * An ATK object which encapsulates a link or set of links in a hypertext
 * document.
 *
 * An ATK object which encapsulates a link or set of links (for
 * instance in the case of client-side image maps) in a hypertext
 * document.  It may implement the AtkAction interface.  AtkHyperlink
 * may also be used to refer to inline embedded content, since it
 * allows specification of a start and end offset within the host
 * AtkHypertext object.
 */

static void jaw_hyperlink_dispose (GObject *gobject);
static void jaw_hyperlink_finalize (GObject *gobject);

/* AtkObject */
static gchar *jaw_hyperlink_get_uri (AtkHyperlink *atk_hyperlink,
                                     gint i);
static AtkObject *jaw_hyperlink_get_object (AtkHyperlink *atk_hyperlink,
                                            gint i);
static gint jaw_hyperlink_get_end_index (AtkHyperlink *atk_hyperlink);
static gint jaw_hyperlink_get_start_index (AtkHyperlink *atk_hyperlink);
static gboolean jaw_hyperlink_is_valid (AtkHyperlink *atk_hyperlink);
static gint jaw_hyperlink_get_n_anchors (AtkHyperlink *atk_hyperlink);

G_DEFINE_TYPE (JawHyperlink, jaw_hyperlink, ATK_TYPE_HYPERLINK)

#define JAW_GET_HYPERLINK(atk_hyperlink, def_ret) \
  JAW_GET_OBJ (atk_hyperlink, JAW_HYPERLINK, JawHyperlink, jaw_hyperlink, jhyperlink, jniEnv, jhyperlink, def_ret)

JawHyperlink *
jaw_hyperlink_new (jobject jhyperlink)
{
  JAW_DEBUG_ALL ("%p", jhyperlink);
  JawHyperlink *jaw_hyperlink = g_object_new (JAW_TYPE_HYPERLINK, NULL);
  JNIEnv *jniEnv = jaw_util_get_jni_env ();
  jaw_hyperlink->jhyperlink = (*jniEnv)->NewGlobalRef (jniEnv, jhyperlink);

  return jaw_hyperlink;
}

/**
 * _AtkHyperlinkClass:
 * @get_uri:
 * @get_object:
 * @get_end_index:
 * @get_start_index:
 * @is_valid:
 * @get_n_anchors:
 * @link_state:
 * @is_selected_link:
 * @link_activated -- The signal link-activated is emitted when a link is
 *activated.
 **/

static void
jaw_hyperlink_class_init (JawHyperlinkClass *klass)
{
  JAW_DEBUG_ALL ("%p", klass);
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->dispose = jaw_hyperlink_dispose;
  gobject_class->finalize = jaw_hyperlink_finalize;

  AtkHyperlinkClass *atk_hyperlink_class = ATK_HYPERLINK_CLASS (klass);
  atk_hyperlink_class->get_uri = jaw_hyperlink_get_uri;
  atk_hyperlink_class->get_object = jaw_hyperlink_get_object;
  atk_hyperlink_class->get_end_index = jaw_hyperlink_get_end_index;
  atk_hyperlink_class->get_start_index = jaw_hyperlink_get_start_index;
  atk_hyperlink_class->is_valid = jaw_hyperlink_is_valid;
  atk_hyperlink_class->get_n_anchors = jaw_hyperlink_get_n_anchors;
  // TODO: missing java support for atk_hyperlink_class->link_state
  // TODO: missing java support for atk_hyperlink_class->is_selected_link
}

static void
jaw_hyperlink_init (JawHyperlink *link)
{
  JAW_DEBUG_ALL ("%p", link);
}

static void
jaw_hyperlink_dispose (GObject *gobject)
{
  JAW_DEBUG_ALL ("%p", gobject);
  /* Chain up to parent's dispose */
  G_OBJECT_CLASS (jaw_hyperlink_parent_class)->dispose (gobject);
}

static void
jaw_hyperlink_finalize (GObject *gobject)
{
  JAW_DEBUG_ALL ("%p", gobject);
  JawHyperlink *jaw_hyperlink = JAW_HYPERLINK (gobject);

  JNIEnv *jniEnv = jaw_util_get_jni_env ();
  (*jniEnv)->DeleteGlobalRef (jniEnv, jaw_hyperlink->jhyperlink);
  jaw_hyperlink->jhyperlink = NULL;

  /* Chain up to parent's finalize */
  G_OBJECT_CLASS (jaw_hyperlink_parent_class)->finalize (gobject);
}

/**
 * jaw_hyperlink_get_uri:
 * @atk_hyperlink: an #AtkHyperlink
 * @i: a (zero-index) integer specifying the desired anchor
 *
 * Get the URI associated with the anchor specified
 * by @i of @atk_hyperlink.
 *
 * Multiple anchors are primarily used by client-side image maps.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * Returns: a string specifying the URI.
 * The caller of the method takes ownership of the returned data, and is
 * responsible for freeing it
 **/

static gchar *
jaw_hyperlink_get_uri (AtkHyperlink *atk_hyperlink,
                       gint i)
{
  JAW_DEBUG_C ("%p, %d", atk_hyperlink, i);
  JAW_GET_HYPERLINK (atk_hyperlink, NULL);

  jclass classAtkHyperlink = (*jniEnv)->FindClass (jniEnv, "org/GNOME/Accessibility/AtkHyperlink");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv, classAtkHyperlink, "get_uri", "(I)Ljava/lang/String;");
  jstring jstr = (*jniEnv)->CallObjectMethod (jniEnv, jhyperlink, jmid, (jint) i);
  (*jniEnv)->DeleteGlobalRef (jniEnv, jhyperlink);

  if (jaw_hyperlink->uri != NULL)
    {
      (*jniEnv)->ReleaseStringUTFChars (jniEnv, jaw_hyperlink->jstrUri, jaw_hyperlink->uri);
      (*jniEnv)->DeleteGlobalRef (jniEnv, jaw_hyperlink->jstrUri);
    }

  jaw_hyperlink->jstrUri = (*jniEnv)->NewGlobalRef (jniEnv, jstr);
  jaw_hyperlink->uri = (gchar *) (*jniEnv)->GetStringUTFChars (jniEnv, jaw_hyperlink->jstrUri, NULL);

  return jaw_hyperlink->uri;
}

/**
 * jaw_hyperlink_get_object:
 * @atk_hyperlink: an #AtkHyperlink
 * @i: a (zero-index) integer specifying the desired anchor
 *
 * Returns the item associated with this hyperlinks nth anchor.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * Returns: (transfer none): an #AtkObject associated with this hyperlinks
 * i-th anchor
 **/

static AtkObject *
jaw_hyperlink_get_object (AtkHyperlink *atk_hyperlink,
                          gint i)
{
  JAW_DEBUG_C ("%p, %d", atk_hyperlink, i);
  JAW_GET_HYPERLINK (atk_hyperlink, NULL);

  jclass classAtkHyperlink = (*jniEnv)->FindClass (jniEnv, "org/GNOME/Accessibility/AtkHyperlink");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv, classAtkHyperlink, "get_object", "(I)Ljavax/accessibility/AccessibleContext;");
  jobject ac = (*jniEnv)->CallObjectMethod (jniEnv, jhyperlink, jmid, (jint) i);
  (*jniEnv)->DeleteGlobalRef (jniEnv, jhyperlink);
  if (ac == NULL)
    {
      return NULL;
    }

  AtkObject *obj = (AtkObject *) jaw_impl_get_instance_from_jaw (jniEnv, ac);
  /* get_object returns with transfer: none */

  return obj;
}

/**
 * atk_hyperlink_get_end_index:
 * @atk_hyperlink: an #AtkHyperlink
 *
 * Gets the index with the hypertext document at which this link ends.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * Returns: the index with the hypertext document at which this link ends, 0 if
 *an error happened.
 **/

static gint
jaw_hyperlink_get_end_index (AtkHyperlink *atk_hyperlink)
{
  JAW_DEBUG_C ("%p", atk_hyperlink);
  JAW_GET_HYPERLINK (atk_hyperlink, 0);

  jclass classAtkHyperlink = (*jniEnv)->FindClass (jniEnv, "org/GNOME/Accessibility/AtkHyperlink");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv, classAtkHyperlink, "get_end_index", "()I");
  jint jindex = (*jniEnv)->CallIntMethod (jniEnv, jhyperlink, jmid);
  (*jniEnv)->DeleteGlobalRef (jniEnv, jhyperlink);

  return jindex;
}

/**
 * jaw_hyperlink_get_start_index:
 * @atk_hyperlink: an #AtkHyperlink
 *
 * Gets the index with the hypertext document at which this link begins.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * Returns: the index with the hypertext document at which this link begins, 0
 * if an error happened
 **/

static gint
jaw_hyperlink_get_start_index (AtkHyperlink *atk_hyperlink)
{
  JAW_DEBUG_C ("%p", atk_hyperlink);
  JAW_GET_HYPERLINK (atk_hyperlink, 0);

  jclass classAtkHyperlink = (*jniEnv)->FindClass (jniEnv, "org/GNOME/Accessibility/AtkHyperlink");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv, classAtkHyperlink, "get_start_index", "()I");
  jint jindex = (*jniEnv)->CallIntMethod (jniEnv, jhyperlink, jmid);
  (*jniEnv)->DeleteGlobalRef (jniEnv, jhyperlink);

  return jindex;
}

/**
 * jaw_hyperlink_is_valid:
 * @atk_hyperlink: an #AtkHyperlink
 *
 * Since the document that a link is associated with may have changed
 * this method returns %TRUE if the link is still valid (with
 * respect to the document it references) and %FALSE otherwise.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * Returns: whether or not this link is still valid
 **/

static gboolean
jaw_hyperlink_is_valid (AtkHyperlink *atk_hyperlink)
{
  JAW_DEBUG_C ("%p", atk_hyperlink);
  JAW_GET_HYPERLINK (atk_hyperlink, FALSE);

  jclass classAtkHyperlink = (*jniEnv)->FindClass (jniEnv, "org/GNOME/Accessibility/AtkHyperlink");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv, classAtkHyperlink, "is_valid", "()Z");
  jboolean jvalid = (*jniEnv)->CallBooleanMethod (jniEnv, jhyperlink, jmid);
  (*jniEnv)->DeleteGlobalRef (jniEnv, jhyperlink);

  return jvalid;
}

/**
 * jaw_hyperlink_get_n_anchors:
 * @atk_hyperlink: an #AtkHyperlink
 *
 * Gets the number of anchors associated with this hyperlink.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * Returns: the number of anchors associated with this hyperlink
 **/

static gint
jaw_hyperlink_get_n_anchors (AtkHyperlink *atk_hyperlink)
{
  JAW_DEBUG_C ("%p", atk_hyperlink);
  JAW_GET_HYPERLINK (atk_hyperlink, 0);

  jclass classAtkHyperlink = (*jniEnv)->FindClass (jniEnv, "org/GNOME/Accessibility/AtkHyperlink");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv, classAtkHyperlink, "get_n_anchors", "()I");
  jint janchors = (*jniEnv)->CallIntMethod (jniEnv, jhyperlink, jmid);
  (*jniEnv)->DeleteGlobalRef (jniEnv, jhyperlink);

  return janchors;
}
