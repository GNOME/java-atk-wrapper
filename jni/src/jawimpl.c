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

#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include "jawutil.h"
#include "jawimpl.h"
#include "jawtoplevel.h"

static void			jaw_impl_class_init			(JawImplClass		*klass);
//static void			jaw_impl_init				(JawImpl		*impl);
static void			jaw_impl_dispose			(GObject		*gobject);
static void			jaw_impl_finalize			(GObject		*gobject);

static gpointer			jaw_impl_get_interface_data		(JawObject		*jaw_obj,
									 guint			iface);

/* AtkObject */
static void			jaw_impl_initialize			(AtkObject		*atk_obj,
									 gpointer		data);
static AtkObject*		jaw_impl_get_parent			(AtkObject		*atk_obj);
static AtkObject*		jaw_impl_ref_child			(AtkObject		*atk_obj,
									 gint			i);
static AtkRelationSet*		jaw_impl_ref_relation_set		(AtkObject		*atk_obj);

extern void	jaw_action_interface_init (AtkActionIface*);
extern gpointer	jaw_action_data_init (jobject);
extern void	jaw_action_data_finalize (gpointer);

extern void	jaw_component_interface_init (AtkComponentIface*);
extern gpointer	jaw_component_data_init (jobject);
extern void	jaw_component_data_finalize (gpointer);

extern void	jaw_text_interface_init (AtkTextIface*);
extern gpointer	jaw_text_data_init (jobject);
extern void	jaw_text_data_finalize (gpointer);

extern void	jaw_editable_text_interface_init (AtkEditableTextIface*);
extern gpointer	jaw_editable_text_data_init (jobject);
extern void	jaw_editable_text_data_finalize (gpointer);

extern void	jaw_hypertext_interface_init (AtkHypertextIface*);
extern gpointer	jaw_hypertext_data_init (jobject);
extern void	jaw_hypertext_data_finalize (gpointer);

extern void	jaw_image_interface_init (AtkImageIface*);
extern gpointer jaw_image_data_init (jobject);
extern void	jaw_image_data_finalize (gpointer);

extern void	jaw_selection_interface_init (AtkSelectionIface*);
extern gpointer	jaw_selection_data_init (jobject);
extern void	jaw_selection_data_finalize (gpointer);

extern void	jaw_value_interface_init (AtkValueIface*);
extern gpointer	jaw_value_data_init (jobject);
extern void	jaw_value_data_finalize (gpointer);

extern void	jaw_table_interface_init (AtkTableIface*);
extern gpointer	jaw_table_data_init (jobject);
extern void	jaw_table_data_finalize (gpointer);

typedef struct _JawInterfaceInfo {
	void (*finalize) (gpointer);
	gpointer data;
} JawInterfaceInfo;

static gpointer			jaw_impl_parent_class = NULL;

static GHashTable *typeTable = NULL;
static GHashTable *objectTable = NULL;
static GMutex *objectTableMutex = NULL;

void
jaw_impl_init_mutex ()
{
	if (objectTableMutex == NULL)
		objectTableMutex = g_mutex_new();
}

static void
object_table_insert ( JNIEnv *jniEnv, jobject ac, JawImpl * jaw_impl )
{
	jclass classAccessibleContext = (*jniEnv)->FindClass( jniEnv, "javax/accessibility/AccessibleContext" );
	jmethodID jmid = (*jniEnv)->GetMethodID( jniEnv, classAccessibleContext, "hashCode", "()I" );
	gint hash_key = (gint)(*jniEnv)->CallIntMethod( jniEnv, ac, jmid );

	g_mutex_lock(objectTableMutex);
	g_hash_table_insert(objectTable, (gpointer)hash_key, (gpointer)jaw_impl);
	g_mutex_unlock(objectTableMutex);
}

