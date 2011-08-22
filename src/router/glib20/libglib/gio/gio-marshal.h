
#ifndef ___gio_marshal_MARSHAL_H__
#define ___gio_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:STRING,STRING,STRING,FLAGS (./gio-marshal.list:1) */
G_GNUC_INTERNAL void _gio_marshal_VOID__STRING_STRING_STRING_FLAGS (GClosure     *closure,
                                                                    GValue       *return_value,
                                                                    guint         n_param_values,
                                                                    const GValue *param_values,
                                                                    gpointer      invocation_hint,
                                                                    gpointer      marshal_data);

/* VOID:STRING,BOXED (./gio-marshal.list:2) */
G_GNUC_INTERNAL void _gio_marshal_VOID__STRING_BOXED (GClosure     *closure,
                                                      GValue       *return_value,
                                                      guint         n_param_values,
                                                      const GValue *param_values,
                                                      gpointer      invocation_hint,
                                                      gpointer      marshal_data);

/* VOID:STRING,VARIANT (./gio-marshal.list:3) */
G_GNUC_INTERNAL void _gio_marshal_VOID__STRING_VARIANT (GClosure     *closure,
                                                        GValue       *return_value,
                                                        guint         n_param_values,
                                                        const GValue *param_values,
                                                        gpointer      invocation_hint,
                                                        gpointer      marshal_data);

/* VOID:BOOLEAN,POINTER (./gio-marshal.list:4) */
G_GNUC_INTERNAL void _gio_marshal_VOID__BOOLEAN_POINTER (GClosure     *closure,
                                                         GValue       *return_value,
                                                         guint         n_param_values,
                                                         const GValue *param_values,
                                                         gpointer      invocation_hint,
                                                         gpointer      marshal_data);

/* VOID:OBJECT,OBJECT,ENUM (./gio-marshal.list:5) */
G_GNUC_INTERNAL void _gio_marshal_VOID__OBJECT_OBJECT_ENUM (GClosure     *closure,
                                                            GValue       *return_value,
                                                            guint         n_param_values,
                                                            const GValue *param_values,
                                                            gpointer      invocation_hint,
                                                            gpointer      marshal_data);

/* BOOLEAN:OBJECT,OBJECT (./gio-marshal.list:6) */
G_GNUC_INTERNAL void _gio_marshal_BOOLEAN__OBJECT_OBJECT (GClosure     *closure,
                                                          GValue       *return_value,
                                                          guint         n_param_values,
                                                          const GValue *param_values,
                                                          gpointer      invocation_hint,
                                                          gpointer      marshal_data);

/* VOID:STRING,BOXED,BOXED (./gio-marshal.list:7) */
G_GNUC_INTERNAL void _gio_marshal_VOID__STRING_BOXED_BOXED (GClosure     *closure,
                                                            GValue       *return_value,
                                                            guint         n_param_values,
                                                            const GValue *param_values,
                                                            gpointer      invocation_hint,
                                                            gpointer      marshal_data);

/* BOOL:POINTER,INT (./gio-marshal.list:8) */
G_GNUC_INTERNAL void _gio_marshal_BOOLEAN__POINTER_INT (GClosure     *closure,
                                                        GValue       *return_value,
                                                        guint         n_param_values,
                                                        const GValue *param_values,
                                                        gpointer      invocation_hint,
                                                        gpointer      marshal_data);
#define _gio_marshal_BOOL__POINTER_INT	_gio_marshal_BOOLEAN__POINTER_INT

/* BOOL:UINT (./gio-marshal.list:9) */
G_GNUC_INTERNAL void _gio_marshal_BOOLEAN__UINT (GClosure     *closure,
                                                 GValue       *return_value,
                                                 guint         n_param_values,
                                                 const GValue *param_values,
                                                 gpointer      invocation_hint,
                                                 gpointer      marshal_data);
#define _gio_marshal_BOOL__UINT	_gio_marshal_BOOLEAN__UINT

