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

#include <jni.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include "jawutil.h"
#include "jawimpl.h"
#include "jawtoplevel.h"

#define KEY_DISPATCH_NOT_DISPATCHED	0
#define KEY_DISPATCH_CONSUMED		1
#define KEY_DISPATCH_NOT_CONSUMED	2

GMutex *key_dispatch_mutex = NULL;
GCond *key_dispatch_cond = NULL;
static gint key_dispatch_result = KEY_DISPATCH_NOT_DISPATCHED;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *javaVM, void *reserve) {
	globalJvm = javaVM;
	return JNI_VERSION_1_2;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *javaVM, void *reserve) {
}

gpointer jni_main_loop(gpointer data) {
	if (!g_module_supported()) {
		return NULL;
	}

	GModule *module;
	module = g_module_open("/usr/lib/gtk-2.0/modules/libatk-bridge.so", G_MODULE_BIND_LAZY);
	if (!module) {
		return NULL;
	}

	typedef void (*DL_INIT)(void);
	DL_INIT dl_init;
	if (!g_module_symbol( module, "gnome_accessibility_module_init", (gpointer*)&dl_init)) {
		g_module_close(module);
		return NULL;
	}

	(*dl_init)();

	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();

	return NULL;
}

JNIEXPORT void JNICALL Java_org_GNOME_Accessibility_AtkWrapper_initNativeLibrary(JNIEnv *jniEnv, jclass jClass) {
	g_type_init();
	
	g_type_class_unref(g_type_class_ref(JAW_TYPE_UTIL));
	g_type_class_unref(g_type_class_ref(JAW_TYPE_MISC));
	/* Force to invoke base initialization function of each ATK interfaces */
	g_type_class_unref(g_type_class_ref(ATK_TYPE_NO_OP_OBJECT));
	
	//gdk_init(NULL, NULL);

	if (!g_thread_supported()) {
		g_thread_init(NULL);
		gdk_threads_init();
		//XInitThreads();
	}

	jaw_impl_init_mutex();

	key_dispatch_mutex = g_mutex_new();
	key_dispatch_cond = g_cond_new();

	GThread *main_loop = g_thread_create( jni_main_loop,
			(gpointer)globalJvm, FALSE, NULL);
}

typedef enum _SigalType {
	Sig_Text_Caret_Moved = 0,
	Sig_Text_Property_Changed_Insert = 1,
	Sig_Text_Property_Changed_Delete = 2,
	Sig_Text_Property_Changed_Replace = 3,
	Sig_Object_Children_Changed_Add = 4,
	Sig_Object_Children_Changed_Remove = 5,
	Sig_Object_Active_Descendant_Changed = 6,
	Sig_Object_Selection_Changed = 7,
	Sig_Object_Visible_Data_Changed = 8,
	Sig_Object_Property_Change_Accessible_Actions = 9,
	Sig_Object_Property_Change_Accessible_Value = 10,
	Sig_Object_Property_Change_Accessible_Description = 11,
	Sig_Object_Property_Change_Accessible_Name = 12,
	Sig_Object_Property_Change_Accessible_Hypertext_Offset = 13,
	Sig_Object_Property_Change_Accessible_Table_Caption = 14,
	Sig_Object_Property_Change_Accessible_Table_Summary = 15,
	Sig_Object_Property_Change_Accessible_Table_Column_Header = 16,
	Sig_Object_Property_Change_Accessible_Table_Column_Description = 17,
	Sig_Object_Property_Change_Accessible_Table_Row_Header = 18,
	Sig_Object_Property_Change_Accessible_Table_Row_Description = 19,
	Sig_Table_Model_Changed = 20,
	Sig_Text_Property_Changed = 21
}SignalType;

typedef struct _CallbackPara {
	jobject global_ac;
	gboolean is_toplevel;
	SignalType signal_id;
	jobjectArray args;
	AtkStateType atk_state;
	gboolean state_value;
} CallbackPara;

static CallbackPara*
alloc_callback_para (jobject ac)
{
	CallbackPara *para = g_new(CallbackPara, 1);
	para->global_ac = ac;
	para->args = NULL;

	return para;
}

static void
free_callback_para (CallbackPara *para)
{
	JNIEnv *jniEnv = jaw_util_get_jni_env();
	(*jniEnv)->DeleteGlobalRef(jniEnv, para->global_ac);

	if (para->args) {
		(*jniEnv)->DeleteGlobalRef(jniEnv, para->args);
	}

	g_free(para);
}

