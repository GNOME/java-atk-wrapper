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

#include <atk/atk.h>
#include <glib.h>
#include "jawobject.h"
#include "jawutil.h"
#include "jawimpl.h"
#include "jawtoplevel.h"

#ifdef __cplusplus
extern "C" {
#endif

static void jaw_object_class_init(JawObjectClass *klass);
static void jaw_object_init(JawObject *object);
static void jaw_object_initialize(AtkObject *jaw_obj, gpointer data);
static void jaw_object_dispose(GObject *gobject);
static void jaw_object_finalize(GObject *gobject);

/* AtkObject */
static const gchar* jaw_object_get_name(AtkObject *atk_obj);
static const gchar* jaw_object_get_description(AtkObject *atk_obj);

static gint jaw_object_get_n_children(AtkObject *atk_obj);

static gint jaw_object_get_index_in_parent(AtkObject *atk_obj);

static AtkRole jaw_object_get_role(AtkObject *atk_obj);
static AtkStateSet* jaw_object_ref_state_set(AtkObject *atk_obj);
static AtkObject* jaw_object_get_parent(AtkObject *obj);

static void jaw_object_set_name (AtkObject *atk_obj, const gchar *name);
static void jaw_object_set_description (AtkObject *atk_obj, const gchar *description);
static void jaw_object_set_parent(AtkObject *atk_obj, AtkObject *parent);
static void jaw_object_set_role (AtkObject *atk_obj, AtkRole role);
static const gchar *jaw_object_get_object_locale (AtkObject *atk_obj);
static AtkRelationSet *jaw_object_ref_relation_set (AtkObject *atk_obj);
static AtkObject *jaw_object_ref_child(AtkObject *atk_obj, gint i);

static gpointer parent_class = NULL;
static GHashTable *object_table = NULL;
static JawObject* jaw_object_table_lookup (JNIEnv *jniEnv, jobject ac);

enum {
  ACTIVATE,
  CREATE,
  DEACTIVATE,
  DESTROY,
  MAXIMIZE,
  MINIMIZE,
  MOVE,
  RESIZE,
  RESTORE,
  LAST_SIGNAL
};

static guint jaw_window_signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (JawObject, jaw_object, ATK_TYPE_OBJECT);

static guint
jaw_window_add_signal (const gchar *name, JawObjectClass *klass)
{
  return g_signal_new (name,
                       G_TYPE_FROM_CLASS(klass),
                       G_SIGNAL_RUN_LAST,
                       0,
                       (GSignalAccumulator) NULL, NULL,
                       g_cclosure_marshal_VOID__VOID,
                       G_TYPE_NONE,
                       0);
}

static void
jaw_object_class_init (JawObjectClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->dispose = jaw_object_dispose;
  gobject_class->finalize = jaw_object_finalize;

  AtkObjectClass *atk_class = ATK_OBJECT_CLASS (klass);
  parent_class = g_type_class_peek_parent (klass);

  atk_class->get_name = jaw_object_get_name;
  atk_class->set_name = jaw_object_set_name;
  atk_class->get_description = jaw_object_get_description;
  atk_class->set_description = jaw_object_set_description;
  atk_class->get_n_children = jaw_object_get_n_children;
  atk_class->get_index_in_parent = jaw_object_get_index_in_parent;
  atk_class->get_role = jaw_object_get_role;
  atk_class->get_parent = jaw_object_get_parent;
  atk_class->set_parent = jaw_object_set_parent;
  atk_class->set_role = jaw_object_set_role;
  atk_class->get_object_locale = jaw_object_get_object_locale;
  atk_class->ref_relation_set = jaw_object_ref_relation_set;
  atk_class->ref_child = jaw_object_ref_child;

  atk_class->ref_state_set = jaw_object_ref_state_set;
  atk_class->initialize = jaw_object_initialize;

  jaw_window_signals[ACTIVATE]    = jaw_window_add_signal ("activate", klass);
  jaw_window_signals[CREATE]      = jaw_window_add_signal ("create", klass);
  jaw_window_signals[DEACTIVATE]  = jaw_window_add_signal ("deactivate", klass);
  jaw_window_signals[DESTROY]     = jaw_window_add_signal ("destroy", klass);
  jaw_window_signals[MAXIMIZE]    = jaw_window_add_signal ("maximize", klass);
  jaw_window_signals[MINIMIZE]    = jaw_window_add_signal ("minimize", klass);
  jaw_window_signals[MOVE]        = jaw_window_add_signal ("move", klass);
  jaw_window_signals[RESIZE]      = jaw_window_add_signal ("resize", klass);
  jaw_window_signals[RESTORE]     = jaw_window_add_signal ("restore", klass);

  klass->get_interface_data = NULL;
}

static void
jaw_object_initialize(AtkObject *atk_obj, gpointer data)
{
  ATK_OBJECT_CLASS (jaw_object_parent_class)->initialize(atk_obj, data);
}

gpointer
jaw_object_get_interface_data (JawObject *jaw_obj, guint iface)
{
  JawObjectClass *klass = JAW_OBJECT_GET_CLASS(jaw_obj);
  if (klass->get_interface_data)
    return klass->get_interface_data(jaw_obj, iface);

  return NULL;
}

static void
jaw_object_init (JawObject *object)
{
  AtkObject *atk_obj = ATK_OBJECT(object);
  atk_obj->description = NULL;

  object->state_set = atk_state_set_new();
}

static void
jaw_object_dispose (GObject *gobject)
{
  /* Customized dispose code */

  /* Chain up to parent's dispose method */
  G_OBJECT_CLASS(jaw_object_parent_class)->dispose(gobject);
}

static void
jaw_object_finalize (GObject *gobject)
{
  /* Customized finalize code */
  JawObject *jaw_obj = JAW_OBJECT(gobject);
  AtkObject *atk_obj = ATK_OBJECT(gobject);
  JNIEnv *jniEnv = jaw_util_get_jni_env();

  if (atk_obj->name != NULL)
  {
    (*jniEnv)->ReleaseStringUTFChars(jniEnv, jaw_obj->jstrName, atk_obj->name);
    (*jniEnv)->DeleteGlobalRef(jniEnv, jaw_obj->jstrName);
    jaw_obj->jstrName = NULL;
    atk_obj->name = NULL;
  }

  if (atk_obj->description != NULL)
  {
    (*jniEnv)->ReleaseStringUTFChars(jniEnv,
                                     jaw_obj->jstrDescription,
                                     atk_obj->description);

    (*jniEnv)->DeleteGlobalRef(jniEnv, jaw_obj->jstrDescription);
    jaw_obj->jstrDescription = NULL;
    atk_obj->description = NULL;
  }

  if (G_OBJECT(jaw_obj->state_set) != NULL)
  {
    g_object_unref(G_OBJECT(jaw_obj->state_set));
    /* Chain up to parent's finalize method */
    G_OBJECT_CLASS(jaw_object_parent_class)->finalize(gobject);
  }
}

static AtkObject* jaw_object_get_parent(AtkObject *atk_obj)
{
  if (jaw_toplevel_get_child_index(JAW_TOPLEVEL(atk_get_root()), atk_obj) != -1)
  {
    return ATK_OBJECT(atk_get_root());
  }

  JawObject *jaw_obj = JAW_OBJECT(atk_obj);
  jobject ac = jaw_obj->acc_context;
  JNIEnv *jniEnv = jaw_util_get_jni_env();

  jclass classAccessibleContext = (*jniEnv)->FindClass(jniEnv,
                                                       "javax/accessibility/AccessibleContext" );
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAccessibleContext,
                                          "getAccessibleParent",
                                          "()Ljavax/accessibility/AccessibleContext;");
  jobject jparent = (*jniEnv)->CallObjectMethod( jniEnv, ac, jmid );
  if (jparent != NULL )
  {
    jclass classAccessible = (*jniEnv)->FindClass(jniEnv,
                                                  "javax/accessibility/Accessible" );
    jmid = (*jniEnv)->GetMethodID(jniEnv,
                                  classAccessible,
                                  "getAccessibleContext",
                                  "()Ljavax/accessibility/AccessibleContext;");
    jobject parent_ac = (*jniEnv)->CallObjectMethod(jniEnv, jparent, jmid);
    AtkObject *parent_obj = (AtkObject*) jaw_object_table_lookup( jniEnv, parent_ac );
    if (parent_obj != NULL )
       return parent_obj;
  }
  return ATK_OBJECT(atk_get_root());
}