/* BOOL:VARIANT (./gio-marshal.list:10) */
G_GNUC_INTERNAL void _gio_marshal_BOOLEAN__VARIANT (GClosure     *closure,
                                                    GValue       *return_value,
                                                    guint         n_param_values,
                                                    const GValue *param_values,
                                                    gpointer      invocation_hint,
                                                    gpointer      marshal_data);
#define _gio_marshal_BOOL__VARIANT	_gio_marshal_BOOLEAN__VARIANT

/* BOOL:VOID (./gio-marshal.list:11) */
G_GNUC_INTERNAL void _gio_marshal_BOOLEAN__VOID (GClosure     *closure,
                                                 GValue       *return_value,
                                                 guint         n_param_values,
                                                 const GValue *param_values,
                                                 gpointer      invocation_hint,
                                                 gpointer      marshal_data);
#define _gio_marshal_BOOL__VOID	_gio_marshal_BOOLEAN__VOID

/* VOID:STRING,STRING,BOXED (./gio-marshal.list:12) */
G_GNUC_INTERNAL void _gio_marshal_VOID__STRING_STRING_BOXED (GClosure     *closure,
                                                             GValue       *return_value,
                                                             guint         n_param_values,
                                                             const GValue *param_values,
                                                             gpointer      invocation_hint,
                                                             gpointer      marshal_data);

/* VOID:BOOL,BOXED (./gio-marshal.list:13) */
G_GNUC_INTERNAL void _gio_marshal_VOID__BOOLEAN_BOXED (GClosure     *closure,
                                                       GValue       *return_value,
                                                       guint         n_param_values,
                                                       const GValue *param_values,
                                                       gpointer      invocation_hint,
                                                       gpointer      marshal_data);
#define _gio_marshal_VOID__BOOL_BOXED	_gio_marshal_VOID__BOOLEAN_BOXED

/* VOID:VARIANT,VARIANT (./gio-marshal.list:14) */
G_GNUC_INTERNAL void _gio_marshal_VOID__VARIANT_VARIANT (GClosure     *closure,
                                                         GValue       *return_value,
                                                         guint         n_param_values,
                                                         const GValue *param_values,
                                                         gpointer      invocation_hint,
                                                         gpointer      marshal_data);

/* VOID:INT (./gio-marshal.list:15) */
#define _gio_marshal_VOID__INT	g_cclosure_marshal_VOID__INT

/* VOID:STRING,INT (./gio-marshal.list:16) */
G_GNUC_INTERNAL void _gio_marshal_VOID__STRING_INT (GClosure     *closure,
                                                    GValue       *return_value,
                                                    guint         n_param_values,
                                                    const GValue *param_values,
                                                    gpointer      invocation_hint,
                                                    gpointer      marshal_data);

/* VOID:STRING,UINT (./gio-marshal.list:17) */
G_GNUC_INTERNAL void _gio_marshal_VOID__STRING_UINT (GClosure     *closure,
                                                     GValue       *return_value,
                                                     guint         n_param_values,
                                                     const GValue *param_values,
                                                     gpointer      invocation_hint,
                                                     gpointer      marshal_data);

/* VOID:BOXED,BOXED (./gio-marshal.list:18) */
G_GNUC_INTERNAL void _gio_marshal_VOID__BOXED_BOXED (GClosure     *closure,
                                                     GValue       *return_value,
                                                     guint         n_param_values,
                                                     const GValue *param_values,
                                                     gpointer      invocation_hint,
                                                     gpointer      marshal_data);

/* VOID:VARIANT,BOXED (./gio-marshal.list:19) */
G_GNUC_INTERNAL void _gio_marshal_VOID__VARIANT_BOXED (GClosure     *closure,
                                                       GValue       *return_value,
                                                       guint         n_param_values,
                                                       const GValue *param_values,
                                                       gpointer      invocation_hint,
                                                       gpointer      marshal_data);

/* VOID:STRING,STRING,VARIANT (./gio-marshal.list:20) */
G_GNUC_INTERNAL void _gio_marshal_VOID__STRING_STRING_VARIANT (GClosure     *closure,
                                                               GValue       *return_value,
                                                               guint         n_param_values,
                                                               const GValue *param_values,
                                                               gpointer      invocation_hint,
                                                               gpointer      marshal_data);