static gboolean
focus_notify_handler (gpointer p)
{
	CallbackPara *para = (CallbackPara*)p;
	jobject global_ac = para->global_ac;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	JawImpl* jaw_impl = jaw_impl_find_instance(jniEnv, global_ac);
	
	if (jaw_impl == NULL) {
		free_callback_para(para);
		return FALSE;
	}

	AtkObject* atk_obj = ATK_OBJECT(jaw_impl);
	atk_focus_tracker_notify(atk_obj);

	free_callback_para(para);

	return FALSE;
}

JNIEXPORT void JNICALL Java_org_GNOME_Accessibility_AtkWrapper_focusNotify(
		JNIEnv *jniEnv, jclass jClass, jobject jAccContext) {
	jobject global_ac = (*jniEnv)->NewGlobalRef(jniEnv, jAccContext);
	CallbackPara *para = alloc_callback_para(global_ac);

	gdk_threads_add_idle(focus_notify_handler, para);
}

static gboolean
window_open_handler (gpointer p)
{
	CallbackPara *para = (CallbackPara*)p;
	jobject global_ac = para->global_ac;
	gboolean is_toplevel = para->is_toplevel;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	JawImpl* jaw_impl = jaw_impl_get_instance(jniEnv, global_ac);
	AtkObject* atk_obj = ATK_OBJECT(jaw_impl);

	if (!strcmp(atk_role_get_name(atk_object_get_role(atk_obj)), "redundant object")) {
		free_callback_para(para);
		return FALSE;
	}

	if (atk_object_get_role(atk_obj) == ATK_ROLE_TOOL_TIP) {
		free_callback_para(para);
		return FALSE;
	}

	if (is_toplevel) {
		gint n = jaw_toplevel_add_window(JAW_TOPLEVEL(atk_get_root()), atk_obj);
		
		g_signal_emit_by_name(ATK_OBJECT(atk_get_root()),
				"children-changed::add", n, atk_obj, NULL);
		g_signal_emit(atk_obj, g_signal_lookup("window_create", JAW_TYPE_OBJECT), 0);
	}

	free_callback_para(para);

	return FALSE;
}

JNIEXPORT void JNICALL Java_org_GNOME_Accessibility_AtkWrapper_windowOpen(
		JNIEnv *jniEnv, jclass jClass, jobject jAccContext, jboolean jIsToplevel) {
	
	jobject global_ac = (*jniEnv)->NewGlobalRef(jniEnv, jAccContext);
	CallbackPara *para = alloc_callback_para(global_ac);
	para->is_toplevel = (jIsToplevel == JNI_TRUE) ? TRUE : FALSE;

	gdk_threads_add_idle(window_open_handler, para);
}

static gboolean
window_close_handler (gpointer p)
{
	CallbackPara *para = (CallbackPara*)p;
	jobject global_ac = para->global_ac;
	gboolean is_toplevel = para->is_toplevel;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	JawImpl* jaw_impl = jaw_impl_find_instance(jniEnv, global_ac);
	
	if (jaw_impl == NULL) {
		free_callback_para(para);
		return FALSE;
	}

	AtkObject* atk_obj = ATK_OBJECT(jaw_impl);

	if (!strcmp(atk_role_get_name(atk_object_get_role(atk_obj)), "redundant object")) {
		free_callback_para(para);
		return FALSE;
	}

	if (atk_object_get_role(atk_obj) == ATK_ROLE_TOOL_TIP) {
		free_callback_para(para);
		return FALSE;
	}

	if (is_toplevel) {
		gint n = jaw_toplevel_remove_window(JAW_TOPLEVEL(atk_get_root()), atk_obj);

		g_signal_emit_by_name(ATK_OBJECT(atk_get_root()),
				"children-changed::remove", n, atk_obj, NULL);
		g_signal_emit(atk_obj, g_signal_lookup("window_destroy", JAW_TYPE_OBJECT), 0);
	}

	free_callback_para(para);
	
	return FALSE;
}

