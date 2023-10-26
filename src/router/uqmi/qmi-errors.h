/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * libqmi-glib -- GLib/GIO based library to control QMI devices
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 * Copyright (C) 2012-2017 Aleksander Morgado <aleksander@aleksander.es>
 */

#ifndef _LIBQMI_GLIB_QMI_ERRORS_H_
#define _LIBQMI_GLIB_QMI_ERRORS_H_

/**
 * SECTION: qmi-errors
 *
 * This section defines common error types used in the interface.
 */

/**
 * QMI_DBUS_ERROR_PREFIX:
 *
 * Symbol defining the common string prefix used for all libqmi errors in DBus.
 *
 * Since: 1.0
 */
#define QMI_DBUS_ERROR_PREFIX "org.freedesktop.libqmi.Error"

/**
 * QMI_CORE_ERROR_DBUS_PREFIX:
 *
 * Symbol defining the common string prefix used for all #QmiCoreError errors in DBus.
 *
 * Since: 1.0
 */
#define QMI_CORE_ERROR_DBUS_PREFIX QMI_DBUS_ERROR_PREFIX ".Core"

/**
 * QMI_PROTOCOL_ERROR_DBUS_PREFIX:
 *
 * Symbol defining the common string prefix used for all #QmiProtocolError errors in DBus.
 *
 * Since: 1.0
 */
#define QMI_PROTOCOL_ERROR_DBUS_PREFIX QMI_DBUS_ERROR_PREFIX ".Protocol"

/**
 * QmiCoreError:
 * @QMI_CORE_ERROR_FAILED: Operation failed.
 * @QMI_CORE_ERROR_WRONG_STATE: Operation cannot be executed in the current state.
 * @QMI_CORE_ERROR_TIMEOUT: Operation timed out.
 * @QMI_CORE_ERROR_INVALID_ARGS: Invalid arguments given.
 * @QMI_CORE_ERROR_INVALID_MESSAGE: QMI message is invalid.
 * @QMI_CORE_ERROR_TLV_NOT_FOUND: TLV not found.
 * @QMI_CORE_ERROR_TLV_TOO_LONG: TLV is too long.
 * @QMI_CORE_ERROR_UNSUPPORTED: Not supported.
 * @QMI_CORE_ERROR_TLV_EMPTY: TLV has no value. Empty TLVs are not a real error, so this error type is never generated. Since: 1.12. Deprecated: 1.22.
 * @QMI_CORE_ERROR_UNEXPECTED_MESSAGE: QMI message is unexpected. Since: 1.16.
 * @QMI_CORE_ERROR_INVALID_DATA: Invalid data found in the message. Since: 1.24.6.
 *
 * Common errors that may be reported by libqmi-glib.
 *
 * Since: 1.0
 */
typedef enum { /*< since=1.0 >*/
    QMI_CORE_ERROR_FAILED             = 0, /*< nick=Failed >*/
    QMI_CORE_ERROR_WRONG_STATE        = 1, /*< nick=WrongState >*/
    QMI_CORE_ERROR_TIMEOUT            = 2, /*< nick=Timeout >*/
    QMI_CORE_ERROR_INVALID_ARGS       = 3, /*< nick=InvalidArgs >*/
    QMI_CORE_ERROR_INVALID_MESSAGE    = 4, /*< nick=InvalidMessage >*/
    QMI_CORE_ERROR_TLV_NOT_FOUND      = 5, /*< nick=TlvNotFound >*/
    QMI_CORE_ERROR_TLV_TOO_LONG       = 6, /*< nick=TlvTooLong >*/
    QMI_CORE_ERROR_UNSUPPORTED        = 7, /*< nick=Unsupported >*/
    QMI_CORE_ERROR_TLV_EMPTY          = 8, /*< nick=TlvEmpty >*/
    QMI_CORE_ERROR_UNEXPECTED_MESSAGE = 9, /*< nick=UnexpectedMessage >*/
    QMI_CORE_ERROR_INVALID_DATA       = 10, /*< nick=InvalidData >*/
} QmiCoreError;

