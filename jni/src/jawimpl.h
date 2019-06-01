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

#ifndef _JAW_IMPL_H_
#define _JAW_IMPL_H_

#include <glib.h>
#include "jawobject.h"

G_BEGIN_DECLS

#define JAW_TYPE_IMPL(tf)		(jaw_impl_get_type (tf))
#define JAW_IMPL(tf, obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), JAW_TYPE_IMPL(tf), JawImpl))
#define JAW_IMPL_CLASS(tf, klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), JAW_TYPE_IMPL(tf), JawImplClass))
#define JAW_IS_IMPL(tf, obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), JAW_TYPE_IMPL(tf)))
#define JAW_IS_IMPL_CLASS(tf, klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), JAW_TYPE_IMPL(tf)))
#define JAW_IMPL_GET_CLASS(tf, obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), JAW_TYPE_IMPL(tf), JawImplClass))

#ifdef GPOINTER_TO_SIZE
  #define GPOINTER_TO_GTYPE(gpointer) (GPOINTER_TO_SIZE (gpointer))
#endif
#ifdef GSIZE_TO_POINTER
  #define GTYPE_TO_POINTER(gtype) (GSIZE_TO_POINTER(gtype))
#endif

typedef struct _JawImpl			JawImpl;
typedef struct _JawImplClass		JawImplClass;

struct _JawImpl
{
	JawObject parent;

	GHashTable *ifaceTable;
	gint hash_key;
};

JawImpl* jaw_impl_get_instance(JNIEnv*, jobject);
JawImpl* jaw_impl_find_instance(JNIEnv*, jobject);
GHashTable* jaw_impl_get_object_hash_table(void);
GMutex* jaw_impl_get_object_hash_table_mutex(void);
void object_table_gc(JNIEnv *jniEnv);

GType jaw_impl_get_type (guint);
AtkRelationType jaw_impl_get_atk_relation_type(JNIEnv *jniEnv, jstring jrel_key);

struct _JawImplClass
{
	JawObjectClass parent_class;
};

extern void jaw_action_interface_init(AtkActionIface*);
extern gpointer jaw_action_data_init(jobject);
extern void jaw_action_data_finalize(gpointer);

extern void jaw_component_interface_init(AtkComponentIface*);
extern gpointer jaw_component_data_init(jobject);
extern void jaw_component_data_finalize(gpointer);

extern void jaw_editable_text_interface_init(AtkEditableTextIface*);
extern gpointer jaw_editable_text_data_init(jobject);
extern void jaw_editable_text_data_finalize (gpointer);

extern void jaw_hypertext_interface_init(AtkHypertextIface*);
extern gpointer jaw_hypertext_data_init(jobject);
extern void jaw_hypertext_data_finalize(gpointer);

extern void jaw_image_interface_init(AtkImageIface*);
extern gpointer jaw_image_data_init(jobject);
extern void jaw_image_data_finalize(gpointer);

extern void jaw_selection_interface_init(AtkSelectionIface*);
extern gpointer jaw_selection_data_init(jobject);
extern void jaw_selection_data_finalize(gpointer);

extern void jaw_table_interface_init (AtkTableIface*);
extern gpointer jaw_table_data_init (jobject);
extern void jaw_table_data_finalize (gpointer);

extern void jaw_table_cell_interface_init (AtkTableCellIface*);
extern gpointer jaw_table_cell_data_init (jobject ac);
extern void jaw_table_cell_data_finalize (gpointer);

extern void jaw_text_interface_init(AtkTextIface*);
extern gpointer	jaw_text_data_init(jobject);
extern void jaw_text_data_finalize(gpointer);

extern void jaw_value_interface_init (AtkValueIface*);
extern gpointer jaw_value_data_init (jobject);
extern void jaw_value_data_finalize (gpointer);

G_END_DECLS

#endif