JNIEXPORT void JNICALL Java_org_GNOME_Accessibility_AtkWrapper_windowClose(
		JNIEnv *jniEnv, jclass jClass, jobject jAccContext, jboolean jIsToplevel) {
	
	jobject global_ac = (*jniEnv)->NewGlobalRef(jniEnv, jAccContext);
	CallbackPara *para = alloc_callback_para(global_ac);
	para->is_toplevel = (jIsToplevel == JNI_TRUE) ? TRUE : FALSE;

	gdk_threads_add_idle(window_close_handler, para);
}

static gboolean
window_minimize_handler (gpointer p)
{
	CallbackPara *para = (CallbackPara*)p;
	jobject global_ac = para->global_ac;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	JawImpl* jaw_impl = jaw_impl_find_instance(jniEnv, global_ac);
	
	if (jaw_impl == NULL) {
		free_callback_para(para);
		return FALSE;
	}

	AtkObject* atk_obj = ATK_OBJECT(jaw_impl);
	g_signal_emit(atk_obj, g_signal_lookup("window_minimize", JAW_TYPE_OBJECT), 0);

	free_callback_para(para);

	return FALSE;
}

JNIEXPORT void JNICALL Java_org_GNOME_Accessibility_AtkWrapper_windowMinimize(
		JNIEnv *jniEnv, jclass jClass, jobject jAccContext) {
	jobject global_ac = (*jniEnv)->NewGlobalRef(jniEnv, jAccContext);
	CallbackPara *para = alloc_callback_para(global_ac);

	gdk_threads_add_idle(window_minimize_handler, para);
}

static gboolean
window_maximize_handler (gpointer p)
{
	CallbackPara *para = (CallbackPara*)p;
	jobject global_ac = para->global_ac;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	JawImpl* jaw_impl = jaw_impl_find_instance(jniEnv, global_ac);
	
	if (jaw_impl == NULL) {
		free_callback_para(para);
		return FALSE;
	}

	AtkObject* atk_obj = ATK_OBJECT(jaw_impl);
	g_signal_emit(atk_obj, g_signal_lookup("window_maximize", JAW_TYPE_OBJECT), 0);

	free_callback_para(para);

	return FALSE;
}

JNIEXPORT void JNICALL Java_org_GNOME_Accessibility_AtkWrapper_windowMaximize(
		JNIEnv *jniEnv, jclass jClass, jobject jAccContext) {

	jobject global_ac = (*jniEnv)->NewGlobalRef(jniEnv, jAccContext);
	CallbackPara *para = alloc_callback_para(global_ac);

	gdk_threads_add_idle(window_maximize_handler, para);
}

static gboolean
window_restore_handler (gpointer p)
{
	CallbackPara *para = (CallbackPara*)p;
	jobject global_ac = para->global_ac;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	JawImpl* jaw_impl = jaw_impl_find_instance(jniEnv, global_ac);
	
	if (jaw_impl == NULL) {
		free_callback_para(para);
		return FALSE;
	}

	AtkObject* atk_obj = ATK_OBJECT(jaw_impl);
	g_signal_emit(atk_obj, g_signal_lookup("window_restore", JAW_TYPE_OBJECT), 0);

	free_callback_para(para);

	return FALSE;
}

JNIEXPORT void JNICALL Java_org_GNOME_Accessibility_AtkWrapper_windowRestore(
		JNIEnv *jniEnv, jclass jClass, jobject jAccContext) {

	jobject global_ac = (*jniEnv)->NewGlobalRef(jniEnv, jAccContext);
	CallbackPara *para = alloc_callback_para(global_ac);

	gdk_threads_add_idle(window_restore_handler, para);
}

static gboolean
window_activate_handler (gpointer p)
{
	CallbackPara *para = (CallbackPara*)p;
	jobject global_ac = para->global_ac;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	JawImpl* jaw_impl = jaw_impl_find_instance(jniEnv, global_ac);
	
	if (jaw_impl == NULL) {
		free_callback_para(para);
		return FALSE;
	}

	AtkObject* atk_obj = ATK_OBJECT(jaw_impl);
	g_signal_emit(atk_obj, g_signal_lookup("window_activate", JAW_TYPE_OBJECT), 0);

	free_callback_para(para);

	return FALSE;
}

JNIEXPORT void JNICALL Java_org_GNOME_Accessibility_AtkWrapper_windowActivate(
		JNIEnv *jniEnv, jclass jClass, jobject jAccContext) {

	jobject global_ac = (*jniEnv)->NewGlobalRef(jniEnv, jAccContext);
	CallbackPara *para = alloc_callback_para(global_ac);

	gdk_threads_add_idle(window_activate_handler, para);
}