/**
 * QmiProtocolError:
  * @QMI_PROTOCOL_ERROR_NONE: No error.
  * @QMI_PROTOCOL_ERROR_MALFORMED_MESSAGE: Malformed message.
  * @QMI_PROTOCOL_ERROR_NO_MEMORY: No memory.
  * @QMI_PROTOCOL_ERROR_INTERNAL: Internal.
  * @QMI_PROTOCOL_ERROR_ABORTED: Aborted.
  * @QMI_PROTOCOL_ERROR_CLIENT_IDS_EXHAUSTED: Client IDs exhausted.
  * @QMI_PROTOCOL_ERROR_UNABORTABLE_TRANSACTION: Unabortable transaction.
  * @QMI_PROTOCOL_ERROR_INVALID_CLIENT_ID: Invalid client ID.
  * @QMI_PROTOCOL_ERROR_NO_THRESHOLDS_PROVIDED: No thresholds provided.
  * @QMI_PROTOCOL_ERROR_INVALID_HANDLE: Invalid handle.
  * @QMI_PROTOCOL_ERROR_INVALID_PROFILE: Invalid profile.
  * @QMI_PROTOCOL_ERROR_INVALID_PIN_ID: Invalid PIN ID.
  * @QMI_PROTOCOL_ERROR_INCORRECT_PIN: Incorrect PIN.
  * @QMI_PROTOCOL_ERROR_NO_NETWORK_FOUND: No network found.
  * @QMI_PROTOCOL_ERROR_CALL_FAILED: Call failed.
  * @QMI_PROTOCOL_ERROR_OUT_OF_CALL: Out of call.
  * @QMI_PROTOCOL_ERROR_NOT_PROVISIONED: Not provisioned.
  * @QMI_PROTOCOL_ERROR_MISSING_ARGUMENT: Missing argument.
  * @QMI_PROTOCOL_ERROR_ARGUMENT_TOO_LONG: Argument too long.
  * @QMI_PROTOCOL_ERROR_INVALID_TRANSACTION_ID: Invalid transaction ID.
  * @QMI_PROTOCOL_ERROR_DEVICE_IN_USE: Device in use.
  * @QMI_PROTOCOL_ERROR_NETWORK_UNSUPPORTED: Network unsupported.
  * @QMI_PROTOCOL_ERROR_DEVICE_UNSUPPORTED: Device unsupported.
  * @QMI_PROTOCOL_ERROR_NO_EFFECT: No effect.
  * @QMI_PROTOCOL_ERROR_NO_FREE_PROFILE: No free profile.
  * @QMI_PROTOCOL_ERROR_INVALID_PDP_TYPE: Invalid PDP type.
  * @QMI_PROTOCOL_ERROR_INVALID_TECHNOLOGY_PREFERENCE: Invalid technology preference.
  * @QMI_PROTOCOL_ERROR_INVALID_PROFILE_TYPE: Invalid profile type.
  * @QMI_PROTOCOL_ERROR_INVALID_SERVICE_TYPE: Invalid service type.
  * @QMI_PROTOCOL_ERROR_INVALID_REGISTER_ACTION: Invalid register action.
  * @QMI_PROTOCOL_ERROR_INVALID_PS_ATTACH_ACTION: Invalid PS attach action.
  * @QMI_PROTOCOL_ERROR_AUTHENTICATION_FAILED: Authentication failed.
  * @QMI_PROTOCOL_ERROR_PIN_BLOCKED: PIN blocked.
  * @QMI_PROTOCOL_ERROR_PIN_ALWAYS_BLOCKED: PIN always blocked.
  * @QMI_PROTOCOL_ERROR_UIM_UNINITIALIZED: UIM uninitialized.
  * @QMI_PROTOCOL_ERROR_MAXIMUM_QOS_REQUESTS_IN_USE: Maximum QoS requests in use.
  * @QMI_PROTOCOL_ERROR_INCORRECT_FLOW_FILTER: Incorrect flow filter.
  * @QMI_PROTOCOL_ERROR_NETWORK_QOS_UNAWARE: Network QoS unaware.
  * @QMI_PROTOCOL_ERROR_INVALID_QOS_ID: Invalid QoS ID.
  * @QMI_PROTOCOL_ERROR_REQUESTED_NUMBER_UNSUPPORTED: Requested number unsupported. Since: 1.22.
  * @QMI_PROTOCOL_ERROR_INTERFACE_NOT_FOUND: Interface not found. Since: 1.22.
  * @QMI_PROTOCOL_ERROR_FLOW_SUSPENDED: Flow suspended.
  * @QMI_PROTOCOL_ERROR_INVALID_DATA_FORMAT: Invalid data format. Since: 1.22.
  * @QMI_PROTOCOL_ERROR_GENERAL_ERROR: General error.
  * @QMI_PROTOCOL_ERROR_UNKNOWN_ERROR: Unknown error.
  * @QMI_PROTOCOL_ERROR_INVALID_ARGUMENT: Invalid argument.
  * @QMI_PROTOCOL_ERROR_INVALID_INDEX: Invalid index.
  * @QMI_PROTOCOL_ERROR_NO_ENTRY: No entry.
  * @QMI_PROTOCOL_ERROR_DEVICE_STORAGE_FULL: Device storage full.
  * @QMI_PROTOCOL_ERROR_DEVICE_NOT_READY: Device not ready.
  * @QMI_PROTOCOL_ERROR_NETWORK_NOT_READY: Network not ready.
  * @QMI_PROTOCOL_ERROR_WMS_CAUSE_CODE: WMS cause code.
  * @QMI_PROTOCOL_ERROR_WMS_MESSAGE_NOT_SENT: WMS message not sent.
  * @QMI_PROTOCOL_ERROR_WMS_MESSAGE_DELIVERY_FAILURE: WMS message delivery failure.
  * @QMI_PROTOCOL_ERROR_WMS_INVALID_MESSAGE_ID: WMS invalid message ID.
  * @QMI_PROTOCOL_ERROR_WMS_ENCODING: WMS encoding.
  * @QMI_PROTOCOL_ERROR_AUTHENTICATION_LOCK: Authentication lock.
  * @QMI_PROTOCOL_ERROR_INVALID_TRANSITION: Invalid transition.
  * @QMI_PROTOCOL_ERROR_NOT_MCAST_INTERFACE: Not a multicast interface. Since: 1.22.
  * @QMI_PROTOCOL_ERROR_MAXIMUM_MCAST_REQUESTS_IN_USE: Maximum multicast requests in use. Since: 1.22.
  * @QMI_PROTOCOL_ERROR_INVALID_MCAST_HANDLE: Invalid mulitcast handle. Since: 1.22.
  * @QMI_PROTOCOL_ERROR_INVALID_IP_FAMILY_PREFERENCE: Invalid IP family preference. Since: 1.22.
  * @QMI_PROTOCOL_ERROR_SESSION_INACTIVE: Session inactive.
  * @QMI_PROTOCOL_ERROR_SESSION_INVALID: Session invalid.
  * @QMI_PROTOCOL_ERROR_SESSION_OWNERSHIP: Session ownership.
  * @QMI_PROTOCOL_ERROR_INSUFFICIENT_RESOURCES: Insufficient resources.
  * @QMI_PROTOCOL_ERROR_DISABLED: Disabled.
  * @QMI_PROTOCOL_ERROR_INVALID_OPERATION: Invalid operation.
  * @QMI_PROTOCOL_ERROR_INVALID_QMI_COMMAND: Invalid QMI command.
  * @QMI_PROTOCOL_ERROR_WMS_T_PDU_TYPE: WMS T-PDU type.
  * @QMI_PROTOCOL_ERROR_WMS_SMSC_ADDRESS: WMS SMSC address.
  * @QMI_PROTOCOL_ERROR_INFORMATION_UNAVAILABLE: Information unavailable.
  * @QMI_PROTOCOL_ERROR_SEGMENT_TOO_LONG: Segment too long.
  * @QMI_PROTOCOL_ERROR_SEGMENT_ORDER: Segment order.
  * @QMI_PROTOCOL_ERROR_BUNDLING_NOT_SUPPORTED: Bundling not supported.
  * @QMI_PROTOCOL_ERROR_OPERATION_PARTIAL_FAILURE: Operation partial failure. Since: 1.22.
  * @QMI_PROTOCOL_ERROR_POLICY_MISMATCH: Policy mismatch. Since: 1.6.
  * @QMI_PROTOCOL_ERROR_SIM_FILE_NOT_FOUND: SIM file not found.
  * @QMI_PROTOCOL_ERROR_EXTENDED_INTERNAL: Extended internal error. Since: 1.6.
  * @QMI_PROTOCOL_ERROR_ACCESS_DENIED: Access denied.
  * @QMI_PROTOCOL_ERROR_HARDWARE_RESTRICTED: Hardware restricted.
  * @QMI_PROTOCOL_ERROR_ACK_NOT_SENT: ACK not sent. Since: 1.6.
  * @QMI_PROTOCOL_ERROR_INJECT_TIMEOUT: Inject timeout. Since: 1.6.
  * @QMI_PROTOCOL_ERROR_INCOMPATIBLE_STATE: Incompatible state. Since: 1.6.
  * @QMI_PROTOCOL_ERROR_FDN_RESTRICT: FDN restrict. Since: 1.6.
  * @QMI_PROTOCOL_ERROR_SUPS_FAILURE_CASE: SUPS failure case. Since: 1.6.
  * @QMI_PROTOCOL_ERROR_NO_RADIO: No radio. Since: 1.6.
  * @QMI_PROTOCOL_ERROR_NOT_SUPPORTED: Not supported. Since: 1.6.
  * @QMI_PROTOCOL_ERROR_NO_SUBSCRIPTION: No subscription. Since: 1.6.
  * @QMI_PROTOCOL_ERROR_CARD_CALL_CONTROL_FAILED: Card call control failed. Since: 1.6.
  * @QMI_PROTOCOL_ERROR_NETWORK_ABORTED: Network aborted. Since: 1.6.
  * @QMI_PROTOCOL_ERROR_MSG_BLOCKED: Message blocked. Since: 1.6.
  * @QMI_PROTOCOL_ERROR_INVALID_SESSION_TYPE: Invalid session type. Since: 1.6.
  * @QMI_PROTOCOL_ERROR_INVALID_PB_TYPE: Invalid PB type. Since: 1.6.
  * @QMI_PROTOCOL_ERROR_NO_SIM: No SIM. Since: 1.6.
  * @QMI_PROTOCOL_ERROR_PB_NOT_READY: PB not ready. Since: 1.6.
  * @QMI_PROTOCOL_ERROR_PIN_RESTRICTION: PIN restriction. Since: 1.6.
  * @QMI_PROTOCOL_ERROR_PIN2_RESTRICTION: PIN2 restriction. Since: 1.6.
  * @QMI_PROTOCOL_ERROR_PUK_RESTRICTION: PUK restriction. Since: 1.6.
  * @QMI_PROTOCOL_ERROR_PUK2_RESTRICTION: PUK2 restriction. Since: 1.6.
  * @QMI_PROTOCOL_ERROR_PB_ACCESS_RESTRICTED: PB access restricted. Since: 1.6.
  * @QMI_PROTOCOL_ERROR_PB_DELETE_IN_PROGRESS: PB delete in progress. Since: 1.22.
  * @QMI_PROTOCOL_ERROR_PB_TEXT_TOO_LONG: PB text too long. Since: 1.6.
  * @QMI_PROTOCOL_ERROR_PB_NUMBER_TOO_LONG: PB number too long. Since: 1.6.
  * @QMI_PROTOCOL_ERROR_PB_HIDDEN_KEY_RESTRICTION: PB hidden key restriction. Since: 1.6.
  * @QMI_PROTOCOL_ERROR_PB_NOT_AVAILABLE: PB not available. Since: 1.22.
  * @QMI_PROTOCOL_ERROR_DEVICE_MEMORY_ERROR: Device memory error. Since: 1.22.
  * @QMI_PROTOCOL_ERROR_NO_PERMISSION: No permission. Since: 1.22.
  * @QMI_PROTOCOL_ERROR_TOO_SOON: Too soon. Since: 1.22.
  * @QMI_PROTOCOL_ERROR_TIME_NOT_ACQUIRED: Time not acquired. Since: 1.22.
  * @QMI_PROTOCOL_ERROR_OPERATION_IN_PROGRESS: Operation in progress. Since: 1.22.
  * @QMI_PROTOCOL_ERROR_FW_WRITE_FAILED: Firmware write failed. Since: 1.22.
  * @QMI_PROTOCOL_ERROR_FW_INFO_READ_FAILED: Firmware info read failed. Since: 1.22.
  * @QMI_PROTOCOL_ERROR_FW_FILE_NOT_FOUND: Firmware file not found. Since: 1.22.
  * @QMI_PROTOCOL_ERROR_FW_DIR_NOT_FOUND: Firmware dir not found. Since: 1.22.
  * @QMI_PROTOCOL_ERROR_FW_ALREADY_ACTIVATED: Firmware already activated. Since: 1.22.
  * @QMI_PROTOCOL_ERROR_FW_CANNOT_GENERIC_IMAGE: Firmware cannot generic image. Since: 1.22.
  * @QMI_PROTOCOL_ERROR_FW_FILE_OPEN_FAILED: Firmware file open failed. Since: 1.22.
  * @QMI_PROTOCOL_ERROR_FW_UPDATE_DISCONTINUOUS_FRAME: Firmware update discontinuous frame. Since: 1.22.
  * @QMI_PROTOCOL_ERROR_FW_UPDATE_FAILED: Firmware update failed. Since: 1.22.
  * @QMI_PROTOCOL_ERROR_CAT_EVENT_REGISTRATION_FAILED: Event registration failed.
  * @QMI_PROTOCOL_ERROR_CAT_INVALID_TERMINAL_RESPONSE: Invalid terminal response.
  * @QMI_PROTOCOL_ERROR_CAT_INVALID_ENVELOPE_COMMAND: Invalid envelope command.
  * @QMI_PROTOCOL_ERROR_CAT_ENVELOPE_COMMAND_BUSY: Envelope command busy.
  * @QMI_PROTOCOL_ERROR_CAT_ENVELOPE_COMMAND_FAILED: Envelope command failed.
  *
  * QMI protocol errors.
  *
  * Since: 1.0
  */