static JawImpl*
object_table_lookup ( JNIEnv *jniEnv, jobject ac )
{
	jclass classAccessibleContext = (*jniEnv)->FindClass( jniEnv, "javax/accessibility/AccessibleContext" );
	jmethodID jmid = (*jniEnv)->GetMethodID( jniEnv, classAccessibleContext, "hashCode", "()I" );
	gint hash_key = (gint)(*jniEnv)->CallIntMethod( jniEnv, ac, jmid );
	gpointer value = NULL;
	
	g_mutex_lock(objectTableMutex);
	value = g_hash_table_lookup(objectTable, (gpointer)hash_key);
	g_mutex_unlock(objectTableMutex);

	return (JawImpl*)value;
}

static void
object_table_remove (JNIEnv *jniEnv, jobject ac )
{
	jclass classAccessibleContext = (*jniEnv)->FindClass( jniEnv, "javax/accessibility/AccessibleContext" );
	jmethodID jmid = (*jniEnv)->GetMethodID( jniEnv, classAccessibleContext, "hashCode", "()I" );
	gint hash_key = (gint)(*jniEnv)->CallIntMethod( jniEnv, ac, jmid );
	
	g_mutex_lock(objectTableMutex);
	g_hash_table_remove( objectTable, (gpointer)hash_key );
	g_mutex_unlock(objectTableMutex);
}

static void
aggregate_interface (JNIEnv *jniEnv,
		JawObject *jaw_obj,
		guint tflag)
{
	JawImpl *jaw_impl = JAW_IMPL(tflag, jaw_obj);

	jobject ac = jaw_obj->acc_context;
	jaw_impl->ifaceTable = g_hash_table_new(NULL, NULL);

	if (tflag & INTERFACE_ACTION) {
		JawInterfaceInfo *info = g_new(JawInterfaceInfo, 1);
		info->data = jaw_action_data_init(ac);
		info->finalize = jaw_action_data_finalize;
		g_hash_table_insert(jaw_impl->ifaceTable, (gpointer)INTERFACE_ACTION, (gpointer)info);
	}

	if (tflag & INTERFACE_COMPONENT) {
		JawInterfaceInfo *info = g_new(JawInterfaceInfo, 1);
		info->data = jaw_component_data_init(ac);
		info->finalize = jaw_component_data_finalize;
		g_hash_table_insert(jaw_impl->ifaceTable, (gpointer)INTERFACE_COMPONENT, (gpointer)info);
	}

	if (tflag & INTERFACE_TEXT) {
		JawInterfaceInfo *info = g_new(JawInterfaceInfo, 1);
		info->data = jaw_text_data_init(ac);
		info->finalize = jaw_text_data_finalize;
		g_hash_table_insert(jaw_impl->ifaceTable, (gpointer)INTERFACE_TEXT, (gpointer)info);
	}

	if (tflag & INTERFACE_EDITABLE_TEXT) {
		JawInterfaceInfo *info = g_new(JawInterfaceInfo, 1);
		info->data = jaw_editable_text_data_init(ac);
		info->finalize = jaw_editable_text_data_finalize;
		g_hash_table_insert(jaw_impl->ifaceTable, (gpointer)INTERFACE_EDITABLE_TEXT, (gpointer)info);
	}

	if (tflag & INTERFACE_HYPERTEXT) {
		JawInterfaceInfo *info = g_new(JawInterfaceInfo, 1);
		info->data = jaw_hypertext_data_init(ac);
		info->finalize = jaw_hypertext_data_finalize;
		g_hash_table_insert(jaw_impl->ifaceTable, (gpointer)INTERFACE_HYPERTEXT, (gpointer)info);
	}

	if (tflag & INTERFACE_IMAGE) {
		JawInterfaceInfo *info = g_new(JawInterfaceInfo, 1);
		info->data = jaw_image_data_init(ac);
		info->finalize = jaw_image_data_finalize;
		g_hash_table_insert(jaw_impl->ifaceTable, (gpointer)INTERFACE_IMAGE, (gpointer)info);
	}

	if (tflag & INTERFACE_SELECTION) {
		JawInterfaceInfo *info = g_new(JawInterfaceInfo, 1);
		info->data = jaw_selection_data_init(ac);
		info->finalize = jaw_selection_data_finalize;
		g_hash_table_insert(jaw_impl->ifaceTable, (gpointer)INTERFACE_SELECTION, (gpointer)info);
	}

	if (tflag & INTERFACE_VALUE) {
		JawInterfaceInfo *info = g_new(JawInterfaceInfo, 1);
		info->data = jaw_value_data_init(ac);
		info->finalize = jaw_value_data_finalize;
		g_hash_table_insert(jaw_impl->ifaceTable, (gpointer)INTERFACE_VALUE, (gpointer)info);
	}

	if (tflag & INTERFACE_TABLE) {
		JawInterfaceInfo *info = g_new(JawInterfaceInfo, 1);
		info->data = jaw_table_data_init(ac);
		info->finalize = jaw_table_data_finalize;
		g_hash_table_insert(jaw_impl->ifaceTable, (gpointer)INTERFACE_TABLE, (gpointer)info);
	}
}