static void
jaw_object_set_parent(AtkObject *atk_obj, AtkObject *parent)
{
  JawObject *jaw_obj = JAW_OBJECT(atk_obj);
  jobject ac = jaw_obj->acc_context;
  JNIEnv *jniEnv = jaw_util_get_jni_env();

  jclass classAccessibleContext = (*jniEnv)->FindClass(jniEnv,
                                                       "javax/accessibility/AccessibleContext" );
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAccessibleContext,
                                          "setAccessibleParent",
                                          "(Ljavax/accessibility/AccessibleContext;)");
  jobject jparent = (*jniEnv)->CallObjectMethod( jniEnv, ac, jmid );
  if (jparent != NULL )
  {
    jclass classAccessible = (*jniEnv)->FindClass(jniEnv,
                                                  "javax/accessibility/Accessible" );
    jmid = (*jniEnv)->GetMethodID(jniEnv,
                                  classAccessible,
                                  "getAccessibleContext",
                                  "()Ljavax/accessibility/AccessibleContext;");
    jobject parent_ac = (*jniEnv)->CallObjectMethod(jniEnv, jparent, jmid);
    AtkObject *parent_obj = (AtkObject*) jaw_object_table_lookup( jniEnv, parent_ac );
    if (parent_obj == NULL)
      return;
  }
}

