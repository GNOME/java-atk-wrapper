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
#include "jawhyperlink.h"

extern void	jaw_hypertext_interface_init	(AtkHypertextIface*);
extern gpointer	jaw_hypertext_data_init		(jobject);
extern void	jaw_hypertext_data_finalize	(gpointer);

static AtkHyperlink* 		jaw_hypertext_get_link		(AtkHypertext *hypertext,
								 gint link_index);
static gint			jaw_hypertext_get_n_links	(AtkHypertext *hypertext);
static gint			jaw_hypertext_get_link_index	(AtkHypertext *hypertext,
								 gint char_index);

typedef struct _HypertextData {
	jobject atk_hypertext;
	GHashTable *link_table;
} HypertextData;

void
jaw_hypertext_interface_init (AtkHypertextIface *iface)
{
	iface->get_link = jaw_hypertext_get_link;
	iface->get_n_links = jaw_hypertext_get_n_links;
	iface->get_link_index = jaw_hypertext_get_link_index;
}

static void
link_destroy_notify (gpointer p)
{
	JawHyperlink* jaw_hyperlink = (JawHyperlink*)p;
	if(G_OBJECT(jaw_hyperlink) != NULL)
		g_object_unref(G_OBJECT(jaw_hyperlink));
}

gpointer
jaw_hypertext_data_init (jobject ac)
{
	HypertextData *data = g_new0(HypertextData, 1);

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classHypertext = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkHypertext");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classHypertext, "<init>", "(Ljavax/accessibility/AccessibleContext;)V");
	jobject jatk_hypertext = (*jniEnv)->NewObject(jniEnv, classHypertext, jmid, ac);
	data->atk_hypertext = (*jniEnv)->NewGlobalRef(jniEnv, jatk_hypertext);

	data->link_table = g_hash_table_new_full(NULL, NULL, NULL, link_destroy_notify);

	return data;
}

void
jaw_hypertext_data_finalize (gpointer p)
{
	HypertextData *data = (HypertextData*)p;
	JNIEnv *jniEnv = jaw_util_get_jni_env();

	if (data && data->atk_hypertext) {
		g_hash_table_remove_all(data->link_table);

		(*jniEnv)->DeleteGlobalRef(jniEnv, data->atk_hypertext);
		data->atk_hypertext = NULL;
	}
}

static AtkHyperlink*
jaw_hypertext_get_link (AtkHypertext *hypertext, gint link_index)
{
	JawObject *jaw_obj = JAW_OBJECT(hypertext);
	HypertextData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_HYPERTEXT);
	jobject atk_hypertext = data->atk_hypertext;
	
	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkHypertext = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkHypertext");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkHypertext, "get_link", "(I)Lorg/GNOME/Accessibility/AtkHyperlink;");
	jobject jhyperlink = (*jniEnv)->CallObjectMethod(jniEnv, atk_hypertext, jmid, (jint)link_index);

	if (!jhyperlink) {
		return NULL;
	}

	JawHyperlink *jaw_hyperlink = jaw_hyperlink_new(jhyperlink);
	g_hash_table_insert(data->link_table, GINT_TO_POINTER(link_index), (gpointer)jaw_hyperlink);

	return ATK_HYPERLINK(jaw_hyperlink);
}

static gint
jaw_hypertext_get_n_links (AtkHypertext *hypertext)
{
	JawObject *jaw_obj = JAW_OBJECT(hypertext);
	HypertextData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_HYPERTEXT);
	jobject atk_hypertext = data->atk_hypertext;
	
	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkHypertext = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkHypertext");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkHypertext, "get_n_links", "()I");
	
	return (gint)(*jniEnv)->CallIntMethod(jniEnv, atk_hypertext, jmid);
}

static gint
jaw_hypertext_get_link_index (AtkHypertext *hypertext, gint char_index)
{
	JawObject *jaw_obj = JAW_OBJECT(hypertext);
	HypertextData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_HYPERTEXT);
	jobject atk_hypertext = data->atk_hypertext;
	
	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkHypertext = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkHypertext");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkHypertext, "get_link_index", "(I)I");
	
	return (gint)(*jniEnv)->CallIntMethod(jniEnv, atk_hypertext, jmid, (jint)char_index);
}