static gboolean
window_deactivate_handler (gpointer p)
{
	CallbackPara *para = (CallbackPara*)p;
	jobject global_ac = para->global_ac;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	JawImpl* jaw_impl = jaw_impl_find_instance(jniEnv, global_ac);
	
	if (jaw_impl == NULL) {
		free_callback_para(para);
		return FALSE;
	}

	AtkObject* atk_obj = ATK_OBJECT(jaw_impl);
	g_signal_emit(atk_obj, g_signal_lookup("window_deactivate", JAW_TYPE_OBJECT), 0);

	free_callback_para(para);

	return FALSE;
}

JNIEXPORT void JNICALL Java_org_GNOME_Accessibility_AtkWrapper_windowDeactivate(
		JNIEnv *jniEnv, jclass jClass, jobject jAccContext) {

	jobject global_ac = (*jniEnv)->NewGlobalRef(jniEnv, jAccContext);
	CallbackPara *para = alloc_callback_para(global_ac);

	gdk_threads_add_idle(window_deactivate_handler, para);
}

static gboolean
window_state_change_handler (gpointer p)
{
	CallbackPara *para = (CallbackPara*)p;
	jobject global_ac = para->global_ac;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	JawImpl* jaw_impl = jaw_impl_find_instance(jniEnv, global_ac);
	
	if (jaw_impl == NULL) {
		free_callback_para(para);
		return FALSE;
	}

	AtkObject* atk_obj = ATK_OBJECT(jaw_impl);
	g_signal_emit(atk_obj, g_signal_lookup("state-change", JAW_TYPE_OBJECT), 0);

	free_callback_para(para);

	return FALSE;
}

JNIEXPORT void JNICALL Java_org_GNOME_Accessibility_AtkWrapper_windowStateChange(
		JNIEnv *jniEnv, jclass jClass, jobject jAccContext) {

	jobject global_ac = (*jniEnv)->NewGlobalRef(jniEnv, jAccContext);
	CallbackPara *para = alloc_callback_para(global_ac);

	gdk_threads_add_idle(window_state_change_handler, para);
}

static gchar
get_char_value (JNIEnv *jniEnv, jobject o)
{
	jclass classByte = (*jniEnv)->FindClass(jniEnv, "java/lang/Byte");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classByte, "byteValue", "()B");
	return (gchar)(*jniEnv)->CallByteMethod(jniEnv, o, jmid);
}

static gdouble
get_double_value (JNIEnv *jniEnv, jobject o)
{
	jclass classDouble = (*jniEnv)->FindClass(jniEnv, "java/lang/Double");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classDouble, "doubleValue", "()D");
	return (gdouble)(*jniEnv)->CallDoubleMethod(jniEnv, o, jmid);
}

static gfloat
get_float_value (JNIEnv *jniEnv, jobject o)
{
	jclass classFloat = (*jniEnv)->FindClass(jniEnv, "java/lang/Float");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classFloat, "floatValue", "()F");
	return (gfloat)(*jniEnv)->CallFloatMethod(jniEnv, o, jmid);
}

static gint
get_int_value (JNIEnv *jniEnv, jobject o)
{
	jclass classInteger = (*jniEnv)->FindClass(jniEnv, "java/lang/Integer");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classInteger, "intValue", "()I");
	return (gint)(*jniEnv)->CallIntMethod(jniEnv, o, jmid);
}

static gint64
get_int64_value (JNIEnv *jniEnv, jobject o)
{
	jclass classLong = (*jniEnv)->FindClass(jniEnv, "java/lang/Long");
	jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv, classLong, "longValue", "()J");
	return (gint64)(*jniEnv)->CallLongMethod(jniEnv, o, jmid);
}