/* VOID:STRING (./gio-marshal.list:21) */
#define _gio_marshal_VOID__STRING	g_cclosure_marshal_VOID__STRING

/* VOID:STRING,STRING (./gio-marshal.list:22) */
G_GNUC_INTERNAL void _gio_marshal_VOID__STRING_STRING (GClosure     *closure,
                                                       GValue       *return_value,
                                                       guint         n_param_values,
                                                       const GValue *param_values,
                                                       gpointer      invocation_hint,
                                                       gpointer      marshal_data);

/* VOID:STRING,BOOLEAN (./gio-marshal.list:23) */
G_GNUC_INTERNAL void _gio_marshal_VOID__STRING_BOOLEAN (GClosure     *closure,
                                                        GValue       *return_value,
                                                        guint         n_param_values,
                                                        const GValue *param_values,
                                                        gpointer      invocation_hint,
                                                        gpointer      marshal_data);

/* VOID:POINTER,INT,STRING (./gio-marshal.list:24) */
G_GNUC_INTERNAL void _gio_marshal_VOID__POINTER_INT_STRING (GClosure     *closure,
                                                            GValue       *return_value,
                                                            guint         n_param_values,
                                                            const GValue *param_values,
                                                            gpointer      invocation_hint,
                                                            gpointer      marshal_data);

/* BOOLEAN:OBJECT (./gio-marshal.list:25) */
G_GNUC_INTERNAL void _gio_marshal_BOOLEAN__OBJECT (GClosure     *closure,
                                                   GValue       *return_value,
                                                   guint         n_param_values,
                                                   const GValue *param_values,
                                                   gpointer      invocation_hint,
                                                   gpointer      marshal_data);

/* INT:OBJECT (./gio-marshal.list:26) */
G_GNUC_INTERNAL void _gio_marshal_INT__OBJECT (GClosure     *closure,
                                               GValue       *return_value,
                                               guint         n_param_values,
                                               const GValue *param_values,
                                               gpointer      invocation_hint,
                                               gpointer      marshal_data);

/* VOID:INT64 (./gio-marshal.list:27) */
G_GNUC_INTERNAL void _gio_marshal_VOID__INT64 (GClosure     *closure,
                                               GValue       *return_value,
                                               guint         n_param_values,
                                               const GValue *param_values,
                                               gpointer      invocation_hint,
                                               gpointer      marshal_data);

/* VOID:UINT64 (./gio-marshal.list:28) */
G_GNUC_INTERNAL void _gio_marshal_VOID__UINT64 (GClosure     *closure,
                                                GValue       *return_value,
                                                guint         n_param_values,
                                                const GValue *param_values,
                                                gpointer      invocation_hint,
                                                gpointer      marshal_data);

/* BOOLEAN:FLAGS (./gio-marshal.list:29) */
G_GNUC_INTERNAL void _gio_marshal_BOOLEAN__FLAGS (GClosure     *closure,
                                                  GValue       *return_value,
                                                  guint         n_param_values,
                                                  const GValue *param_values,
                                                  gpointer      invocation_hint,
                                                  gpointer      marshal_data);

/* BOOLEAN:OBJECT,FLAGS (./gio-marshal.list:30) */
G_GNUC_INTERNAL void _gio_marshal_BOOLEAN__OBJECT_FLAGS (GClosure     *closure,
                                                         GValue       *return_value,
                                                         guint         n_param_values,
                                                         const GValue *param_values,
                                                         gpointer      invocation_hint,
                                                         gpointer      marshal_data);

/* OBJECT:VOID (./gio-marshal.list:31) */
G_GNUC_INTERNAL void _gio_marshal_OBJECT__VOID (GClosure     *closure,
                                                GValue       *return_value,
                                                guint         n_param_values,
                                                const GValue *param_values,
                                                gpointer      invocation_hint,
                                                gpointer      marshal_data);

G_END_DECLS

#endif /* ___gio_marshal_MARSHAL_H__ */