static const gchar*
jaw_object_get_name (AtkObject *atk_obj)
{
  JawObject *jaw_obj = JAW_OBJECT(atk_obj);
  jobject ac = jaw_obj->acc_context;
  JNIEnv *jniEnv = jaw_util_get_jni_env();

  atk_obj->name = (gchar *)ATK_OBJECT_CLASS (parent_class)->get_name (atk_obj);

  if (atk_object_get_role(atk_obj) == ATK_ROLE_COMBO_BOX &&
      atk_object_get_n_accessible_children(atk_obj) == 1)
  {
    AtkSelection *selection = ATK_SELECTION(atk_obj);
    if (selection != NULL)
    {
      AtkObject *child = atk_selection_ref_selection(selection, 0);
      if (child != NULL)
      {
        return atk_object_get_name(child);
      }
    }
  }

  jclass classAccessibleContext = (*jniEnv)->FindClass(jniEnv,
                                                       "javax/accessibility/AccessibleContext" );
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAccessibleContext,
                                          "getAccessibleName",
                                          "()Ljava/lang/String;");
  jstring jstr = (*jniEnv)->CallObjectMethod( jniEnv, ac, jmid );

  if (atk_obj->name != NULL)
  {
    (*jniEnv)->ReleaseStringUTFChars(jniEnv, jaw_obj->jstrName, atk_obj->name);
    (*jniEnv)->DeleteGlobalRef(jniEnv, jaw_obj->jstrName);
  }

  if (jstr != NULL)
  {
    jaw_obj->jstrName = (*jniEnv)->NewGlobalRef(jniEnv, jstr);
    atk_obj->name = (gchar*)(*jniEnv)->GetStringUTFChars(jniEnv,
                                                         jaw_obj->jstrName,
                                                         NULL);
  }

  return atk_obj->name;
}

static void jaw_object_set_name (AtkObject *atk_obj, const gchar *name)
{
  JawObject *jaw_obj = JAW_OBJECT(atk_obj);
  jobject ac = jaw_obj->acc_context;
  JNIEnv *jniEnv = jaw_util_get_jni_env();

  atk_obj->name = (gchar *)ATK_OBJECT_CLASS (parent_class)->get_name (atk_obj);

  jclass classAccessibleContext = (*jniEnv)->FindClass(jniEnv,
                                                       "javax/accessibility/AccessibleContext" );
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAccessibleContext,
                                          "setAccessibleName",
                                          "(Ljava/lang/String;)");
  jstring jstr = (*jniEnv)->CallObjectMethod( jniEnv, ac, jmid );

  if (atk_obj->name != NULL)
  {
    (*jniEnv)->ReleaseStringUTFChars(jniEnv, jaw_obj->jstrName, atk_obj->name);
    (*jniEnv)->DeleteGlobalRef(jniEnv, jaw_obj->jstrName);
  }

  if (jstr != NULL)
  {
    jaw_obj->jstrName = (*jniEnv)->NewGlobalRef(jniEnv, jstr);
    atk_obj->name = (gchar*)(*jniEnv)->GetStringUTFChars(jniEnv,
                                                         jaw_obj->jstrName,
                                                         NULL);
  }
  if (jstr == NULL)
  {
    name = "";
    return;
  }
}