JawImpl*
jaw_impl_get_instance (JNIEnv *jniEnv, jobject ac)
{
	JawImpl *jaw_impl;

	g_mutex_lock(objectTableMutex);
	if (objectTable == NULL) {
		objectTable = g_hash_table_new ( NULL, NULL );
	}
	g_mutex_unlock(objectTableMutex);
	
	jaw_impl = object_table_lookup( jniEnv, ac );

	if (jaw_impl == NULL) {
		jobject global_ac = (*jniEnv)->NewGlobalRef(jniEnv, ac);
		guint tflag = jaw_util_get_tflag_from_jobj(jniEnv, global_ac);
		jaw_impl = g_object_new( JAW_TYPE_IMPL(tflag), NULL );
		JawObject *jaw_obj = JAW_OBJECT(jaw_impl);
		jaw_obj->acc_context = global_ac;
		jaw_obj->storedData = g_hash_table_new(g_str_hash, g_str_equal);
		aggregate_interface(jniEnv, jaw_obj, tflag);

		atk_object_initialize( ATK_OBJECT(jaw_impl), NULL );

		object_table_insert( jniEnv, global_ac, jaw_impl);
	}

	return jaw_impl;
}

JawImpl*
jaw_impl_find_instance (JNIEnv *jniEnv, jobject ac)
{
	JawImpl *jaw_impl;

	g_mutex_lock(objectTableMutex);
	if (objectTable == NULL) {
		g_mutex_unlock(objectTableMutex);
		return NULL;
	}
	g_mutex_unlock(objectTableMutex);

	jaw_impl = object_table_lookup( jniEnv, ac );

	return jaw_impl;
}

static void
jaw_impl_class_intern_init (gpointer klass)
{
	if (jaw_impl_parent_class == NULL)
		jaw_impl_parent_class = g_type_class_peek_parent (klass);
	
	jaw_impl_class_init ((JawImplClass*) klass);
}

