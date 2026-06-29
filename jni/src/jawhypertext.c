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
#include <atk/atk.h>
#include <glib.h>

/**
 * (From Atk documentation)
 *
 * AtkHypertext:
 *
 * The ATK interface which provides standard mechanism for manipulating
 * hyperlinks.
 *
 * An interface used for objects which implement linking between
 * multiple resource or content locations, or multiple 'markers'
 * within a single document.  A Hypertext instance is associated with
 * one or more Hyperlinks, which are associated with particular
 * offsets within the Hypertext's included content.  While this
 * interface is derived from Text, there is no requirement that
 * Hypertext instances have textual content; they may implement Image
 * as well, and Hyperlinks need not have non-zero text offsets.
 */

static AtkHyperlink *jaw_hypertext_get_link (AtkHypertext *hypertext,
                                             gint link_index);
static gint jaw_hypertext_get_n_links (AtkHypertext *hypertext);
static gint jaw_hypertext_get_link_index (AtkHypertext *hypertext,
                                          gint char_index);

typedef struct _HypertextData
{
  jobject atk_hypertext;
  GHashTable *link_table;
} HypertextData;

#define JAW_GET_HYPERTEXT(hypertext, def_ret) \
  JAW_GET_OBJ_IFACE (hypertext, INTERFACE_HYPERTEXT, HypertextData, atk_hypertext, jniEnv, atk_hypertext, def_ret)

/**
 * AtkHypertextIface:
 * @get_link:
 * @get_n_links:
 * @get_link_index:
 **/

void
jaw_hypertext_interface_init (AtkHypertextIface *iface, gpointer data)
{
  iface->get_link = jaw_hypertext_get_link;
  iface->get_n_links = jaw_hypertext_get_n_links;
  iface->get_link_index = jaw_hypertext_get_link_index;
}

static void
link_destroy_notify (gpointer p)
{
  JAW_DEBUG_C ("%p", p);
  JawHyperlink *jaw_hyperlink = (JawHyperlink *) p;
  if (G_OBJECT (jaw_hyperlink) != NULL)
    g_object_unref (G_OBJECT (jaw_hyperlink));
}

/**
 * jaw_hypertext_data_init:
 * @ac: a Java AccessibleContext object
 *
 * Initializes hypertext interface data for an AccessibleContext.
 *
 * Creates a Java AtkHypertext wrapper and stores it in a HypertextData
 * structure.
 *
 * Explicitly manages a JNI local reference frame using
 * PushLocalFrame/PopLocalFrame; all local references are released
 * before the function returns.
 *
 * Returns: (transfer full): HypertextData pointer, or %NULL on error
 */

gpointer
jaw_hypertext_data_init (jobject ac)
{
  JAW_DEBUG_ALL ("%p", ac);
  HypertextData *data = g_new0 (HypertextData, 1);

  JNIEnv *jniEnv = jaw_util_get_jni_env ();
  jclass classHypertext = (*jniEnv)->FindClass (jniEnv, "org/GNOME/Accessibility/AtkHypertext");
  jmethodID jmid = (*jniEnv)->GetStaticMethodID (jniEnv, classHypertext, "create_atk_hypertext", "(Ljavax/accessibility/AccessibleContext;)Lorg/GNOME/Accessibility/AtkHypertext;");
  jobject jatk_hypertext = (*jniEnv)->CallStaticObjectMethod (jniEnv, classHypertext, jmid, ac);
  data->atk_hypertext = (*jniEnv)->NewGlobalRef (jniEnv, jatk_hypertext);

  data->link_table = g_hash_table_new_full (NULL, NULL, NULL, link_destroy_notify);

  return data;
}

/**
 * jaw_hypertext_data_finalize:
 * @p: HypertextData pointer to finalize
 *
 * Cleans up HypertextData when the parent GObject is finalized.
 * Called from jaw_impl_finalize() when the object's reference count reaches
 * zero.
 */

void
jaw_hypertext_data_finalize (gpointer p)
{
  JAW_DEBUG_ALL ("%p", p);
  HypertextData *data = (HypertextData *) p;
  JNIEnv *jniEnv = jaw_util_get_jni_env ();

  if (data && data->atk_hypertext)
    {
      g_hash_table_remove_all (data->link_table);

      (*jniEnv)->DeleteGlobalRef (jniEnv, data->atk_hypertext);
      data->atk_hypertext = NULL;
    }
}

/**
 * jaw_hypertext_get_link:
 * @hypertext: an #AtkHypertext
 * @link_index: an integer specifying the desired link
 *
 * Gets the link in this hypertext document at index
 * @link_index
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * Returns: (transfer none): the link in this hypertext document at
 * index @link_index
 **/

static AtkHyperlink *
jaw_hypertext_get_link (AtkHypertext *hypertext, gint link_index)
{
  JAW_DEBUG_C ("%p, %d", hypertext, link_index);
  JAW_GET_HYPERTEXT (hypertext, NULL);

  jclass classAtkHypertext = (*jniEnv)->FindClass (jniEnv, "org/GNOME/Accessibility/AtkHypertext");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv, classAtkHypertext, "get_link", "(I)Lorg/GNOME/Accessibility/AtkHyperlink;");
  jobject jhyperlink = (*jniEnv)->CallObjectMethod (jniEnv, atk_hypertext, jmid, (jint) link_index);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_hypertext);

  if (!jhyperlink)
    {
      return NULL;
    }

  JawHyperlink *jaw_hyperlink = jaw_hyperlink_new (jhyperlink);
  g_hash_table_insert (data->link_table, GINT_TO_POINTER (link_index), (gpointer) jaw_hyperlink);

  return ATK_HYPERLINK (jaw_hyperlink);
}

/**
 * jaw_hypertext_get_n_links:
 * @hypertext: an #AtkHypertext
 *
 * Gets the number of links within this hypertext document.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * Returns: the number of links within this hypertext document
 **/

static gint
jaw_hypertext_get_n_links (AtkHypertext *hypertext)
{
  JAW_DEBUG_C ("%p", hypertext);
  JAW_GET_HYPERTEXT (hypertext, 0);

  jclass classAtkHypertext = (*jniEnv)->FindClass (jniEnv, "org/GNOME/Accessibility/AtkHypertext");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv, classAtkHypertext, "get_n_links", "()I");

  gint ret = (gint) (*jniEnv)->CallIntMethod (jniEnv, atk_hypertext, jmid);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_hypertext);
  return ret;
}

/**
 * jaw_hypertext_get_link_index:
 * @hypertext: an #AtkHypertext
 * @char_index: a character index
 *
 * Gets the index into the array of hyperlinks that is associated with
 * the character specified by @char_index.
 *
 * Invoked from GLib main loop; no Push/PopLocalFrame/DeleteLocalRef needed.
 *
 * Returns: an index into the array of hyperlinks in @hypertext,
 * or -1 if there is no hyperlink associated with this character.
 **/

static gint
jaw_hypertext_get_link_index (AtkHypertext *hypertext, gint char_index)
{
  JAW_DEBUG_C ("%p, %d", hypertext, char_index);
  JAW_GET_HYPERTEXT (hypertext, -1);

  jclass classAtkHypertext = (*jniEnv)->FindClass (jniEnv, "org/GNOME/Accessibility/AtkHypertext");
  jmethodID jmid = (*jniEnv)->GetMethodID (jniEnv, classAtkHypertext, "get_link_index", "(I)I");

  gint ret = (gint) (*jniEnv)->CallIntMethod (jniEnv, atk_hypertext, jmid, (jint) char_index);
  (*jniEnv)->DeleteGlobalRef (jniEnv, atk_hypertext);
  return ret;
}