static const gchar*
jaw_object_get_description (AtkObject *atk_obj)
{
  JawObject *jaw_obj = JAW_OBJECT(atk_obj);
  jobject ac = jaw_obj->acc_context;
  JNIEnv *jniEnv = jaw_util_get_jni_env();

  jclass classAccessibleContext = (*jniEnv)->FindClass( jniEnv,
                                                       "javax/accessibility/AccessibleContext" );
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAccessibleContext,
                                          "getAccessibleDescription",
                                          "()Ljava/lang/String;");
  jstring jstr = (*jniEnv)->CallObjectMethod( jniEnv, ac, jmid );

  if (atk_obj->description != NULL)
  {
    (*jniEnv)->ReleaseStringUTFChars(jniEnv, jaw_obj->jstrDescription, atk_obj->description);
    (*jniEnv)->DeleteGlobalRef(jniEnv, jaw_obj->jstrDescription);
    atk_obj->description = NULL;
  }

  if (jstr != NULL)
  {
    jaw_obj->jstrDescription = (*jniEnv)->NewGlobalRef(jniEnv, jstr);
    atk_obj->description = (gchar*)(*jniEnv)->GetStringUTFChars(jniEnv,
                                                                jaw_obj->jstrDescription,
                                                                NULL);
  }

  return atk_obj->description;
}

static void jaw_object_set_description (AtkObject *atk_obj, const gchar *description)
{
  JawObject *jaw_obj = JAW_OBJECT(atk_obj);
  jobject ac = jaw_obj->acc_context;
  JNIEnv *jniEnv = jaw_util_get_jni_env();

  jclass classAccessibleContext = (*jniEnv)->FindClass( jniEnv,
                                                       "javax/accessibility/AccessibleContext" );
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAccessibleContext,
                                          "setAccessibleDescription",
                                          "(Ljava/lang/String;)");
  jstring jstr = (*jniEnv)->CallObjectMethod( jniEnv, ac, jmid );

  if (description != NULL)
  {
    (*jniEnv)->ReleaseStringUTFChars(jniEnv, jaw_obj->jstrDescription, description);
    (*jniEnv)->DeleteGlobalRef(jniEnv, jaw_obj->jstrDescription);
    description = NULL;
  }

  if (jstr != NULL)
  {
    jaw_obj->jstrDescription = (*jniEnv)->NewGlobalRef(jniEnv, jstr);
    description = (gchar*)(*jniEnv)->GetStringUTFChars(jniEnv,
                                                       jaw_obj->jstrDescription,
                                                       NULL);
  }
  if (jstr != NULL)
  {
    description = "";
  }
}


static gint
jaw_object_get_n_children (AtkObject *atk_obj)
{
  JawObject *jaw_obj = JAW_OBJECT(atk_obj);
  jobject ac = jaw_obj->acc_context;
  JNIEnv *jniEnv = jaw_util_get_jni_env();

  jclass classAccessibleContext = (*jniEnv)->FindClass(jniEnv,
                                                       "javax/accessibility/AccessibleContext" );
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAccessibleContext,
                                          "getAccessibleChildrenCount",
                                          "()I");
  jint count = (*jniEnv)->CallIntMethod( jniEnv, ac, jmid );

  return (gint)count;
}

static gint
jaw_object_get_index_in_parent (AtkObject *atk_obj)
{
  if (jaw_toplevel_get_child_index(JAW_TOPLEVEL(atk_get_root()), atk_obj) != -1)
  {
    return jaw_toplevel_get_child_index(JAW_TOPLEVEL(atk_get_root()), atk_obj);
  }

  JawObject *jaw_obj = JAW_OBJECT(atk_obj);
  jobject ac = jaw_obj->acc_context;
  JNIEnv *jniEnv = jaw_util_get_jni_env();

  jclass classAccessibleContext = (*jniEnv)->FindClass(jniEnv,
                                                       "javax/accessibility/AccessibleContext" );
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAccessibleContext,
                                          "getAccessibleIndexInParent",
                                          "()I");
  jint index = (*jniEnv)->CallIntMethod( jniEnv, ac, jmid );

  return (gint)index;
}

static AtkRole
jaw_object_get_role (AtkObject *atk_obj)
{
  JawObject *jaw_obj = JAW_OBJECT(atk_obj);
  atk_obj->role = jaw_util_get_atk_role_from_jobj(jaw_obj->acc_context);
  return atk_obj->role;
}

static void
jaw_object_set_role (AtkObject *atk_obj, AtkRole role)
{
  atk_obj->role = role;
  if (atk_obj != NULL && atk_obj->role)
    atk_object_set_role(atk_obj, atk_obj->role);
}

