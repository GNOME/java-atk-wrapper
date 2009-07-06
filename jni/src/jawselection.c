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

extern void	jaw_selection_interface_init (AtkSelectionIface*);
extern gpointer	jaw_selection_data_init (jobject ac);
extern void	jaw_selection_data_finalize (gpointer);

static gboolean			jaw_selection_add_selection		(AtkSelection	*selection,
									 gint		i);
static gboolean			jaw_selection_clear_selection		(AtkSelection	*selection);
static AtkObject*		jaw_selection_ref_selection		(AtkSelection	*selection,
									 gint		i);
static gint			jaw_selection_get_selection_count	(AtkSelection	*selection);
static gboolean			jaw_selection_is_child_selected		(AtkSelection	*selection,
									 gint		i);
static gboolean			jaw_selection_remove_selection		(AtkSelection	*selection,
									 gint		i);
static gboolean			jaw_selection_select_all_selection	(AtkSelection	*selection);

typedef struct _SelectionData {
	jobject atk_selection;
} SelectionData;

void
jaw_selection_interface_init (AtkSelectionIface *iface)
{
	iface->add_selection = jaw_selection_add_selection;
	iface->clear_selection = jaw_selection_clear_selection;
	iface->ref_selection = jaw_selection_ref_selection;
	iface->get_selection_count = jaw_selection_get_selection_count;
	iface->is_child_selected = jaw_selection_is_child_selected;
	iface->remove_selection = jaw_selection_remove_selection;
	iface->select_all_selection = jaw_selection_select_all_selection;
}

gpointer
jaw_selection_data_init (jobject ac)
{
	SelectionData *data = g_new0(SelectionData, 1);

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classSelection = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkSelection");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classSelection, "<init>", "(Ljavax/accessibility/AccessibleContext;)V");
	jobject jatk_selection = (*jniEnv)->NewObject(jniEnv, classSelection, jmid, ac);
	data->atk_selection = (*jniEnv)->NewGlobalRef(jniEnv, jatk_selection);

	return data;
}

void
jaw_selection_data_finalize (gpointer p)
{
	SelectionData *data = (SelectionData*)p;
	JNIEnv *jniEnv = jaw_util_get_jni_env();

	if (data && data->atk_selection) {
		(*jniEnv)->DeleteGlobalRef(jniEnv, data->atk_selection);
		data->atk_selection = NULL;
	}
}

static gboolean
jaw_selection_add_selection (AtkSelection *selection, gint i)
{
	JawObject *jaw_obj = JAW_OBJECT(selection);
	SelectionData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_SELECTION);
	jobject atk_selection = data->atk_selection;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkSelection = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkSelection");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkSelection, "add_selection", "(I)Z");
	jboolean jbool = (*jniEnv)->CallBooleanMethod(jniEnv, atk_selection, jmid, (jint)i);

	if (jbool == JNI_TRUE) {
		return TRUE;
	} else {
		return FALSE;
	}
}

static gboolean
jaw_selection_clear_selection (AtkSelection *selection)
{
	JawObject *jaw_obj = JAW_OBJECT(selection);
	SelectionData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_SELECTION);
	jobject atk_selection = data->atk_selection;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkSelection = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkSelection");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkSelection, "clear_selection", "()Z");
	jboolean jbool = (*jniEnv)->CallBooleanMethod(jniEnv, atk_selection, jmid);

	if (jbool == JNI_TRUE) {
		return TRUE;
	} else {
		return FALSE;
	}
}

static AtkObject*
jaw_selection_ref_selection (AtkSelection *selection, gint i)
{
	JawObject *jaw_obj = JAW_OBJECT(selection);
	SelectionData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_SELECTION);
	jobject atk_selection = data->atk_selection;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkSelection = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkSelection");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkSelection, "ref_selection", "(I)Ljavax/accessibility/Accessible;");
	jobject jchild = (*jniEnv)->CallObjectMethod(jniEnv, atk_selection, jmid, (jint)i);
	
	jclass classAccessible = (*jniEnv)->FindClass( jniEnv, "javax/accessibility/Accessible" );
	jmid = (*jniEnv)->GetMethodID( jniEnv, classAccessible, "getAccessibleContext", "()Ljavax/accessibility/AccessibleContext;" );
	jobject child_ac = (*jniEnv)->CallObjectMethod( jniEnv, jchild, jmid );

	AtkObject *obj = (AtkObject*) jaw_impl_get_instance( jniEnv, child_ac );
	g_object_ref (G_OBJECT(obj));

	return obj;
}

static gint
jaw_selection_get_selection_count (AtkSelection *selection)
{
	JawObject *jaw_obj = JAW_OBJECT(selection);
	SelectionData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_SELECTION);
	jobject atk_selection = data->atk_selection;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkSelection = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkSelection");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkSelection, "get_selection_count", "()I");
	jint jcount = (*jniEnv)->CallIntMethod(jniEnv, atk_selection, jmid);

	return (gint)jcount;
}

static gboolean
jaw_selection_is_child_selected (AtkSelection *selection, gint i)
{
	JawObject *jaw_obj = JAW_OBJECT(selection);
	SelectionData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_SELECTION);
	jobject atk_selection = data->atk_selection;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkSelection = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkSelection");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkSelection, "is_child_selected", "(I)Z");
	jboolean jbool = (*jniEnv)->CallBooleanMethod(jniEnv, atk_selection, jmid, (jint)i);

	if (jbool == JNI_TRUE) {
		return TRUE;
	} else {
		return FALSE;
	}
}

static gboolean
jaw_selection_remove_selection (AtkSelection *selection, gint i)
{
	JawObject *jaw_obj = JAW_OBJECT(selection);
	SelectionData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_SELECTION);
	jobject atk_selection = data->atk_selection;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkSelection = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkSelection");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkSelection, "remove_selection", "(I)Z");
	jboolean jbool = (*jniEnv)->CallBooleanMethod(jniEnv, atk_selection, jmid, (jint)i);

	if (jbool == JNI_TRUE) {
		return TRUE;
	} else {
		return FALSE;
	}
}

static gboolean
jaw_selection_select_all_selection (AtkSelection *selection)
{
	JawObject *jaw_obj = JAW_OBJECT(selection);
	SelectionData *data = jaw_object_get_interface_data(jaw_obj, INTERFACE_SELECTION);
	jobject atk_selection = data->atk_selection;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	jclass classAtkSelection = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkSelection");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classAtkSelection, "select_all_selection", "()Z");
	jboolean jbool = (*jniEnv)->CallBooleanMethod(jniEnv, atk_selection, jmid);

	if (jbool == JNI_TRUE) {
		return TRUE;
	} else {
		return FALSE;
	}
}