static gboolean
signal_emit_handler (gpointer p)
{
	CallbackPara *para = (CallbackPara*)p;
	jobject global_ac = para->global_ac;
	jobjectArray args = para->args;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	JawImpl* jaw_impl = jaw_impl_find_instance(jniEnv, global_ac);
	
	if (jaw_impl == NULL) {
		free_callback_para(para);
		return FALSE;
	}
	
	AtkObject* atk_obj = ATK_OBJECT(jaw_impl);

	switch (para->signal_id) {
		case Sig_Text_Caret_Moved:
		{
			gint cursor_pos = get_int_value(
					jniEnv,
					(*jniEnv)->GetObjectArrayElement(jniEnv, args, 0));
			g_signal_emit_by_name(atk_obj, "text_caret_moved", cursor_pos);
			break;
		}
		case Sig_Text_Property_Changed_Insert:
		{
			gint insert_position = get_int_value(
					jniEnv,
					(*jniEnv)->GetObjectArrayElement(jniEnv, args, 0));
			gint insert_length = get_int_value(
					jniEnv,
					(*jniEnv)->GetObjectArrayElement(jniEnv, args, 1));
			g_signal_emit_by_name(atk_obj,
					"text_changed::insert",
					insert_position,
					insert_length);
			break;
		}
		case Sig_Text_Property_Changed_Delete:
		{
			gint delete_position = get_int_value(
					jniEnv,
					(*jniEnv)->GetObjectArrayElement(jniEnv, args, 0));
			gint delete_length = get_int_value(
					jniEnv,
					(*jniEnv)->GetObjectArrayElement(jniEnv, args, 1));
			g_signal_emit_by_name(atk_obj,
					"text_changed::delete",
					delete_position,
					delete_length);
			break;
		}
		case Sig_Object_Children_Changed_Add:
		{
			gint child_index = get_int_value(
					jniEnv,
					(*jniEnv)->GetObjectArrayElement(jniEnv, args, 0));
			jobject child_ac = (*jniEnv)->GetObjectArrayElement(jniEnv, args, 1);
			JawImpl *child_impl = jaw_impl_get_instance(jniEnv, child_ac);
			if (!child_impl) {
				break;
			}

			g_signal_emit_by_name(atk_obj,
					"children_changed::add",
					child_index,
					child_impl);
			break;
		}
		case Sig_Object_Children_Changed_Remove:
		{
			gint child_index = get_int_value(
					jniEnv,
					(*jniEnv)->GetObjectArrayElement(jniEnv, args, 0));
			jobject child_ac = (*jniEnv)->GetObjectArrayElement(jniEnv, args, 1);
			JawImpl *child_impl = jaw_impl_find_instance(jniEnv, child_ac);
			if (!child_impl) {
				break;
			}

			g_signal_emit_by_name(atk_obj,
					"children_changed::remove",
					child_index,
					child_impl);
			g_object_unref(G_OBJECT(atk_obj));
			break;
		}
		case Sig_Object_Active_Descendant_Changed:
		{
			jobject child_ac = (*jniEnv)->GetObjectArrayElement(jniEnv, args, 0);
			JawImpl *child_impl = jaw_impl_find_instance(jniEnv, child_ac);
			if (!child_impl) {
				break;
			}

			g_signal_emit_by_name(atk_obj,
					"active_descendant_changed",
					child_impl);
			break;
		}
		case Sig_Object_Selection_Changed:
		{
			g_signal_emit_by_name(atk_obj,
					"selection_changed");
			break;
		}
		case Sig_Object_Visible_Data_Changed:
		{
			g_signal_emit_by_name(atk_obj,
					"visible_data_changed");
			break;
		}
		case Sig_Object_Property_Change_Accessible_Actions:
		{
			gint oldValue = get_int_value(
					jniEnv,
					(*jniEnv)->GetObjectArrayElement(jniEnv, args, 0));
			gint newValue = get_int_value(
					jniEnv,
					(*jniEnv)->GetObjectArrayElement(jniEnv, args, 1));
			AtkPropertyValues values = { NULL };
			g_value_init(&values.old_value, G_TYPE_INT);
			g_value_set_int(&values.old_value, oldValue);
			g_value_init(&values.new_value, G_TYPE_INT);
			g_value_set_int(&values.new_value, newValue);
			values.property_name = "accessible-actions";

			g_signal_emit_by_name(atk_obj,
					"property_change::accessible-actions",
					&values);
			break;
		}
		case Sig_Object_Property_Change_Accessible_Value:
		{
			gdouble oldValue = get_double_value(
					jniEnv,
					(*jniEnv)->GetObjectArrayElement(jniEnv, args, 0));
			gdouble newValue = get_double_value(
					jniEnv,
					(*jniEnv)->GetObjectArrayElement(jniEnv, args, 1));
			AtkPropertyValues values = { NULL };
			g_value_init(&values.old_value, G_TYPE_DOUBLE);
			g_value_set_double(&values.old_value, oldValue);
			g_value_init(&values.new_value, G_TYPE_DOUBLE);
			g_value_set_double(&values.new_value, newValue);
			values.property_name = "accessible-value";

			g_signal_emit_by_name(atk_obj,
					"property_change::accessible-value",
					&values);
			break;
		}
		case Sig_Object_Property_Change_Accessible_Description:
		{
			g_signal_emit_by_name(atk_obj,
					"property_change::accessible-description",
					NULL);
			break;
		}
		case Sig_Object_Property_Change_Accessible_Name:
		{
			g_signal_emit_by_name(atk_obj,
					"property_change::accessible-name",
					NULL);
			break;
		}
		case Sig_Object_Property_Change_Accessible_Hypertext_Offset:
		{
			g_signal_emit_by_name(atk_obj,
					"property_change::accessible-hypertext-offset",
					NULL);
			break;
		}
		case Sig_Object_Property_Change_Accessible_Table_Caption:
		{
			g_signal_emit_by_name(atk_obj,
					"property_change::accessible-table-caption",
					NULL);
			break;
		}
		case Sig_Object_Property_Change_Accessible_Table_Summary:
		{
			g_signal_emit_by_name(atk_obj,
					"property_change::accessible-table-summary",
					NULL);
			break;
		}
		case Sig_Object_Property_Change_Accessible_Table_Column_Header:
		{
			g_signal_emit_by_name(atk_obj,
					"property_change::accessible-table-column-header",
					NULL);
			break;
		}
		case Sig_Object_Property_Change_Accessible_Table_Column_Description:
		{
			g_signal_emit_by_name(atk_obj,
					"property_change::accessible-table-column-description",
					NULL);
			break;
		}
		case Sig_Object_Property_Change_Accessible_Table_Row_Header:
		{
			g_signal_emit_by_name(atk_obj,
					"property_change::accessible-table-row-header",
					NULL);
			break;
		}
		case Sig_Object_Property_Change_Accessible_Table_Row_Description:
		{
			g_signal_emit_by_name(atk_obj,
					"property_change::accessible-table-row-description",
					NULL);
			break;
		}
		case Sig_Table_Model_Changed:
		{
			g_signal_emit_by_name(atk_obj,
					"model_changed");
			break;
		}
		case Sig_Text_Property_Changed:
		{
			JawObject * jaw_obj = JAW_OBJECT(atk_obj);

			gint newValue = get_int_value(
					jniEnv,
					(*jniEnv)->GetObjectArrayElement(jniEnv, args, 0));

			gint prevCount = (gint)g_hash_table_lookup(
					jaw_obj->storedData,
					"Previous_Count");
			gint curCount = atk_text_get_character_count(
					ATK_TEXT(jaw_obj));

			g_hash_table_insert(
					jaw_obj->storedData,
					"Previous_Count",
					(gpointer)curCount);

			if (curCount > prevCount) {
				g_signal_emit_by_name(atk_obj,
						"text_changed::insert",
						newValue,
						curCount - prevCount);
			} else if (curCount < prevCount) {
				g_signal_emit_by_name(atk_obj,
						"text_changed::delete",
						newValue,
						prevCount - curCount);
			}

			break;
		}
		default:
			break;
	}

	free_callback_para(para);

	return FALSE;
}