static AtkStateSet*
jaw_object_ref_state_set (AtkObject *atk_obj)
{
  JawObject *jaw_obj = JAW_OBJECT(atk_obj);
  AtkStateSet* state_set = jaw_obj->state_set;
  atk_state_set_clear_states( state_set );

  jobject ac = jaw_obj->acc_context;
  JNIEnv *jniEnv = jaw_util_get_jni_env();
  jclass classAccessibleContext = (*jniEnv)->FindClass(jniEnv,
                                                       "javax/accessibility/AccessibleContext" );
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAccessibleContext,
                                          "getAccessibleStateSet",
                                          "()Ljavax/accessibility/AccessibleStateSet;" );
  jobject jstate_set = (*jniEnv)->CallObjectMethod( jniEnv, ac, jmid );

  jclass classAccessibleStateSet = (*jniEnv)->FindClass(jniEnv,
                                                        "javax/accessibility/AccessibleStateSet" );
  jmid = (*jniEnv)->GetMethodID(jniEnv,
                                classAccessibleStateSet,
                                "toArray",
                                "()[Ljavax/accessibility/AccessibleState;");

  jobjectArray jstate_arr = (*jniEnv)->CallObjectMethod( jniEnv, jstate_set, jmid );

  jsize jarr_size = (*jniEnv)->GetArrayLength(jniEnv, jstate_arr);
  jsize i;
  for (i = 0; i < jarr_size; i++)
  {
    jobject jstate = (*jniEnv)->GetObjectArrayElement( jniEnv, jstate_arr, i );
    AtkStateType state_type = jaw_util_get_atk_state_type_from_java_state( jniEnv, jstate );
    atk_state_set_add_state( state_set, state_type );
    if (state_type == ATK_STATE_ENABLED)
    {
      atk_state_set_add_state( state_set, ATK_STATE_SENSITIVE );
    }
  }

  if (G_OBJECT(state_set) != NULL)
    g_object_ref(G_OBJECT(state_set));

  return state_set;
}

static const gchar *jaw_object_get_object_locale (AtkObject *atk_obj)
{
  JawObject *jaw_obj = JAW_OBJECT(atk_obj);
  jobject ac = jaw_obj->acc_context;
  JNIEnv *jniEnv = jaw_util_get_jni_env();

  jclass classAccessibleContext = (*jniEnv)->FindClass(jniEnv,
                                                       "javax/accessibility/AccessibleContext" );
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAccessibleContext,
                                          "getLocale",
                                          "()Ljavax/accessibility/AccessibleContext;");
  jobject locale = (*jniEnv)->CallObjectMethod( jniEnv, ac, jmid );
  JawImpl *target_obj = jaw_impl_get_instance(jniEnv, locale);
  if(target_obj == NULL)
    return NULL;

  return atk_object_get_object_locale((AtkObject*) target_obj);
}