typedef enum { /*< since=1.0 >*/
  QMI_PROTOCOL_ERROR_NONE                             = 0,  /*< nick=None >*/
  QMI_PROTOCOL_ERROR_MALFORMED_MESSAGE                = 1,  /*< nick=MalformedMessage >*/
  QMI_PROTOCOL_ERROR_NO_MEMORY                        = 2,  /*< nick=NoMemory >*/
  QMI_PROTOCOL_ERROR_INTERNAL                         = 3,  /*< nick=Internal >*/
  QMI_PROTOCOL_ERROR_ABORTED                          = 4,  /*< nick=Aborted >*/
  QMI_PROTOCOL_ERROR_CLIENT_IDS_EXHAUSTED             = 5,  /*< nick=ClientIdsExhausted >*/
  QMI_PROTOCOL_ERROR_UNABORTABLE_TRANSACTION          = 6,  /*< nick=UnabortableTransaction >*/
  QMI_PROTOCOL_ERROR_INVALID_CLIENT_ID                = 7,  /*< nick=InvalidClientId >*/
  QMI_PROTOCOL_ERROR_NO_THRESHOLDS_PROVIDED           = 8,  /*< nick=NoThresholdsProvided >*/
  QMI_PROTOCOL_ERROR_INVALID_HANDLE                   = 9,  /*< nick=InvalidHandle >*/
  QMI_PROTOCOL_ERROR_INVALID_PROFILE                  = 10, /*< nick=InvalidProfile >*/
  QMI_PROTOCOL_ERROR_INVALID_PIN_ID                   = 11, /*< nick=InvalidPinId >*/
  QMI_PROTOCOL_ERROR_INCORRECT_PIN                    = 12, /*< nick=IncorrectPin >*/
  QMI_PROTOCOL_ERROR_NO_NETWORK_FOUND                 = 13, /*< nick=NoNetworkFound >*/
  QMI_PROTOCOL_ERROR_CALL_FAILED                      = 14, /*< nick=CallFailed >*/
  QMI_PROTOCOL_ERROR_OUT_OF_CALL                      = 15, /*< nick=OutOfCall >*/
  QMI_PROTOCOL_ERROR_NOT_PROVISIONED                  = 16, /*< nick=NotProvisioned >*/
  QMI_PROTOCOL_ERROR_MISSING_ARGUMENT                 = 17, /*< nick=MissingArgument >*/
  QMI_PROTOCOL_ERROR_ARGUMENT_TOO_LONG                = 19, /*< nick=ArgumentTooLong >*/
  QMI_PROTOCOL_ERROR_INVALID_TRANSACTION_ID           = 22, /*< nick=InvalidTransactionId >*/
  QMI_PROTOCOL_ERROR_DEVICE_IN_USE                    = 23, /*< nick=DeviceInUse >*/
  QMI_PROTOCOL_ERROR_NETWORK_UNSUPPORTED              = 24, /*< nick=NetworkUnsupported >*/
  QMI_PROTOCOL_ERROR_DEVICE_UNSUPPORTED               = 25, /*< nick=DeviceUnsupported >*/
  QMI_PROTOCOL_ERROR_NO_EFFECT                        = 26, /*< nick=NoEffect >*/
  QMI_PROTOCOL_ERROR_NO_FREE_PROFILE                  = 27, /*< nick=NoFreeProfile >*/
  QMI_PROTOCOL_ERROR_INVALID_PDP_TYPE                 = 28, /*< nick=InvalidPdpType >*/
  QMI_PROTOCOL_ERROR_INVALID_TECHNOLOGY_PREFERENCE    = 29, /*< nick=InvalidTechnologyPreference >*/
  QMI_PROTOCOL_ERROR_INVALID_PROFILE_TYPE             = 30, /*< nick=InvalidProfileType >*/
  QMI_PROTOCOL_ERROR_INVALID_SERVICE_TYPE             = 31, /*< nick=InvalidServiceType >*/
  QMI_PROTOCOL_ERROR_INVALID_REGISTER_ACTION          = 32, /*< nick=InvalidRegisterAction >*/
  QMI_PROTOCOL_ERROR_INVALID_PS_ATTACH_ACTION         = 33, /*< nick=InvalidPsAttachAction >*/
  QMI_PROTOCOL_ERROR_AUTHENTICATION_FAILED            = 34, /*< nick=AuthenticationFailed >*/
  QMI_PROTOCOL_ERROR_PIN_BLOCKED                      = 35, /*< nick=PinBlocked >*/
  QMI_PROTOCOL_ERROR_PIN_ALWAYS_BLOCKED               = 36, /*< nick=PinAlwaysBlocked >*/
  QMI_PROTOCOL_ERROR_UIM_UNINITIALIZED                = 37, /*< nick=UimUninitialized >*/
  QMI_PROTOCOL_ERROR_MAXIMUM_QOS_REQUESTS_IN_USE      = 38, /*< nick=MaximumQosRequestsInUse >*/
  QMI_PROTOCOL_ERROR_INCORRECT_FLOW_FILTER            = 39, /*< nick=IncorrectFlowFilter >*/
  QMI_PROTOCOL_ERROR_NETWORK_QOS_UNAWARE              = 40, /*< nick=NetworkQosUnaware >*/
  QMI_PROTOCOL_ERROR_INVALID_QOS_ID                   = 41, /*< nick=InvalidQosId >*/
  QMI_PROTOCOL_ERROR_REQUESTED_NUMBER_UNSUPPORTED     = 42, /*< nick=RequestedNumberUnsupported >*/
  QMI_PROTOCOL_ERROR_INTERFACE_NOT_FOUND              = 43, /*< nick=InterfaceNotFound >*/
  QMI_PROTOCOL_ERROR_FLOW_SUSPENDED                   = 44, /*< nick=FlowSuspended >*/
  QMI_PROTOCOL_ERROR_INVALID_DATA_FORMAT              = 45, /*< nick=InvalidDataFormat >*/
  QMI_PROTOCOL_ERROR_GENERAL_ERROR                    = 46, /*< nick=GeneralError >*/
  QMI_PROTOCOL_ERROR_UNKNOWN_ERROR                    = 47, /*< nick=UnknownError >*/
  QMI_PROTOCOL_ERROR_INVALID_ARGUMENT                 = 48, /*< nick=InvalidArgument >*/
  QMI_PROTOCOL_ERROR_INVALID_INDEX                    = 49, /*< nick=InvalidIndex >*/
  QMI_PROTOCOL_ERROR_NO_ENTRY                         = 50, /*< nick=NoEntry >*/
  QMI_PROTOCOL_ERROR_DEVICE_STORAGE_FULL              = 51, /*< nick=DeviceStorageFull >*/
  QMI_PROTOCOL_ERROR_DEVICE_NOT_READY                 = 52, /*< nick=DeviceNotReady >*/
  QMI_PROTOCOL_ERROR_NETWORK_NOT_READY                = 53, /*< nick=NetworkNotReady >*/
  QMI_PROTOCOL_ERROR_WMS_CAUSE_CODE                   = 54, /*< nick=WmsCauseCode >*/
  QMI_PROTOCOL_ERROR_WMS_MESSAGE_NOT_SENT             = 55, /*< nick=WmsMessageNotSent >*/
  QMI_PROTOCOL_ERROR_WMS_MESSAGE_DELIVERY_FAILURE     = 56, /*< nick=WmsMessageDeliveryFailure >*/
  QMI_PROTOCOL_ERROR_WMS_INVALID_MESSAGE_ID           = 57, /*< nick=WmsInvalidMessageId >*/
  QMI_PROTOCOL_ERROR_WMS_ENCODING                     = 58, /*< nick=WmsEncoding >*/
  QMI_PROTOCOL_ERROR_AUTHENTICATION_LOCK              = 59, /*< nick=AuthenticationLock >*/
  QMI_PROTOCOL_ERROR_INVALID_TRANSITION               = 60, /*< nick=InvalidTransition >*/
  QMI_PROTOCOL_ERROR_NOT_MCAST_INTERFACE              = 61, /*< nick=NotMcastInterface >*/
  QMI_PROTOCOL_ERROR_MAXIMUM_MCAST_REQUESTS_IN_USE    = 62, /*< nick=MaximumMcastRequestsInUse >*/
  QMI_PROTOCOL_ERROR_INVALID_MCAST_HANDLE             = 63, /*< nick=InvalidMcastHandle >*/
  QMI_PROTOCOL_ERROR_INVALID_IP_FAMILY_PREFERENCE     = 64, /*< nick=InvalidIpFamilyPreference >*/
  QMI_PROTOCOL_ERROR_SESSION_INACTIVE                 = 65, /*< nick=SessionInactive >*/
  QMI_PROTOCOL_ERROR_SESSION_INVALID                  = 66, /*< nick=SessionInvalid >*/
  QMI_PROTOCOL_ERROR_SESSION_OWNERSHIP                = 67, /*< nick=SessionOwnership >*/
  QMI_PROTOCOL_ERROR_INSUFFICIENT_RESOURCES           = 68, /*< nick=InsufficientResources >*/
  QMI_PROTOCOL_ERROR_DISABLED                         = 69, /*< nick=Disabled >*/
  QMI_PROTOCOL_ERROR_INVALID_OPERATION                = 70, /*< nick=InvalidOperation >*/
  QMI_PROTOCOL_ERROR_INVALID_QMI_COMMAND              = 71, /*< nick=InvalidQmiCommand >*/
  QMI_PROTOCOL_ERROR_WMS_T_PDU_TYPE                   = 72, /*< nick=WmsTPduType >*/
  QMI_PROTOCOL_ERROR_WMS_SMSC_ADDRESS                 = 73, /*< nick=WmsSmscAddress >*/
  QMI_PROTOCOL_ERROR_INFORMATION_UNAVAILABLE          = 74, /*< nick=InformationUnavailable >*/
  QMI_PROTOCOL_ERROR_SEGMENT_TOO_LONG                 = 75, /*< nick=SegmentTooLong >*/
  QMI_PROTOCOL_ERROR_SEGMENT_ORDER                    = 76, /*< nick=SegmentOrder >*/
  QMI_PROTOCOL_ERROR_BUNDLING_NOT_SUPPORTED           = 77, /*< nick=BundlingNotSupported >*/
  QMI_PROTOCOL_ERROR_OPERATION_PARTIAL_FAILURE        = 78, /*< nick=OperationPartialFailure >*/
  QMI_PROTOCOL_ERROR_POLICY_MISMATCH                  = 79, /*< nick=PolicyMismatch >*/
  QMI_PROTOCOL_ERROR_SIM_FILE_NOT_FOUND               = 80, /*< nick=SimFileNotFound >*/
  QMI_PROTOCOL_ERROR_EXTENDED_INTERNAL                = 81, /*< nick=ExtendedInternal >*/
  QMI_PROTOCOL_ERROR_ACCESS_DENIED                    = 82, /*< nick=AccessDenied >*/
  QMI_PROTOCOL_ERROR_HARDWARE_RESTRICTED              = 83, /*< nick=HardwareRestricted >*/
  QMI_PROTOCOL_ERROR_ACK_NOT_SENT                     = 84, /*< nick=AckNotSent >*/
  QMI_PROTOCOL_ERROR_INJECT_TIMEOUT                   = 85, /*< nick=InjectTimeout >*/
  QMI_PROTOCOL_ERROR_INCOMPATIBLE_STATE               = 90, /*< nick=IncompatibleState >*/
  QMI_PROTOCOL_ERROR_FDN_RESTRICT                     = 91, /*< nick=FdnRestrict >*/
  QMI_PROTOCOL_ERROR_SUPS_FAILURE_CASE                = 92, /*< nick=SupsFailureCase >*/
  QMI_PROTOCOL_ERROR_NO_RADIO                         = 93, /*< nick=NoRadio >*/
  QMI_PROTOCOL_ERROR_NOT_SUPPORTED                    = 94, /*< nick=NotSupported >*/
  QMI_PROTOCOL_ERROR_NO_SUBSCRIPTION                  = 95, /*< nick=NoSubscription >*/
  QMI_PROTOCOL_ERROR_CARD_CALL_CONTROL_FAILED         = 96, /*< nick=CardCallControlFailed >*/
  QMI_PROTOCOL_ERROR_NETWORK_ABORTED                  = 97, /*< nick=NetworkAborted >*/
  QMI_PROTOCOL_ERROR_MSG_BLOCKED                      = 98, /*< nick=MsgBlocked >*/
  QMI_PROTOCOL_ERROR_INVALID_SESSION_TYPE             = 100, /*< nick=InvalidSessionType >*/
  QMI_PROTOCOL_ERROR_INVALID_PB_TYPE                  = 101, /*< nick=InvalidPbType >*/
  QMI_PROTOCOL_ERROR_NO_SIM                           = 102, /*< nick=NoSim >*/
  QMI_PROTOCOL_ERROR_PB_NOT_READY                     = 103, /*< nick=PbNotReady >*/
  QMI_PROTOCOL_ERROR_PIN_RESTRICTION                  = 104, /*< nick=PinRestriction >*/
  QMI_PROTOCOL_ERROR_PIN2_RESTRICTION                 = 105, /*< nick=Pin1Restriction >*/
  QMI_PROTOCOL_ERROR_PUK_RESTRICTION                  = 106, /*< nick=PukRestriction >*/
  QMI_PROTOCOL_ERROR_PUK2_RESTRICTION                 = 107, /*< nick=Puk2Restriction >*/
  QMI_PROTOCOL_ERROR_PB_ACCESS_RESTRICTED             = 108, /*< nick=PbAccessRestricted >*/
  QMI_PROTOCOL_ERROR_PB_DELETE_IN_PROGRESS            = 109, /*< nick=PbDeleteInProgress >*/
  QMI_PROTOCOL_ERROR_PB_TEXT_TOO_LONG                 = 110, /*< nick=PbTextTooLong >*/
  QMI_PROTOCOL_ERROR_PB_NUMBER_TOO_LONG               = 111, /*< nick=PbNumberTooLong >*/
  QMI_PROTOCOL_ERROR_PB_HIDDEN_KEY_RESTRICTION        = 112, /*< nick=PbHiddenKeyRestriction >*/
  QMI_PROTOCOL_ERROR_PB_NOT_AVAILABLE                 = 113, /*< nick=PbNotAvailable >*/
  QMI_PROTOCOL_ERROR_DEVICE_MEMORY_ERROR              = 114, /*< nick=DeviceMemoryError >*/
  QMI_PROTOCOL_ERROR_NO_PERMISSION                    = 115, /*< nick=NoPermission >*/
  QMI_PROTOCOL_ERROR_TOO_SOON                         = 116, /*< nick=TooSoon >*/
  QMI_PROTOCOL_ERROR_TIME_NOT_ACQUIRED                = 117, /*< nick=TimeNotAcquired >*/
  QMI_PROTOCOL_ERROR_OPERATION_IN_PROGRESS            = 118, /*< nick=OperationInProgress >*/
  QMI_PROTOCOL_ERROR_FW_WRITE_FAILED                  = 388, /*< nick=FwWriteFailed >*/
  QMI_PROTOCOL_ERROR_FW_INFO_READ_FAILED              = 389, /*< nick=FwInfoReadFailed >*/
  QMI_PROTOCOL_ERROR_FW_FILE_NOT_FOUND                = 390, /*< nick=FwFileNotFound >*/
  QMI_PROTOCOL_ERROR_FW_DIR_NOT_FOUND                 = 391, /*< nick=FwDirNotFound >*/
  QMI_PROTOCOL_ERROR_FW_ALREADY_ACTIVATED             = 392, /*< nick=FwAlreadyActivated >*/
  QMI_PROTOCOL_ERROR_FW_CANNOT_GENERIC_IMAGE          = 393, /*< nick=FwCannotGenericImage >*/
  QMI_PROTOCOL_ERROR_FW_FILE_OPEN_FAILED              = 400, /*< nick=FwFileOpenFailed >*/
  QMI_PROTOCOL_ERROR_FW_UPDATE_DISCONTINUOUS_FRAME    = 401, /*< nick=FwUpdateDiscontinuousFrame >*/
  QMI_PROTOCOL_ERROR_FW_UPDATE_FAILED                 = 402, /*< nick=FwUpdateFailed >*/
  QMI_PROTOCOL_ERROR_CAT_EVENT_REGISTRATION_FAILED    = 61441, /*< nick=CatEventRegistrationFailed >*/
  QMI_PROTOCOL_ERROR_CAT_INVALID_TERMINAL_RESPONSE    = 61442, /*< nick=CatInvalidTerminalResponse >*/
  QMI_PROTOCOL_ERROR_CAT_INVALID_ENVELOPE_COMMAND     = 61443, /*< nick=CatInvalidEnvelopeCommand >*/
  QMI_PROTOCOL_ERROR_CAT_ENVELOPE_COMMAND_BUSY        = 61444, /*< nick=CatEnvelopeCommandBusy >*/
  QMI_PROTOCOL_ERROR_CAT_ENVELOPE_COMMAND_FAILED      = 61445  /*< nick=CatEnvelopeCommandFailed >*/
} QmiProtocolError;

#endif /* _LIBQMI_GLIB_QMI_ERRORS_H_ */
