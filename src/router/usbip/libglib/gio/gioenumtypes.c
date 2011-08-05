
/* Generated data (by glib-mkenums) */

#include <gio.h>
#include "gioalias.h"

/* enumerations from "gioenums.h" */
GType
g_app_info_create_flags_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GFlagsValue values[] = {
        { G_APP_INFO_CREATE_NONE, "G_APP_INFO_CREATE_NONE", "none" },
        { G_APP_INFO_CREATE_NEEDS_TERMINAL, "G_APP_INFO_CREATE_NEEDS_TERMINAL", "needs-terminal" },
        { G_APP_INFO_CREATE_SUPPORTS_URIS, "G_APP_INFO_CREATE_SUPPORTS_URIS", "supports-uris" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_flags_register_static (g_intern_static_string ("GAppInfoCreateFlags"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

GType
g_data_stream_byte_order_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GEnumValue values[] = {
        { G_DATA_STREAM_BYTE_ORDER_BIG_ENDIAN, "G_DATA_STREAM_BYTE_ORDER_BIG_ENDIAN", "big-endian" },
        { G_DATA_STREAM_BYTE_ORDER_LITTLE_ENDIAN, "G_DATA_STREAM_BYTE_ORDER_LITTLE_ENDIAN", "little-endian" },
        { G_DATA_STREAM_BYTE_ORDER_HOST_ENDIAN, "G_DATA_STREAM_BYTE_ORDER_HOST_ENDIAN", "host-endian" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_enum_register_static (g_intern_static_string ("GDataStreamByteOrder"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

GType
g_data_stream_newline_type_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GEnumValue values[] = {
        { G_DATA_STREAM_NEWLINE_TYPE_LF, "G_DATA_STREAM_NEWLINE_TYPE_LF", "lf" },
        { G_DATA_STREAM_NEWLINE_TYPE_CR, "G_DATA_STREAM_NEWLINE_TYPE_CR", "cr" },
        { G_DATA_STREAM_NEWLINE_TYPE_CR_LF, "G_DATA_STREAM_NEWLINE_TYPE_CR_LF", "cr-lf" },
        { G_DATA_STREAM_NEWLINE_TYPE_ANY, "G_DATA_STREAM_NEWLINE_TYPE_ANY", "any" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_enum_register_static (g_intern_static_string ("GDataStreamNewlineType"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

GType
g_file_attribute_type_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GEnumValue values[] = {
        { G_FILE_ATTRIBUTE_TYPE_INVALID, "G_FILE_ATTRIBUTE_TYPE_INVALID", "invalid" },
        { G_FILE_ATTRIBUTE_TYPE_STRING, "G_FILE_ATTRIBUTE_TYPE_STRING", "string" },
        { G_FILE_ATTRIBUTE_TYPE_BYTE_STRING, "G_FILE_ATTRIBUTE_TYPE_BYTE_STRING", "byte-string" },
        { G_FILE_ATTRIBUTE_TYPE_BOOLEAN, "G_FILE_ATTRIBUTE_TYPE_BOOLEAN", "boolean" },
        { G_FILE_ATTRIBUTE_TYPE_UINT32, "G_FILE_ATTRIBUTE_TYPE_UINT32", "uint32" },
        { G_FILE_ATTRIBUTE_TYPE_INT32, "G_FILE_ATTRIBUTE_TYPE_INT32", "int32" },
        { G_FILE_ATTRIBUTE_TYPE_UINT64, "G_FILE_ATTRIBUTE_TYPE_UINT64", "uint64" },
        { G_FILE_ATTRIBUTE_TYPE_INT64, "G_FILE_ATTRIBUTE_TYPE_INT64", "int64" },
        { G_FILE_ATTRIBUTE_TYPE_OBJECT, "G_FILE_ATTRIBUTE_TYPE_OBJECT", "object" },
        { G_FILE_ATTRIBUTE_TYPE_STRINGV, "G_FILE_ATTRIBUTE_TYPE_STRINGV", "stringv" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_enum_register_static (g_intern_static_string ("GFileAttributeType"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

GType
g_file_attribute_info_flags_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GFlagsValue values[] = {
        { G_FILE_ATTRIBUTE_INFO_NONE, "G_FILE_ATTRIBUTE_INFO_NONE", "none" },
        { G_FILE_ATTRIBUTE_INFO_COPY_WITH_FILE, "G_FILE_ATTRIBUTE_INFO_COPY_WITH_FILE", "copy-with-file" },
        { G_FILE_ATTRIBUTE_INFO_COPY_WHEN_MOVED, "G_FILE_ATTRIBUTE_INFO_COPY_WHEN_MOVED", "copy-when-moved" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_flags_register_static (g_intern_static_string ("GFileAttributeInfoFlags"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

GType
g_file_attribute_status_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GEnumValue values[] = {
        { G_FILE_ATTRIBUTE_STATUS_UNSET, "G_FILE_ATTRIBUTE_STATUS_UNSET", "unset" },
        { G_FILE_ATTRIBUTE_STATUS_SET, "G_FILE_ATTRIBUTE_STATUS_SET", "set" },
        { G_FILE_ATTRIBUTE_STATUS_ERROR_SETTING, "G_FILE_ATTRIBUTE_STATUS_ERROR_SETTING", "error-setting" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_enum_register_static (g_intern_static_string ("GFileAttributeStatus"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

GType
g_file_query_info_flags_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GFlagsValue values[] = {
        { G_FILE_QUERY_INFO_NONE, "G_FILE_QUERY_INFO_NONE", "none" },
        { G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, "G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS", "nofollow-symlinks" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_flags_register_static (g_intern_static_string ("GFileQueryInfoFlags"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

GType
g_file_create_flags_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GFlagsValue values[] = {
        { G_FILE_CREATE_NONE, "G_FILE_CREATE_NONE", "none" },
        { G_FILE_CREATE_PRIVATE, "G_FILE_CREATE_PRIVATE", "private" },
        { G_FILE_CREATE_REPLACE_DESTINATION, "G_FILE_CREATE_REPLACE_DESTINATION", "replace-destination" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_flags_register_static (g_intern_static_string ("GFileCreateFlags"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

GType
g_mount_mount_flags_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GEnumValue values[] = {
        { G_MOUNT_MOUNT_NONE, "G_MOUNT_MOUNT_NONE", "none" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_enum_register_static (g_intern_static_string ("GMountMountFlags"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

GType
g_mount_unmount_flags_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GFlagsValue values[] = {
        { G_MOUNT_UNMOUNT_NONE, "G_MOUNT_UNMOUNT_NONE", "none" },
        { G_MOUNT_UNMOUNT_FORCE, "G_MOUNT_UNMOUNT_FORCE", "force" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_flags_register_static (g_intern_static_string ("GMountUnmountFlags"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

GType
g_drive_start_flags_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GEnumValue values[] = {
        { G_DRIVE_START_NONE, "G_DRIVE_START_NONE", "none" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_enum_register_static (g_intern_static_string ("GDriveStartFlags"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

GType
g_drive_start_stop_type_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GEnumValue values[] = {
        { G_DRIVE_START_STOP_TYPE_UNKNOWN, "G_DRIVE_START_STOP_TYPE_UNKNOWN", "unknown" },
        { G_DRIVE_START_STOP_TYPE_SHUTDOWN, "G_DRIVE_START_STOP_TYPE_SHUTDOWN", "shutdown" },
        { G_DRIVE_START_STOP_TYPE_NETWORK, "G_DRIVE_START_STOP_TYPE_NETWORK", "network" },
        { G_DRIVE_START_STOP_TYPE_MULTIDISK, "G_DRIVE_START_STOP_TYPE_MULTIDISK", "multidisk" },
        { G_DRIVE_START_STOP_TYPE_PASSWORD, "G_DRIVE_START_STOP_TYPE_PASSWORD", "password" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_enum_register_static (g_intern_static_string ("GDriveStartStopType"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

GType
g_file_copy_flags_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GFlagsValue values[] = {
        { G_FILE_COPY_NONE, "G_FILE_COPY_NONE", "none" },
        { G_FILE_COPY_OVERWRITE, "G_FILE_COPY_OVERWRITE", "overwrite" },
        { G_FILE_COPY_BACKUP, "G_FILE_COPY_BACKUP", "backup" },
        { G_FILE_COPY_NOFOLLOW_SYMLINKS, "G_FILE_COPY_NOFOLLOW_SYMLINKS", "nofollow-symlinks" },
        { G_FILE_COPY_ALL_METADATA, "G_FILE_COPY_ALL_METADATA", "all-metadata" },
        { G_FILE_COPY_NO_FALLBACK_FOR_MOVE, "G_FILE_COPY_NO_FALLBACK_FOR_MOVE", "no-fallback-for-move" },
        { G_FILE_COPY_TARGET_DEFAULT_PERMS, "G_FILE_COPY_TARGET_DEFAULT_PERMS", "target-default-perms" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_flags_register_static (g_intern_static_string ("GFileCopyFlags"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

GType
g_file_monitor_flags_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GFlagsValue values[] = {
        { G_FILE_MONITOR_NONE, "G_FILE_MONITOR_NONE", "none" },
        { G_FILE_MONITOR_WATCH_MOUNTS, "G_FILE_MONITOR_WATCH_MOUNTS", "watch-mounts" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_flags_register_static (g_intern_static_string ("GFileMonitorFlags"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

GType
g_file_type_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GEnumValue values[] = {
        { G_FILE_TYPE_UNKNOWN, "G_FILE_TYPE_UNKNOWN", "unknown" },
        { G_FILE_TYPE_REGULAR, "G_FILE_TYPE_REGULAR", "regular" },
        { G_FILE_TYPE_DIRECTORY, "G_FILE_TYPE_DIRECTORY", "directory" },
        { G_FILE_TYPE_SYMBOLIC_LINK, "G_FILE_TYPE_SYMBOLIC_LINK", "symbolic-link" },
        { G_FILE_TYPE_SPECIAL, "G_FILE_TYPE_SPECIAL", "special" },
        { G_FILE_TYPE_SHORTCUT, "G_FILE_TYPE_SHORTCUT", "shortcut" },
        { G_FILE_TYPE_MOUNTABLE, "G_FILE_TYPE_MOUNTABLE", "mountable" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_enum_register_static (g_intern_static_string ("GFileType"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

GType
g_filesystem_preview_type_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GEnumValue values[] = {
        { G_FILESYSTEM_PREVIEW_TYPE_IF_ALWAYS, "G_FILESYSTEM_PREVIEW_TYPE_IF_ALWAYS", "if-always" },
        { G_FILESYSTEM_PREVIEW_TYPE_IF_LOCAL, "G_FILESYSTEM_PREVIEW_TYPE_IF_LOCAL", "if-local" },
        { G_FILESYSTEM_PREVIEW_TYPE_NEVER, "G_FILESYSTEM_PREVIEW_TYPE_NEVER", "never" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_enum_register_static (g_intern_static_string ("GFilesystemPreviewType"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

GType
g_file_monitor_event_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GEnumValue values[] = {
        { G_FILE_MONITOR_EVENT_CHANGED, "G_FILE_MONITOR_EVENT_CHANGED", "changed" },
        { G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT, "G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT", "changes-done-hint" },
        { G_FILE_MONITOR_EVENT_DELETED, "G_FILE_MONITOR_EVENT_DELETED", "deleted" },
        { G_FILE_MONITOR_EVENT_CREATED, "G_FILE_MONITOR_EVENT_CREATED", "created" },
        { G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED, "G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED", "attribute-changed" },
        { G_FILE_MONITOR_EVENT_PRE_UNMOUNT, "G_FILE_MONITOR_EVENT_PRE_UNMOUNT", "pre-unmount" },
        { G_FILE_MONITOR_EVENT_UNMOUNTED, "G_FILE_MONITOR_EVENT_UNMOUNTED", "unmounted" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_enum_register_static (g_intern_static_string ("GFileMonitorEvent"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

GType
g_io_error_enum_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GEnumValue values[] = {
        { G_IO_ERROR_FAILED, "G_IO_ERROR_FAILED", "failed" },
        { G_IO_ERROR_NOT_FOUND, "G_IO_ERROR_NOT_FOUND", "not-found" },
        { G_IO_ERROR_EXISTS, "G_IO_ERROR_EXISTS", "exists" },
        { G_IO_ERROR_IS_DIRECTORY, "G_IO_ERROR_IS_DIRECTORY", "is-directory" },
        { G_IO_ERROR_NOT_DIRECTORY, "G_IO_ERROR_NOT_DIRECTORY", "not-directory" },
        { G_IO_ERROR_NOT_EMPTY, "G_IO_ERROR_NOT_EMPTY", "not-empty" },
        { G_IO_ERROR_NOT_REGULAR_FILE, "G_IO_ERROR_NOT_REGULAR_FILE", "not-regular-file" },
        { G_IO_ERROR_NOT_SYMBOLIC_LINK, "G_IO_ERROR_NOT_SYMBOLIC_LINK", "not-symbolic-link" },
        { G_IO_ERROR_NOT_MOUNTABLE_FILE, "G_IO_ERROR_NOT_MOUNTABLE_FILE", "not-mountable-file" },
        { G_IO_ERROR_FILENAME_TOO_LONG, "G_IO_ERROR_FILENAME_TOO_LONG", "filename-too-long" },
        { G_IO_ERROR_INVALID_FILENAME, "G_IO_ERROR_INVALID_FILENAME", "invalid-filename" },
        { G_IO_ERROR_TOO_MANY_LINKS, "G_IO_ERROR_TOO_MANY_LINKS", "too-many-links" },
        { G_IO_ERROR_NO_SPACE, "G_IO_ERROR_NO_SPACE", "no-space" },
        { G_IO_ERROR_INVALID_ARGUMENT, "G_IO_ERROR_INVALID_ARGUMENT", "invalid-argument" },
        { G_IO_ERROR_PERMISSION_DENIED, "G_IO_ERROR_PERMISSION_DENIED", "permission-denied" },
        { G_IO_ERROR_NOT_SUPPORTED, "G_IO_ERROR_NOT_SUPPORTED", "not-supported" },
        { G_IO_ERROR_NOT_MOUNTED, "G_IO_ERROR_NOT_MOUNTED", "not-mounted" },
        { G_IO_ERROR_ALREADY_MOUNTED, "G_IO_ERROR_ALREADY_MOUNTED", "already-mounted" },
        { G_IO_ERROR_CLOSED, "G_IO_ERROR_CLOSED", "closed" },
        { G_IO_ERROR_CANCELLED, "G_IO_ERROR_CANCELLED", "cancelled" },
        { G_IO_ERROR_PENDING, "G_IO_ERROR_PENDING", "pending" },
        { G_IO_ERROR_READ_ONLY, "G_IO_ERROR_READ_ONLY", "read-only" },
        { G_IO_ERROR_CANT_CREATE_BACKUP, "G_IO_ERROR_CANT_CREATE_BACKUP", "cant-create-backup" },
        { G_IO_ERROR_WRONG_ETAG, "G_IO_ERROR_WRONG_ETAG", "wrong-etag" },
        { G_IO_ERROR_TIMED_OUT, "G_IO_ERROR_TIMED_OUT", "timed-out" },
        { G_IO_ERROR_WOULD_RECURSE, "G_IO_ERROR_WOULD_RECURSE", "would-recurse" },
        { G_IO_ERROR_BUSY, "G_IO_ERROR_BUSY", "busy" },
        { G_IO_ERROR_WOULD_BLOCK, "G_IO_ERROR_WOULD_BLOCK", "would-block" },
        { G_IO_ERROR_HOST_NOT_FOUND, "G_IO_ERROR_HOST_NOT_FOUND", "host-not-found" },
        { G_IO_ERROR_WOULD_MERGE, "G_IO_ERROR_WOULD_MERGE", "would-merge" },
        { G_IO_ERROR_FAILED_HANDLED, "G_IO_ERROR_FAILED_HANDLED", "failed-handled" },
        { G_IO_ERROR_TOO_MANY_OPEN_FILES, "G_IO_ERROR_TOO_MANY_OPEN_FILES", "too-many-open-files" },
        { G_IO_ERROR_NOT_INITIALIZED, "G_IO_ERROR_NOT_INITIALIZED", "not-initialized" },
        { G_IO_ERROR_ADDRESS_IN_USE, "G_IO_ERROR_ADDRESS_IN_USE", "address-in-use" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_enum_register_static (g_intern_static_string ("GIOErrorEnum"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

GType
g_ask_password_flags_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GFlagsValue values[] = {
        { G_ASK_PASSWORD_NEED_PASSWORD, "G_ASK_PASSWORD_NEED_PASSWORD", "need-password" },
        { G_ASK_PASSWORD_NEED_USERNAME, "G_ASK_PASSWORD_NEED_USERNAME", "need-username" },
        { G_ASK_PASSWORD_NEED_DOMAIN, "G_ASK_PASSWORD_NEED_DOMAIN", "need-domain" },
        { G_ASK_PASSWORD_SAVING_SUPPORTED, "G_ASK_PASSWORD_SAVING_SUPPORTED", "saving-supported" },
        { G_ASK_PASSWORD_ANONYMOUS_SUPPORTED, "G_ASK_PASSWORD_ANONYMOUS_SUPPORTED", "anonymous-supported" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_flags_register_static (g_intern_static_string ("GAskPasswordFlags"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

GType
g_password_save_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GEnumValue values[] = {
        { G_PASSWORD_SAVE_NEVER, "G_PASSWORD_SAVE_NEVER", "never" },
        { G_PASSWORD_SAVE_FOR_SESSION, "G_PASSWORD_SAVE_FOR_SESSION", "for-session" },
        { G_PASSWORD_SAVE_PERMANENTLY, "G_PASSWORD_SAVE_PERMANENTLY", "permanently" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_enum_register_static (g_intern_static_string ("GPasswordSave"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

GType
g_mount_operation_result_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GEnumValue values[] = {
        { G_MOUNT_OPERATION_HANDLED, "G_MOUNT_OPERATION_HANDLED", "handled" },
        { G_MOUNT_OPERATION_ABORTED, "G_MOUNT_OPERATION_ABORTED", "aborted" },
        { G_MOUNT_OPERATION_UNHANDLED, "G_MOUNT_OPERATION_UNHANDLED", "unhandled" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_enum_register_static (g_intern_static_string ("GMountOperationResult"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

GType
g_output_stream_splice_flags_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GFlagsValue values[] = {
        { G_OUTPUT_STREAM_SPLICE_NONE, "G_OUTPUT_STREAM_SPLICE_NONE", "none" },
        { G_OUTPUT_STREAM_SPLICE_CLOSE_SOURCE, "G_OUTPUT_STREAM_SPLICE_CLOSE_SOURCE", "close-source" },
        { G_OUTPUT_STREAM_SPLICE_CLOSE_TARGET, "G_OUTPUT_STREAM_SPLICE_CLOSE_TARGET", "close-target" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_flags_register_static (g_intern_static_string ("GOutputStreamSpliceFlags"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

GType
g_emblem_origin_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GEnumValue values[] = {
        { G_EMBLEM_ORIGIN_UNKNOWN, "G_EMBLEM_ORIGIN_UNKNOWN", "unknown" },
        { G_EMBLEM_ORIGIN_DEVICE, "G_EMBLEM_ORIGIN_DEVICE", "device" },
        { G_EMBLEM_ORIGIN_LIVEMETADATA, "G_EMBLEM_ORIGIN_LIVEMETADATA", "livemetadata" },
        { G_EMBLEM_ORIGIN_TAG, "G_EMBLEM_ORIGIN_TAG", "tag" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_enum_register_static (g_intern_static_string ("GEmblemOrigin"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

GType
g_resolver_error_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GEnumValue values[] = {
        { G_RESOLVER_ERROR_NOT_FOUND, "G_RESOLVER_ERROR_NOT_FOUND", "not-found" },
        { G_RESOLVER_ERROR_TEMPORARY_FAILURE, "G_RESOLVER_ERROR_TEMPORARY_FAILURE", "temporary-failure" },
        { G_RESOLVER_ERROR_INTERNAL, "G_RESOLVER_ERROR_INTERNAL", "internal" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_enum_register_static (g_intern_static_string ("GResolverError"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

GType
g_socket_family_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GEnumValue values[] = {
        { G_SOCKET_FAMILY_INVALID, "G_SOCKET_FAMILY_INVALID", "invalid" },
        { G_SOCKET_FAMILY_UNIX, "G_SOCKET_FAMILY_UNIX", "unix" },
        { G_SOCKET_FAMILY_IPV4, "G_SOCKET_FAMILY_IPV4", "ipv4" },
        { G_SOCKET_FAMILY_IPV6, "G_SOCKET_FAMILY_IPV6", "ipv6" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_enum_register_static (g_intern_static_string ("GSocketFamily"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

GType
g_socket_type_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GEnumValue values[] = {
        { G_SOCKET_TYPE_INVALID, "G_SOCKET_TYPE_INVALID", "invalid" },
        { G_SOCKET_TYPE_STREAM, "G_SOCKET_TYPE_STREAM", "stream" },
        { G_SOCKET_TYPE_DATAGRAM, "G_SOCKET_TYPE_DATAGRAM", "datagram" },
        { G_SOCKET_TYPE_SEQPACKET, "G_SOCKET_TYPE_SEQPACKET", "seqpacket" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_enum_register_static (g_intern_static_string ("GSocketType"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

GType
g_socket_msg_flags_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GEnumValue values[] = {
        { G_SOCKET_MSG_NONE, "G_SOCKET_MSG_NONE", "none" },
        { G_SOCKET_MSG_OOB, "G_SOCKET_MSG_OOB", "oob" },
        { G_SOCKET_MSG_PEEK, "G_SOCKET_MSG_PEEK", "peek" },
        { G_SOCKET_MSG_DONTROUTE, "G_SOCKET_MSG_DONTROUTE", "dontroute" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_enum_register_static (g_intern_static_string ("GSocketMsgFlags"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

GType
g_socket_protocol_get_type (void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter (&g_define_type_id__volatile))
    {
      static const GEnumValue values[] = {
        { G_SOCKET_PROTOCOL_UNKNOWN, "G_SOCKET_PROTOCOL_UNKNOWN", "unknown" },
        { G_SOCKET_PROTOCOL_DEFAULT, "G_SOCKET_PROTOCOL_DEFAULT", "default" },
        { G_SOCKET_PROTOCOL_TCP, "G_SOCKET_PROTOCOL_TCP", "tcp" },
        { G_SOCKET_PROTOCOL_UDP, "G_SOCKET_PROTOCOL_UDP", "udp" },
        { G_SOCKET_PROTOCOL_SCTP, "G_SOCKET_PROTOCOL_SCTP", "sctp" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_enum_register_static (g_intern_static_string ("GSocketProtocol"), values);
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

  return g_define_type_id__volatile;
}

#define __GIO_ENUM_TYPES_C__
#include "gioaliasdef.c"

/* Generated data ends here */