static AtkRelationSet*
jaw_object_ref_relation_set (AtkObject *atk_obj)
{
  if (atk_obj->relation_set)
    g_object_unref(G_OBJECT(atk_obj->relation_set));
  atk_obj->relation_set = atk_relation_set_new();
  if(atk_obj == NULL)
    return NULL;

  JawObject *jaw_obj = JAW_OBJECT(atk_obj);
  jobject ac = jaw_obj->acc_context;
  JNIEnv *jniEnv = jaw_util_get_jni_env();

  jclass classAccessibleContext = (*jniEnv)->FindClass(jniEnv,
                                                       "javax/accessibility/AccessibleContext" );
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAccessibleContext,
                                          "getAccessibleRelationSet",
                                          "()Ljavax/accessibility/AccessibleRelationSet;" );
  jobject jrel_set = (*jniEnv)->CallObjectMethod( jniEnv, ac, jmid );

  jclass classAccessibleRelationSet = (*jniEnv)->FindClass( jniEnv,
                                                           "javax/accessibility/AccessibleRelationSet");
  jmid = (*jniEnv)->GetMethodID(jniEnv,
                                classAccessibleRelationSet,
                                "toArray",
                                "()[Ljavax/accessibility/AccessibleRelation;");
  jobjectArray jrel_arr = (*jniEnv)->CallObjectMethod(jniEnv, jrel_set, jmid);
  jsize jarr_size = (*jniEnv)->GetArrayLength(jniEnv, jrel_arr);

  jsize i;
  for (i = 0; i < jarr_size; i++)
  {
    jobject jrel = (*jniEnv)->GetObjectArrayElement(jniEnv, jrel_arr, i);
    jclass classAccessibleRelation = (*jniEnv)->FindClass(jniEnv,
                                                          "javax/accessibility/AccessibleRelation");
    jmid = (*jniEnv)->GetMethodID(jniEnv,
                                  classAccessibleRelation,
                                  "getKey",
                                  "()Ljava/lang/String;");
    jstring jrel_key = (*jniEnv)->CallObjectMethod( jniEnv, jrel, jmid );
    AtkRelationType rel_type = jaw_impl_get_atk_relation_type(jniEnv, jrel_key);

    jmid = (*jniEnv)->GetMethodID(jniEnv,
                                  classAccessibleRelation,
                                  "getTarget",
                                  "()[Ljava/lang/Object;");
    jobjectArray jtarget_arr = (*jniEnv)->CallObjectMethod(jniEnv, jrel, jmid);
    jsize jtarget_size = (*jniEnv)->GetArrayLength(jniEnv, jtarget_arr);

    jsize j;
    for (j = 0; j < jtarget_size; j++)
    {
      jobject jtarget = (*jniEnv)->GetObjectArrayElement(jniEnv, jtarget_arr, j);
      jclass classAccessible = (*jniEnv)->FindClass( jniEnv,
                                                    "javax/accessibility/Accessible");
      if ((*jniEnv)->IsInstanceOf(jniEnv, jtarget, classAccessible))
      {
        jmid = (*jniEnv)->GetMethodID(jniEnv,
                                      classAccessible,
                                      "getAccessibleContext",
                                      "()Ljavax/accessibility/AccessibleContext;");
        jobject target_ac = (*jniEnv)->CallObjectMethod(jniEnv, jtarget, jmid);

        JawImpl *target_obj = jaw_impl_get_instance(jniEnv, target_ac);
        if(target_obj == NULL)
          return NULL;
        atk_object_add_relationship(atk_obj, rel_type, ATK_OBJECT(target_obj));
      }
    }
  }
  if(atk_obj->relation_set == NULL)
    return NULL;
  if (G_OBJECT(atk_obj->relation_set) != NULL)
    g_object_ref (atk_obj->relation_set);

  return atk_obj->relation_set;
}

static AtkObject*
jaw_object_ref_child(AtkObject *atk_obj, gint i)
{
  JawObject *jaw_obj = JAW_OBJECT(atk_obj);
  jobject ac = jaw_obj->acc_context;
  JNIEnv *jniEnv = jaw_util_get_jni_env();

  jclass classAccessibleContext = (*jniEnv)->FindClass(jniEnv,
                                                       "javax/accessibility/AccessibleContext" );
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAccessibleContext,
                                          "getAccessibleChild",
                                          "(I)Ljavax/accessibility/Accessible;" );
  jobject jchild = (*jniEnv)->CallObjectMethod( jniEnv, ac, jmid, i );
  if (jchild == NULL)
  {
    return NULL;
  }

  jclass classAccessible = (*jniEnv)->FindClass( jniEnv, "javax/accessibility/Accessible" );
  jmid = (*jniEnv)->GetMethodID(jniEnv,
                                classAccessible,
                                "getAccessibleContext",
                                "()Ljavax/accessibility/AccessibleContext;" );
  jobject child_ac = (*jniEnv)->CallObjectMethod( jniEnv, jchild, jmid );

  AtkObject *obj = (AtkObject*) jaw_impl_get_instance( jniEnv, child_ac );
  if (G_OBJECT(obj) != NULL)
    g_object_ref(G_OBJECT(obj));

  return obj;
}

static JawObject*
jaw_object_table_lookup (JNIEnv *jniEnv, jobject ac)
{
  object_table = jaw_impl_get_object_hash_table();
  jclass classAccessibleContext = (*jniEnv)->FindClass( jniEnv,
                                                       "javax/accessibility/AccessibleContext" );
  jmethodID jmid = (*jniEnv)->GetMethodID(jniEnv,
                                          classAccessibleContext,
                                          "hashCode",
                                          "()I" );
  gint hash_key = (gint)(*jniEnv)->CallIntMethod( jniEnv, ac, jmid );
  gpointer value = NULL;
  if (object_table == NULL)
    return NULL;

  value = g_hash_table_lookup(object_table, GINT_TO_POINTER(hash_key));
  return (JawObject*)value;
}

#ifdef __cplusplus
}
#endif