JNIEXPORT void JNICALL Java_org_GNOME_Accessibility_AtkWrapper_emitSignal(
		JNIEnv *jniEnv, jclass jClass, jobject jAccContext, jint id, jobjectArray args) {
	jobject global_ac = (*jniEnv)->NewGlobalRef(jniEnv, jAccContext);
	jobjectArray global_args = (jobjectArray)(*jniEnv)->NewGlobalRef(jniEnv, args);
	CallbackPara *para = alloc_callback_para(global_ac);
	para->signal_id = (gint)id;
	para->args = global_args;

	gdk_threads_add_idle(signal_emit_handler, para);
}

static gboolean
object_state_change_handler (gpointer p)
{
	CallbackPara *para = (CallbackPara*)p;
	jobject global_ac = para->global_ac;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	JawImpl* jaw_impl = jaw_impl_find_instance(jniEnv, global_ac);
	
	if (jaw_impl == NULL) {
		free_callback_para(para);
		return FALSE;
	}

	atk_object_notify_state_change(
			ATK_OBJECT(jaw_impl),
			para->atk_state,
			para->state_value);

	free_callback_para(para);

	return FALSE;
}

JNIEXPORT void JNICALL Java_org_GNOME_Accessibility_AtkWrapper_objectStateChange(
		JNIEnv *jniEnv, jclass jClass, jobject jAccContext, jobject state, jboolean value) {
	jobject global_ac = (*jniEnv)->NewGlobalRef(jniEnv, jAccContext);
	CallbackPara *para = alloc_callback_para(global_ac);
	AtkStateType state_type = jaw_util_get_atk_state_type_from_java_state( jniEnv, state );
	para->atk_state = state_type;
	if (value == JNI_TRUE) {
		para->state_value = TRUE;
	} else {
		para->state_value = FALSE;
	}

	gdk_threads_add_idle(object_state_change_handler, para);
}