GType
jaw_impl_get_type (guint tflag)
{
	GType type;

	static const GInterfaceInfo atk_action_info =
	{
		(GInterfaceInitFunc) jaw_action_interface_init,
		(GInterfaceFinalizeFunc) NULL,
		NULL
	};

	static const GInterfaceInfo atk_component_info =
	{
		(GInterfaceInitFunc) jaw_component_interface_init,
		(GInterfaceFinalizeFunc) NULL,
		NULL
	};

	static const GInterfaceInfo atk_text_info =
	{
		(GInterfaceInitFunc) jaw_text_interface_init,
		(GInterfaceFinalizeFunc) NULL,
		NULL
	};

	static const GInterfaceInfo atk_editable_text_info = 
	{
		(GInterfaceInitFunc) jaw_editable_text_interface_init,
		(GInterfaceFinalizeFunc) NULL,
		NULL
	};

	static const GInterfaceInfo atk_hypertext_info =
	{
		(GInterfaceInitFunc) jaw_hypertext_interface_init,
		(GInterfaceFinalizeFunc) NULL,
		NULL
	};

	static const GInterfaceInfo atk_image_info =
	{
		(GInterfaceInitFunc) jaw_image_interface_init,
		(GInterfaceFinalizeFunc) NULL,
		NULL
	};

	static const GInterfaceInfo atk_selection_info =
	{
		(GInterfaceInitFunc) jaw_selection_interface_init,
		(GInterfaceFinalizeFunc) NULL,
		NULL
	};

	static const GInterfaceInfo atk_value_info =
	{
		(GInterfaceInitFunc) jaw_value_interface_init,
		(GInterfaceFinalizeFunc) NULL,
		NULL
	};

	static const GInterfaceInfo atk_table_info =
	{
		(GInterfaceInitFunc) jaw_table_interface_init,
		(GInterfaceFinalizeFunc) NULL,
		NULL
	};

	if (typeTable == NULL) {
		typeTable = g_hash_table_new( NULL, NULL );
	}
	
	type = (GType)g_hash_table_lookup(typeTable, (gpointer)tflag);
	if (type == 0) {
		GTypeInfo tinfo = {
			sizeof(JawImplClass),
			(GBaseInitFunc) NULL, /* base init */
			(GBaseFinalizeFunc) NULL, /* base finalize */
			(GClassInitFunc) jaw_impl_class_intern_init, /*class init */
			(GClassFinalizeFunc) NULL, /* class finalize */
			NULL, /* class data */
			sizeof(JawImpl), /* instance size */
			0, /* nb preallocs */
			(GInstanceInitFunc) NULL, /* instance init */
			NULL /* value table */
		};

		gchar className[20];
		g_sprintf(className, "JawImpl_%d", tflag);

		type = g_type_register_static(JAW_TYPE_OBJECT,
				className, &tinfo, 0);

		if (tflag & INTERFACE_ACTION) {
			g_type_add_interface_static (type, ATK_TYPE_ACTION,
						&atk_action_info);
		}

		if (tflag & INTERFACE_COMPONENT) {
			g_type_add_interface_static (type, ATK_TYPE_COMPONENT,
						&atk_component_info);
		}

		if (tflag & INTERFACE_TEXT) {
			g_type_add_interface_static (type, ATK_TYPE_TEXT,
						&atk_text_info);
		}

		if (tflag & INTERFACE_EDITABLE_TEXT) {
			g_type_add_interface_static (type, ATK_TYPE_EDITABLE_TEXT,
						&atk_editable_text_info);
		}

		if (tflag & INTERFACE_HYPERTEXT) {
			g_type_add_interface_static (type, ATK_TYPE_HYPERTEXT,
						&atk_hypertext_info);
		}

		if (tflag & INTERFACE_IMAGE) {
			g_type_add_interface_static (type, ATK_TYPE_IMAGE,
						&atk_image_info);
		}

		if (tflag & INTERFACE_SELECTION) {
			g_type_add_interface_static (type, ATK_TYPE_SELECTION,
						&atk_selection_info);
		}

		if (tflag & INTERFACE_VALUE) {
			g_type_add_interface_static (type, ATK_TYPE_VALUE,
						&atk_value_info);
		}

		if (tflag & INTERFACE_TABLE) {
			g_type_add_interface_static (type, ATK_TYPE_TABLE,
						&atk_table_info);
		}

		g_hash_table_insert(typeTable, (gpointer)tflag, (gpointer)type);
	}

	return type;
}

static void
jaw_impl_class_init(JawImplClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->dispose = jaw_impl_dispose;
	gobject_class->finalize = jaw_impl_finalize;

	AtkObjectClass *atk_class = ATK_OBJECT_CLASS (klass);
	atk_class->initialize = jaw_impl_initialize;
	atk_class->get_parent = jaw_impl_get_parent;
	atk_class->ref_child = jaw_impl_ref_child;
	atk_class->ref_relation_set = jaw_impl_ref_relation_set;

	JawObjectClass *jaw_class = JAW_OBJECT_CLASS (klass);
	jaw_class->get_interface_data = jaw_impl_get_interface_data;
}
/*
static void
jaw_impl_init(JawImpl *impl)
{
	jaw_impl->ifaceTable = g_hash_table_new(NULL, NULL);
}
*/
static void
jaw_impl_dispose(GObject *gobject)
{
	/* Chain up to parent's dispose */
	G_OBJECT_CLASS(jaw_impl_parent_class)->dispose(gobject);
}