static gboolean
component_added_handler (gpointer p)
{
	CallbackPara *para = (CallbackPara*)p;
	jobject global_ac = para->global_ac;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	JawImpl* jaw_impl = jaw_impl_get_instance(jniEnv, global_ac);
	
	free_callback_para(para);

	return FALSE;
}

JNIEXPORT void JNICALL Java_org_GNOME_Accessibility_AtkWrapper_componentAdded(
		JNIEnv *jniEnv, jclass jClass, jobject jAccContext) {
	jobject global_ac = (*jniEnv)->NewGlobalRef(jniEnv, jAccContext);
	CallbackPara *para = alloc_callback_para(global_ac);

	gdk_threads_add_idle(component_added_handler, para);
}

static gboolean
key_dispatch_handler (gpointer p)
{
	g_mutex_lock(key_dispatch_mutex);

	jobject jAtkKeyEvent = (jobject)p;
	JNIEnv *jniEnv = jaw_util_get_jni_env();

	AtkKeyEventStruct *event = g_new0(AtkKeyEventStruct, 1);
	jclass classAtkKeyEvent = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkKeyEvent");

	// type
	jfieldID jfidType = (*jniEnv)->GetFieldID(jniEnv, classAtkKeyEvent, "type", "I");
	jint type = (*jniEnv)->GetIntField(jniEnv, jAtkKeyEvent, jfidType);

	jfieldID jfidTypePressed = (*jniEnv)->GetStaticFieldID(jniEnv, classAtkKeyEvent, "ATK_KEY_EVENT_PRESSED", "I");
	jfieldID jfidTypeReleased = (*jniEnv)->GetStaticFieldID(jniEnv, classAtkKeyEvent, "ATK_KEY_EVENT_RELEASED", "I");
	jint type_pressed = (*jniEnv)->GetStaticIntField(jniEnv, classAtkKeyEvent, jfidTypePressed);
	jint type_released = (*jniEnv)->GetStaticIntField(jniEnv, classAtkKeyEvent, jfidTypeReleased);

	if (type == type_pressed) {
		event->type = ATK_KEY_EVENT_PRESS;
	} else if (type == type_released) {
		event->type = ATK_KEY_EVENT_RELEASE;
	}

	// state
	jfieldID jfidShift = (*jniEnv)->GetFieldID(jniEnv, classAtkKeyEvent, "isShiftKeyDown", "Z");
	jboolean jShiftKeyDown = (*jniEnv)->GetBooleanField(jniEnv, jAtkKeyEvent, jfidShift);
	if (jShiftKeyDown == JNI_TRUE) {
		event->state |= GDK_SHIFT_MASK;
	}

	jfieldID jfidCtrl = (*jniEnv)->GetFieldID(jniEnv, classAtkKeyEvent, "isCtrlKeyDown", "Z");
	jboolean jCtrlKeyDown = (*jniEnv)->GetBooleanField(jniEnv, jAtkKeyEvent, jfidCtrl);
	if (jCtrlKeyDown == JNI_TRUE) {
		event->state |= GDK_CONTROL_MASK;
	}
	
	jfieldID jfidAlt = (*jniEnv)->GetFieldID(jniEnv, classAtkKeyEvent, "isAltKeyDown", "Z");
	jboolean jAltKeyDown = (*jniEnv)->GetBooleanField(jniEnv, jAtkKeyEvent, jfidAlt);
	if (jAltKeyDown == JNI_TRUE) {
		event->state |= GDK_MOD1_MASK;
	}

	jfieldID jfidMeta = (*jniEnv)->GetFieldID(jniEnv, classAtkKeyEvent, "isMetaKeyDown", "Z");
	jboolean jMetaKeyDown = (*jniEnv)->GetBooleanField(jniEnv, jAtkKeyEvent, jfidMeta);
	if (jMetaKeyDown == JNI_TRUE) {
		event->state |= GDK_META_MASK;
	}

	// keyval
	jfieldID jfidKeyval = (*jniEnv)->GetFieldID(jniEnv, classAtkKeyEvent, "keyval", "I");
	jint jkeyval = (*jniEnv)->GetIntField(jniEnv, jAtkKeyEvent, jfidKeyval);
	event->keyval = (guint)jkeyval;

	// string
	jfieldID jfidString = (*jniEnv)->GetFieldID(jniEnv, classAtkKeyEvent, "string", "Ljava/lang/String;");
	jstring jstr = (jstring)(*jniEnv)->GetObjectField(jniEnv, jAtkKeyEvent, jfidString);
	event->length = (gint)(*jniEnv)->GetStringLength(jniEnv, jstr);
	event->string = (gchar*)(*jniEnv)->GetStringUTFChars(jniEnv, jstr, NULL);

	// keycode
	jfieldID jfidKeycode = (*jniEnv)->GetFieldID(jniEnv, classAtkKeyEvent, "keycode", "I");
	event->keycode = (gint)(*jniEnv)->GetIntField(jniEnv, jAtkKeyEvent, jfidKeycode);

	/*GdkKeymapKey *keys = NULL;
	gint n_keys = 0;
	gboolean res = gdk_keymap_get_entries_for_keyval(
			gdk_keymap_get_default(),
			event->keyval,
			&keys,
			&n_keys);
	if (res) {
		event->keycode = 0;
		for (int i = 0; i < n_keys; i++) {
			if (keys[i].group == 0) {
				event->keycode = keys[i].keycode;
				break;
			}
		}

		if (event->keycode == 0) {
			event->keycode = keys[0].keycode;
		}
	}*/

	// timestamp
	jfieldID jfidTimestamp = (*jniEnv)->GetFieldID(jniEnv, classAtkKeyEvent, "timestamp", "I");
	event->timestamp = (guint32)(*jniEnv)->GetIntField(jniEnv, jAtkKeyEvent, jfidTimestamp);

	gboolean b = jaw_util_dispatch_key_event (event);
	if (b) {
		key_dispatch_result = KEY_DISPATCH_CONSUMED;
	} else {
		key_dispatch_result = KEY_DISPATCH_NOT_CONSUMED;
	}

	(*jniEnv)->ReleaseStringUTFChars(jniEnv, jstr, event->string);
	g_free(event);

	(*jniEnv)->DeleteGlobalRef(jniEnv, jAtkKeyEvent);

	g_cond_signal(key_dispatch_cond);
	g_mutex_unlock(key_dispatch_mutex);

	return FALSE;
}

JNIEXPORT jboolean JNICALL Java_org_GNOME_Accessibility_AtkWrapper_dispatchKeyEvent(
		JNIEnv *jniEnv, jclass jClass, jobject jAtkKeyEvent) {
	jboolean key_consumed;
	jobject global_key_event = (*jniEnv)->NewGlobalRef(jniEnv, jAtkKeyEvent);

	g_mutex_lock(key_dispatch_mutex);
	
	gdk_threads_add_idle(key_dispatch_handler, (gpointer)global_key_event);

	while (key_dispatch_result == KEY_DISPATCH_NOT_DISPATCHED) {
		g_cond_wait(key_dispatch_cond, key_dispatch_mutex);
	}

	if (key_dispatch_result == KEY_DISPATCH_CONSUMED) {
		key_consumed = JNI_TRUE;
	} else {
		key_consumed = JNI_FALSE;
	}

	key_dispatch_result = KEY_DISPATCH_NOT_DISPATCHED;
	
	g_mutex_unlock(key_dispatch_mutex);

	return key_consumed;
}