static void
jaw_impl_finalize(GObject *gobject)
{
	JawObject *jaw_obj = JAW_OBJECT(gobject);
	jobject global_ac = jaw_obj->acc_context;

	JawImpl *jaw_impl = (JawImpl*)jaw_obj;

	JNIEnv *jniEnv = jaw_util_get_jni_env();
	object_table_remove( jniEnv, global_ac );

	(*jniEnv)->DeleteGlobalRef(jniEnv, global_ac);
	jaw_obj->acc_context = NULL;

	/* Interface finalize */
	GHashTableIter iter;
	gpointer key;
	gpointer value;

	g_hash_table_iter_init(&iter, jaw_impl->ifaceTable);
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		JawInterfaceInfo *info = (JawInterfaceInfo*)value;
		info->finalize(info->data);

		g_free(info);

		g_hash_table_iter_remove(&iter);
	}
	g_hash_table_unref(jaw_impl->ifaceTable);

	g_hash_table_destroy(jaw_obj->storedData);

	/* Chain up to parent's finalize */
	G_OBJECT_CLASS(jaw_impl_parent_class)->finalize(gobject);
}

static gpointer
jaw_impl_get_interface_data (JawObject *jaw_obj, guint iface)
{
	JawImpl *jaw_impl = (JawImpl*)jaw_obj;

	if (jaw_impl == NULL || jaw_impl->ifaceTable == NULL) {
		return NULL;
	}

	JawInterfaceInfo *info = g_hash_table_lookup(jaw_impl->ifaceTable, (gpointer)iface);

	if (info) {
		return info->data;
	}

	return NULL;
}

static void
jaw_impl_initialize (AtkObject *atk_obj, gpointer data)
{
	ATK_OBJECT_CLASS(jaw_impl_parent_class)->initialize(atk_obj, data);

	JawObject *jaw_obj = JAW_OBJECT(atk_obj);
	jobject ac = jaw_obj->acc_context;
	JNIEnv *jniEnv = jaw_util_get_jni_env();

	jclass classAtkWrapper = (*jniEnv)->FindClass(jniEnv, "org/GNOME/Accessibility/AtkWrapper");
	jmethodID jmid = (*jniEnv)->GetStaticMethodID(jniEnv, classAtkWrapper, "registerPropertyChangeListener", "(Ljavax/accessibility/AccessibleContext;)V");
	(*jniEnv)->CallStaticVoidMethod(jniEnv, classAtkWrapper, jmid, ac);
}

static AtkObject*
jaw_impl_get_parent (AtkObject *atk_obj)
{
	if (jaw_toplevel_get_child_index(JAW_TOPLEVEL(atk_get_root()), atk_obj) != -1) {
		return ATK_OBJECT(atk_get_root());
	}

	JawObject *jaw_obj = JAW_OBJECT(atk_obj);
	jobject ac = jaw_obj->acc_context;
	JNIEnv *jniEnv = jaw_util_get_jni_env();

	jclass classAccessibleContext = (*jniEnv)->FindClass( jniEnv, "javax/accessibility/AccessibleContext" );
	jmethodID jmid = (*jniEnv)->GetMethodID( jniEnv, classAccessibleContext, "getAccessibleParent", "()Ljavax/accessibility/Accessible;");
	jobject jparent = (*jniEnv)->CallObjectMethod( jniEnv, ac, jmid );
	if (jparent != NULL ) {
		jclass classAccessible = (*jniEnv)->FindClass( jniEnv, "javax/accessibility/Accessible" );
		jmid = (*jniEnv)->GetMethodID( jniEnv, classAccessible, "getAccessibleContext", "()Ljavax/accessibility/AccessibleContext;");
		jobject parent_ac = (*jniEnv)->CallObjectMethod( jniEnv, jparent, jmid );

		AtkObject *obj = (AtkObject*) object_table_lookup( jniEnv, parent_ac );
		if (obj != NULL ) {
			return obj;
		}
	}

	return ATK_OBJECT(atk_get_root());
}

static AtkObject*
jaw_impl_ref_child (AtkObject *atk_obj,
			gint i)
{
	JawObject *jaw_obj = JAW_OBJECT(atk_obj);
	jobject ac = jaw_obj->acc_context;
	JNIEnv *jniEnv = jaw_util_get_jni_env();

	jclass classAccessibleContext = (*jniEnv)->FindClass( jniEnv, "javax/accessibility/AccessibleContext" );
	jmethodID jmid = (*jniEnv)->GetMethodID( jniEnv, classAccessibleContext, "getAccessibleChild", "(I)Ljavax/accessibility/Accessible;" );
	jobject jchild = (*jniEnv)->CallObjectMethod( jniEnv, ac, jmid, i );
	if (jchild == NULL) {
		return NULL;
	}

	jclass classAccessible = (*jniEnv)->FindClass( jniEnv, "javax/accessibility/Accessible" );
	jmid = (*jniEnv)->GetMethodID( jniEnv, classAccessible, "getAccessibleContext", "()Ljavax/accessibility/AccessibleContext;" );
	jobject child_ac = (*jniEnv)->CallObjectMethod( jniEnv, jchild, jmid );

	AtkObject *obj = (AtkObject*) jaw_impl_get_instance( jniEnv, child_ac );
	g_object_ref (G_OBJECT(obj));

	return obj;
}

static jstring
get_java_relation_key_constant (JNIEnv *jniEnv,
			const gchar* strKey)
{
	jclass classAccessibleRelation = (*jniEnv)->FindClass(jniEnv, "javax/accessibility/AccessibleRelation");
	jfieldID jfid = (*jniEnv)->GetStaticFieldID(jniEnv, classAccessibleRelation, strKey, "Ljava/lang/String;");
	jstring jkey = (*jniEnv)->GetStaticObjectField(jniEnv, classAccessibleRelation, jfid);

	return jkey;
}

static gboolean
is_java_relation_key (JNIEnv *jniEnv,
			jstring jKey,
			const gchar* strKey)
{
	jstring jConstKey = get_java_relation_key_constant (jniEnv, strKey);

	if ( (*jniEnv)->IsSameObject(jniEnv, jKey, jConstKey) ) {
		return TRUE;
	} else {
		return FALSE;
	}
}

static AtkRelationType
get_atk_relation_type_from_java_key (JNIEnv *jniEnv,
				jstring jrel_key)
{
	if ( is_java_relation_key(jniEnv, jrel_key, "CHILD_NODE_OF") ) {
		return ATK_RELATION_NODE_CHILD_OF;
	}
	if ( is_java_relation_key(jniEnv, jrel_key, "CONTROLLED_BY") ) {
		return ATK_RELATION_CONTROLLED_BY;
	}
	if ( is_java_relation_key(jniEnv, jrel_key, "CONTROLLER_FOR") ) {
		return ATK_RELATION_CONTROLLER_FOR;
	}
	if ( is_java_relation_key(jniEnv, jrel_key, "EMBEDDED_BY") ) {
		return ATK_RELATION_EMBEDDED_BY;
	}
	if ( is_java_relation_key(jniEnv, jrel_key, "EMBEDS") ) {
		return ATK_RELATION_EMBEDS;
	}
	if ( is_java_relation_key(jniEnv, jrel_key, "FLOWS_FROM") ) {
		return ATK_RELATION_FLOWS_FROM;
	}
	if ( is_java_relation_key(jniEnv, jrel_key, "FLOWS_TO") ) {
		return ATK_RELATION_FLOWS_TO;
	}
	if ( is_java_relation_key(jniEnv, jrel_key, "LABEL_FOR") ) {
		return ATK_RELATION_LABEL_FOR;
	}
	if ( is_java_relation_key(jniEnv, jrel_key, "LABELED_BY") ) {
		return ATK_RELATION_LABELLED_BY;
	}
	if ( is_java_relation_key(jniEnv, jrel_key, "MEMBER_OF") ) {
		return ATK_RELATION_MEMBER_OF;
	}
	if ( is_java_relation_key(jniEnv, jrel_key, "PARENT_WINDOW_OF") ) {
		return ATK_RELATION_PARENT_WINDOW_OF;
	}
	if ( is_java_relation_key(jniEnv, jrel_key, "SUBWINDOW_OF") ) {
		return ATK_RELATION_SUBWINDOW_OF;
	}

	return ATK_RELATION_NULL;
}

static AtkRelationSet*
jaw_impl_ref_relation_set (AtkObject *atk_obj)
{
	if (atk_obj->relation_set) {
		g_object_unref(G_OBJECT(atk_obj->relation_set));
	}

	atk_obj->relation_set = atk_relation_set_new();

	JawObject *jaw_obj = JAW_OBJECT(atk_obj);
	jobject ac = jaw_obj->acc_context;
	JNIEnv *jniEnv = jaw_util_get_jni_env();

	jclass classAccessibleContext = (*jniEnv)->FindClass( jniEnv, "javax/accessibility/AccessibleContext" );
	jmethodID jmid = (*jniEnv)->GetMethodID( jniEnv, classAccessibleContext, "getAccessibleRelationSet", "()Ljavax/accessibility/AccessibleRelationSet;" );
	jobject jrel_set = (*jniEnv)->CallObjectMethod( jniEnv, ac, jmid );

	jclass classAccessibleRelationSet = (*jniEnv)->FindClass( jniEnv, "javax/accessibility/AccessibleRelationSet" );
	jmid = (*jniEnv)->GetMethodID(jniEnv, classAccessibleRelationSet, "toArray", "()[Ljavax/accessibility/AccessibleRelation;");
	jobjectArray jrel_arr = (*jniEnv)->CallObjectMethod(jniEnv, jrel_set, jmid);
	jsize jarr_size = (*jniEnv)->GetArrayLength(jniEnv, jrel_arr);

	jsize i;
	for (i = 0; i < jarr_size; i++) {
		jobject jrel = (*jniEnv)->GetObjectArrayElement(jniEnv, jrel_arr, i);
		jclass classAccessibleRelation = (*jniEnv)->FindClass( jniEnv, "javax/accessibility/AccessibleRelation" );
		jmid = (*jniEnv)->GetMethodID( jniEnv, classAccessibleRelation, "getKey", "()Ljava/lang/String;" );
		jstring jrel_key = (*jniEnv)->CallObjectMethod( jniEnv, jrel, jmid );

		AtkRelationType rel_type = get_atk_relation_type_from_java_key(jniEnv, jrel_key);

		jmid = (*jniEnv)->GetMethodID( jniEnv, classAccessibleRelation, "getTarget", "()[Ljava/lang/Object;" );
		jobjectArray jtarget_arr = (*jniEnv)->CallObjectMethod( jniEnv, jrel, jmid );
		jsize jtarget_size = (*jniEnv)->GetArrayLength(jniEnv, jtarget_arr);

		jsize j;
		for (j = 0; j < jtarget_size; j++) {
			jobject jtarget = (*jniEnv)->GetObjectArrayElement(jniEnv, jtarget_arr, j);
			jclass classAccessible = (*jniEnv)->FindClass( jniEnv, "javax/accessibility/Accessible" );
			if ((*jniEnv)->IsInstanceOf(jniEnv, jtarget, classAccessible)) {
				jmid = (*jniEnv)->GetMethodID(jniEnv, classAccessible, "getAccessibleContext", "()Ljavax/accessibility/AccessibleContext;");
				jobject target_ac = (*jniEnv)->CallObjectMethod( jniEnv, jtarget, jmid );

				JawImpl *target_obj = jaw_impl_get_instance( jniEnv, target_ac );
				atk_object_add_relationship(atk_obj, rel_type, target_obj);
			}
		}
	}

	g_object_ref (atk_obj->relation_set);

	return atk_obj->relation_set;
}

