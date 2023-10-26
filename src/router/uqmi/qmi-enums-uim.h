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
 * Copyright (C) 2012 Google Inc.
 * Copyright (C) 2012-2017 Aleksander Morgado <aleksander@aleksander.es>
 */

#ifndef _LIBQMI_GLIB_QMI_ENUMS_UIM_H_
#define _LIBQMI_GLIB_QMI_ENUMS_UIM_H_

/**
 * SECTION: qmi-enums-uim
 *
 * This section defines enumerations and flags used in the UIM service
 * interface.
 */

/*****************************************************************************/
/* Helper enums for the 'QMI UIM Indication Register' indication */

/**
 * QmiUimEventRegistrationFlag:
 * @QMI_UIM_EVENT_REGISTRATION_FLAG_CARD_STATUS: Card status.
 * @QMI_UIM_EVENT_REGISTRATION_FLAG_SAP_CONNECTION: SAP connection.
 * @QMI_UIM_EVENT_REGISTRATION_FLAG_EXTENDED_CARD_STATUS: Extended card status.
 * @QMI_UIM_EVENT_REGISTRATION_FLAG_PHYSICAL_SLOT_STATUS: Physical slot status. Since 1.26.
 *
 * Flags to use to register to UIM indications.
 *
 * Since: 1.22.4
 */
typedef enum { /*< since=1.22.4 >*/
    QMI_UIM_EVENT_REGISTRATION_FLAG_CARD_STATUS          = 1 << 0,
    QMI_UIM_EVENT_REGISTRATION_FLAG_SAP_CONNECTION       = 1 << 1,
    QMI_UIM_EVENT_REGISTRATION_FLAG_EXTENDED_CARD_STATUS = 1 << 2,
    QMI_UIM_EVENT_REGISTRATION_FLAG_PHYSICAL_SLOT_STATUS = 1 << 4,
} QmiUimEventRegistrationFlag;

/*****************************************************************************/
/* Helper enums for the 'QMI UIM Read Record' request/response */

/**
 * QmiUimSessionType:
 * @QMI_UIM_SESSION_TYPE_PRIMARY_GW_PROVISIONING: Primary GSM/WCDMA provisioning.
 * @QMI_UIM_SESSION_TYPE_PRIMARY_1X_PROVISIONING: Primary CDMA1x provisioning.
 * @QMI_UIM_SESSION_TYPE_SECONDARY_GW_PROVISIONING: Secondary GSM/WCDMA provisioning.
 * @QMI_UIM_SESSION_TYPE_SECONDARY_1X_PROVISIONING: Secondary CDMA1x provisioning.
 * @QMI_UIM_SESSION_TYPE_NONPROVISIONING_SLOT_1: Nonprovisioning on slot 1.
 * @QMI_UIM_SESSION_TYPE_NONPROVISIONING_SLOT_2: Nonprovisioning on slot 2.
 * @QMI_UIM_SESSION_TYPE_CARD_SLOT_1: Card on slot 1.
 * @QMI_UIM_SESSION_TYPE_CARD_SLOT_2: Card on slot 2.
 * @QMI_UIM_SESSION_TYPE_LOGICAL_CHANNEL_SLOT_1: Logical channel on slot 1.
 * @QMI_UIM_SESSION_TYPE_LOGICAL_CHANNEL_SLOT_2: Logical channel on slot 2.
 * @QMI_UIM_SESSION_TYPE_TERTIARY_GW_PROVISIONING: Tertiary GSM/WCDMA provisioning. Since 1.28.
 * @QMI_UIM_SESSION_TYPE_TERTIARY_1X_PROVISIONING: Tertiary CDMA1x provisioning. Since 1.28.
 * @QMI_UIM_SESSION_TYPE_QUATERNARY_GW_PROVISIONING: Quaternary GSM/WCDMA provisioning. Since 1.28.
 * @QMI_UIM_SESSION_TYPE_QUATERNARY_1X_PROVISIONING: Quaternary CDMA1x provisioning. Since 1.28.
 * @QMI_UIM_SESSION_TYPE_QUINARY_GW_PROVISIONING: Quinary GSM/WCDMA provisioning. Since 1.28.
 * @QMI_UIM_SESSION_TYPE_QUINARY_1X_PROVISIONING: Quinary CDMA1x provisioning. Since 1.28.
 * @QMI_UIM_SESSION_TYPE_NONPROVISIONING_SLOT_3: Nonprovisioning on slot 3. Since 1.28.
 * @QMI_UIM_SESSION_TYPE_NONPROVISIONING_SLOT_4: Nonprovisioning on slot 4. Since 1.28.
 * @QMI_UIM_SESSION_TYPE_NONPROVISIONING_SLOT_5: Nonprovisioning on slot 5. Since 1.28.
 * @QMI_UIM_SESSION_TYPE_CARD_SLOT_3: Card on slot 3. Since 1.28.
 * @QMI_UIM_SESSION_TYPE_CARD_SLOT_4: Card on slot 4. Since 1.28.
 * @QMI_UIM_SESSION_TYPE_CARD_SLOT_5: Card on slot 5. Since 1.28.
 * @QMI_UIM_SESSION_TYPE_LOGICAL_CHANNEL_SLOT_3: Logical channel on slot 3. Since 1.28.
 * @QMI_UIM_SESSION_TYPE_LOGICAL_CHANNEL_SLOT_4: Logical channel on slot 4. Since 1.28.
 * @QMI_UIM_SESSION_TYPE_LOGICAL_CHANNEL_SLOT_5: Logical channel on slot 5. Since 1.28.
 *
 * Type of UIM session.
 *
 * Since: 1.6
 */
typedef enum { /*< since=1.6 >*/
    QMI_UIM_SESSION_TYPE_PRIMARY_GW_PROVISIONING    = 0,
    QMI_UIM_SESSION_TYPE_PRIMARY_1X_PROVISIONING    = 1,
    QMI_UIM_SESSION_TYPE_SECONDARY_GW_PROVISIONING  = 2,
    QMI_UIM_SESSION_TYPE_SECONDARY_1X_PROVISIONING  = 3,
    QMI_UIM_SESSION_TYPE_NONPROVISIONING_SLOT_1     = 4,
    QMI_UIM_SESSION_TYPE_NONPROVISIONING_SLOT_2     = 5,
    QMI_UIM_SESSION_TYPE_CARD_SLOT_1                = 6,
    QMI_UIM_SESSION_TYPE_CARD_SLOT_2                = 7,
    QMI_UIM_SESSION_TYPE_LOGICAL_CHANNEL_SLOT_1     = 8,
    QMI_UIM_SESSION_TYPE_LOGICAL_CHANNEL_SLOT_2     = 9,
    QMI_UIM_SESSION_TYPE_TERTIARY_GW_PROVISIONING   = 10,
    QMI_UIM_SESSION_TYPE_TERTIARY_1X_PROVISIONING   = 11,
    QMI_UIM_SESSION_TYPE_QUATERNARY_GW_PROVISIONING = 12,
    QMI_UIM_SESSION_TYPE_QUATERNARY_1X_PROVISIONING = 13,
    QMI_UIM_SESSION_TYPE_QUINARY_GW_PROVISIONING    = 14,
    QMI_UIM_SESSION_TYPE_QUINARY_1X_PROVISIONING    = 15,
    QMI_UIM_SESSION_TYPE_NONPROVISIONING_SLOT_3     = 16,
    QMI_UIM_SESSION_TYPE_NONPROVISIONING_SLOT_4     = 17,
    QMI_UIM_SESSION_TYPE_NONPROVISIONING_SLOT_5     = 18,
    QMI_UIM_SESSION_TYPE_CARD_SLOT_3                = 19,
    QMI_UIM_SESSION_TYPE_CARD_SLOT_4                = 20,
    QMI_UIM_SESSION_TYPE_CARD_SLOT_5                = 21,
    QMI_UIM_SESSION_TYPE_LOGICAL_CHANNEL_SLOT_3     = 22,
    QMI_UIM_SESSION_TYPE_LOGICAL_CHANNEL_SLOT_4     = 23,
    QMI_UIM_SESSION_TYPE_LOGICAL_CHANNEL_SLOT_5     = 24,
} QmiUimSessionType;

/*****************************************************************************/
/* Helper enums for the 'QMI UIM Get File Attributes' request/response */

/**
 * QmiUimFileType:
 * @QMI_UIM_FILE_TYPE_TRANSPARENT: Transparent.
 * @QMI_UIM_FILE_TYPE_CYCLIC: Cyclic.
 * @QMI_UIM_FILE_TYPE_LINEAR_FIXED: Linear fixed.
 * @QMI_UIM_FILE_TYPE_DEDICATED_FILE: Dedicated file.
 * @QMI_UIM_FILE_TYPE_MASTER_FILE: Master file.
 *
 * Type of UIM file.
 *
 * Since: 1.6
 */
typedef enum { /*< since=1.6 >*/
    QMI_UIM_FILE_TYPE_TRANSPARENT    = 0,
    QMI_UIM_FILE_TYPE_CYCLIC         = 1,
    QMI_UIM_FILE_TYPE_LINEAR_FIXED   = 2,
    QMI_UIM_FILE_TYPE_DEDICATED_FILE = 3,
    QMI_UIM_FILE_TYPE_MASTER_FILE    = 4
} QmiUimFileType;

/**
 * QmiUimSecurityAttributeLogic:
 * @QMI_UIM_SECURITY_ATTRIBUTE_LOGIC_ALWAYS: Always.
 * @QMI_UIM_SECURITY_ATTRIBUTE_LOGIC_NEVER: Never.
 * @QMI_UIM_SECURITY_ATTRIBUTE_LOGIC_AND: And.
 * @QMI_UIM_SECURITY_ATTRIBUTE_LOGIC_OR: Or.
 * @QMI_UIM_SECURITY_ATTRIBUTE_LOGIC_SINGLE: Single.
 *
 * Logic applicable to security attributes.
 *
 * Since: 1.6
 */
typedef enum { /*< since=1.6 >*/
    QMI_UIM_SECURITY_ATTRIBUTE_LOGIC_ALWAYS = 0,
    QMI_UIM_SECURITY_ATTRIBUTE_LOGIC_NEVER  = 1,
    QMI_UIM_SECURITY_ATTRIBUTE_LOGIC_AND    = 2,
    QMI_UIM_SECURITY_ATTRIBUTE_LOGIC_OR     = 3,
    QMI_UIM_SECURITY_ATTRIBUTE_LOGIC_SINGLE = 4
} QmiUimSecurityAttributeLogic;

/**
 * QmiUimSecurityAttribute:
 * @QMI_UIM_SECURITY_ATTRIBUTE_PIN1: PIN1.
 * @QMI_UIM_SECURITY_ATTRIBUTE_PIN2: PIN2.
 * @QMI_UIM_SECURITY_ATTRIBUTE_UPIN: UPIN.
 * @QMI_UIM_SECURITY_ATTRIBUTE_ADM: ADM.
 *
 * Security Attributes.
 *
 * Since: 1.6
 */
typedef enum { /*< since=1.6 >*/
    QMI_UIM_SECURITY_ATTRIBUTE_PIN1 = 1 << 0,
    QMI_UIM_SECURITY_ATTRIBUTE_PIN2 = 1 << 1,
    QMI_UIM_SECURITY_ATTRIBUTE_UPIN = 1 << 2,
    QMI_UIM_SECURITY_ATTRIBUTE_ADM  = 1 << 3
} QmiUimSecurityAttribute;

/*****************************************************************************/
/* Helper enums for the 'QMI UIM Set PIN Protection' */

/**
 * QmiUimPinId:
 * @QMI_UIM_PIN_ID_UNKNOWN: Unknown.
 * @QMI_UIM_PIN_ID_PIN1: PIN1.
 * @QMI_UIM_PIN_ID_PIN2: PIN2.
 * @QMI_UIM_PIN_ID_UPIN: UPIN.
 * @QMI_UIM_PIN_ID_HIDDEN_KEY: Hidden key.
 *
 * PIN ID.
 *
 * Since: 1.14
 */
typedef enum { /*< since=1.14 >*/
    QMI_UIM_PIN_ID_UNKNOWN    = 0,
    QMI_UIM_PIN_ID_PIN1       = 1,
    QMI_UIM_PIN_ID_PIN2       = 2,
    QMI_UIM_PIN_ID_UPIN       = 3,
    QMI_UIM_PIN_ID_HIDDEN_KEY = 4
} QmiUimPinId;

/*****************************************************************************/
/* Helper enums for the 'QMI UIM Get Card Status' request/response */

/**
 * QmiUimCardState:
 * @QMI_UIM_CARD_STATE_ABSENT: Absent.
 * @QMI_UIM_CARD_STATE_PRESENT: Present.
 * @QMI_UIM_CARD_STATE_ERROR: Error.
 *
 * State of the card.
 *
 * Since: 1.10
 */
typedef enum { /*< since=1.10 >*/
    QMI_UIM_CARD_STATE_ABSENT  = 0,
    QMI_UIM_CARD_STATE_PRESENT = 1,
    QMI_UIM_CARD_STATE_ERROR   = 2
} QmiUimCardState;

/**
 * QmiUimPinState:
 * @QMI_UIM_PIN_STATE_NOT_INITIALIZED: Not initialized.
 * @QMI_UIM_PIN_STATE_ENABLED_NOT_VERIFIED: Enabled, not verified.
 * @QMI_UIM_PIN_STATE_ENABLED_VERIFIED: Enabled, verified.
 * @QMI_UIM_PIN_STATE_DISABLED: Disabled.
 * @QMI_UIM_PIN_STATE_BLOCKED: Blocked.
 * @QMI_UIM_PIN_STATE_PERMANENTLY_BLOCKED: Permanently Blocked.
 *
 * The PIN state.
 *
 * Since: 1.10
 */
typedef enum { /*< since=1.10 >*/
    QMI_UIM_PIN_STATE_NOT_INITIALIZED      = 0,
    QMI_UIM_PIN_STATE_ENABLED_NOT_VERIFIED = 1,
    QMI_UIM_PIN_STATE_ENABLED_VERIFIED     = 2,
    QMI_UIM_PIN_STATE_DISABLED             = 3,
    QMI_UIM_PIN_STATE_BLOCKED              = 4,
    QMI_UIM_PIN_STATE_PERMANENTLY_BLOCKED  = 5,
} QmiUimPinState;

/**
 * QmiUimCardError:
 * @QMI_UIM_CARD_ERROR_UNKNOWN: Unknown error.
 * @QMI_UIM_CARD_ERROR_POWER_DOWN: Power down.
 * @QMI_UIM_CARD_ERROR_POLL: Poll error.
 * @QMI_UIM_CARD_ERROR_NO_ATR_RECEIVED: No ATR received.
 * @QMI_UIM_CARD_ERROR_VOLTAGE_MISMATCH: Voltage mismatch.
 * @QMI_UIM_CARD_ERROR_PARITY: Parity error.
 * @QMI_UIM_CARD_ERROR_POSSIBLY_REMOVED: Unknown error, possibly removed.
 * @QMI_UIM_CARD_ERROR_TECHNICAL: Technical problem.
 *
 * Card error.
 *
 * Since: 1.10
 */
typedef enum { /*< since=1.10 >*/
    QMI_UIM_CARD_ERROR_UNKNOWN          = 0,
    QMI_UIM_CARD_ERROR_POWER_DOWN       = 1,
    QMI_UIM_CARD_ERROR_POLL             = 2,
    QMI_UIM_CARD_ERROR_NO_ATR_RECEIVED  = 3,
    QMI_UIM_CARD_ERROR_VOLTAGE_MISMATCH = 4,
    QMI_UIM_CARD_ERROR_PARITY           = 5,
    QMI_UIM_CARD_ERROR_POSSIBLY_REMOVED = 6,
    QMI_UIM_CARD_ERROR_TECHNICAL        = 7
} QmiUimCardError;

/**
 * QmiUimCardApplicationType:
 * @QMI_UIM_CARD_APPLICATION_TYPE_UNKNOWN: Unknown.
 * @QMI_UIM_CARD_APPLICATION_TYPE_SIM: SIM.
 * @QMI_UIM_CARD_APPLICATION_TYPE_USIM: USIM.
 * @QMI_UIM_CARD_APPLICATION_TYPE_RUIM: RUIM.
 * @QMI_UIM_CARD_APPLICATION_TYPE_CSIM: CSIM.
 * @QMI_UIM_CARD_APPLICATION_TYPE_ISIM: ISIM.
 *
 * Card application type.
 *
 * Since: 1.10
 */
typedef enum { /*< since=1.10 >*/
    QMI_UIM_CARD_APPLICATION_TYPE_UNKNOWN = 0,
    QMI_UIM_CARD_APPLICATION_TYPE_SIM     = 1,
    QMI_UIM_CARD_APPLICATION_TYPE_USIM    = 2,
    QMI_UIM_CARD_APPLICATION_TYPE_RUIM    = 3,
    QMI_UIM_CARD_APPLICATION_TYPE_CSIM    = 4,
    QMI_UIM_CARD_APPLICATION_TYPE_ISIM    = 5,
} QmiUimCardApplicationType;

/**
 * QmiUimCardApplicationState:
 * @QMI_UIM_CARD_APPLICATION_STATE_UNKNOWN: Unknown.
 * @QMI_UIM_CARD_APPLICATION_STATE_DETECTED: Detected.
 * @QMI_UIM_CARD_APPLICATION_STATE_PIN1_OR_UPIN_PIN_REQUIRED: PIN1 or UPIN PIN required.
 * @QMI_UIM_CARD_APPLICATION_STATE_PUK1_OR_UPIN_PUK_REQUIRED: PUK1 or UPIN PUK required.
 * @QMI_UIM_CARD_APPLICATION_STATE_CHECK_PERSONALIZATION_STATE: Personalization state must be checked.
 * @QMI_UIM_CARD_APPLICATION_STATE_PIN1_BLOCKED: PIN1 blocked.
 * @QMI_UIM_CARD_APPLICATION_STATE_ILLEGAL: Illegal.
 * @QMI_UIM_CARD_APPLICATION_STATE_READY: Ready
 *
 * Card application state.
 *
 * Since: 1.10
 */
typedef enum { /*< since=1.10 >*/
    QMI_UIM_CARD_APPLICATION_STATE_UNKNOWN                     = 0,
    QMI_UIM_CARD_APPLICATION_STATE_DETECTED                    = 1,
    QMI_UIM_CARD_APPLICATION_STATE_PIN1_OR_UPIN_PIN_REQUIRED   = 2,
    QMI_UIM_CARD_APPLICATION_STATE_PUK1_OR_UPIN_PUK_REQUIRED   = 3,
    QMI_UIM_CARD_APPLICATION_STATE_CHECK_PERSONALIZATION_STATE = 4,
    QMI_UIM_CARD_APPLICATION_STATE_PIN1_BLOCKED                = 5,
    QMI_UIM_CARD_APPLICATION_STATE_ILLEGAL                     = 6,
    QMI_UIM_CARD_APPLICATION_STATE_READY                       = 7,
} QmiUimCardApplicationState;

/**
 * QmiUimCardApplicationPersonalizationState:
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_STATE_UNKNOWN: Unknown.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_STATE_IN_PROGRESS: Operation in progress.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_STATE_READY: Ready.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_STATE_CODE_REQUIRED: Code required.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_STATE_PUK_CODE_REQUIRED: PUK code required.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_STATE_PERMANENTLY_BLOCKED: Permanently blocked-
 *
 * Card application personalization state.
 *
 * Since: 1.10
 */
typedef enum { /*< since=1.10 >*/
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_STATE_UNKNOWN             = 0,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_STATE_IN_PROGRESS         = 1,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_STATE_READY               = 2,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_STATE_CODE_REQUIRED       = 3,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_STATE_PUK_CODE_REQUIRED   = 4,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_STATE_PERMANENTLY_BLOCKED = 5,
} QmiUimCardApplicationPersonalizationState;

/**
 * QmiUimCardApplicationPersonalizationFeature:
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_GW_NETWORK: GW network.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_GW_NETWORK_SUBSET: GW network subset.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_GW_SERVICE_PROVIDER: GW service provider.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_GW_CORPORATE: GW corporate.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_GW_UIM: UIM.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_1X_NETWORK_TYPE_1: 1X network type 1.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_1X_NETWORK_TYPE_2: 1X network type 2.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_1X_HRPD: 1X HRPD.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_1X_SERVICE_PROVIDER: 1X service provider.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_1X_CORPORATE: 1X corporate.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_1X_RUIM: 1X R-UIM.
 * @QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_UNKNOWN: Unknown.
 *
 * Card application personalization feature, when a code is required.
 *
 * Since: 1.10
 */
typedef enum { /*< since=1.10 >*/
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_GW_NETWORK          = 0,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_GW_NETWORK_SUBSET   = 1,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_GW_SERVICE_PROVIDER = 2,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_GW_CORPORATE        = 3,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_GW_UIM              = 4,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_1X_NETWORK_TYPE_1   = 5,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_1X_NETWORK_TYPE_2   = 6,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_1X_HRPD             = 7,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_1X_SERVICE_PROVIDER = 8,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_1X_CORPORATE        = 9,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_1X_RUIM             = 10,
    QMI_UIM_CARD_APPLICATION_PERSONALIZATION_FEATURE_UNKNOWN             = 11
} QmiUimCardApplicationPersonalizationFeature;

/*****************************************************************************/
/* Helper enums for the 'QMI UIM Refresh' indication */

/**
 * QmiUimRefreshStage:
 * @QMI_UIM_REFRESH_STAGE_WAIT_FOR_OK: Waiting for REFRESH OK message.
 * @QMI_UIM_REFRESH_STAGE_START: Refresh started.
 * @QMI_UIM_REFRESH_STAGE_END_WITH_SUCCESS: Refresh completed successfully.
 * @QMI_UIM_REFRESH_STAGE_END_WITH_FAILURE: Refresh has failed.
 *
 * Current stage of the refresh procedure.
 *
 * Since: 1.28
 */
typedef enum { /*< since=1.28 >*/
    QMI_UIM_REFRESH_STAGE_WAIT_FOR_OK      = 0,
    QMI_UIM_REFRESH_STAGE_START            = 1,
    QMI_UIM_REFRESH_STAGE_END_WITH_SUCCESS = 2,
    QMI_UIM_REFRESH_STAGE_END_WITH_FAILURE = 3
} QmiUimRefreshStage;

/**
 * QmiUimRefreshMode:
 * @QMI_UIM_REFRESH_MODE_RESET: Reset.
 * @QMI_UIM_REFRESH_MODE_INIT: Init.
 * @QMI_UIM_REFRESH_MODE_INIT_FCN: Init & FCN.
 * @QMI_UIM_REFRESH_MODE_FCN: FCN.
 * @QMI_UIM_REFRESH_MODE_INIT_FULL_FCN: Init & full FCN.
 * @QMI_UIM_REFRESH_MODE_APP_RESET: Application reset.
 * @QMI_UIM_REFRESH_MODE_3G_RESET: 3G session reset.
 *
 * Refresh mode
 *
 * Since: 1.28
 */
typedef enum { /*< since=1.28 >*/
    QMI_UIM_REFRESH_MODE_RESET         = 0,
    QMI_UIM_REFRESH_MODE_INIT          = 1,
    QMI_UIM_REFRESH_MODE_INIT_FCN      = 2,
    QMI_UIM_REFRESH_MODE_FCN           = 3,
    QMI_UIM_REFRESH_MODE_INIT_FULL_FCN = 4,
    QMI_UIM_REFRESH_MODE_APP_RESET     = 5,
    QMI_UIM_REFRESH_MODE_3G_RESET      = 6
} QmiUimRefreshMode;

/*****************************************************************************/
/* Helper enums for the 'QMI UIM Get Slot Status' request/response */

/**
 * QmiUimPhysicalCardState:
 * @QMI_UIM_PHYSICAL_CARD_STATE_UNKNOWN: Unknown.
 * @QMI_UIM_PHYSICAL_CARD_STATE_ABSENT: Absent.
 * @QMI_UIM_PHYSICAL_CARD_STATE_PRESENT: Present.
 *
 * State of the physical card.
 *
 * Since: 1.26
 */
typedef enum { /*< since=1.26 >*/
    QMI_UIM_PHYSICAL_CARD_STATE_UNKNOWN = 0,
    QMI_UIM_PHYSICAL_CARD_STATE_ABSENT  = 1,
    QMI_UIM_PHYSICAL_CARD_STATE_PRESENT = 2,
} QmiUimPhysicalCardState;

/**
 * QmiUimSlotState:
 * @QMI_UIM_SLOT_STATE_INACTIVE: Inactive.
 * @QMI_UIM_SLOT_STATE_ACTIVE: Active.
 *
 * State of the slot.
 *
 * Since: 1.26
 */
typedef enum { /*< since=1.26 >*/
    QMI_UIM_SLOT_STATE_INACTIVE = 0,
    QMI_UIM_SLOT_STATE_ACTIVE   = 1,
} QmiUimSlotState;

/**
 * QmiUimCardProtocol:
 * @QMI_UIM_CARD_PROTOCOL_UNKNOWN: Unknown.
 * @QMI_UIM_CARD_PROTOCOL_ICC: ICC protocol.
 * @QMI_UIM_CARD_PROTOCOL_UICC: UICC protocol.
 *
 * Protocol for the card.
 *
 * Since: 1.26
 */
typedef enum { /*< since=1.26 >*/
    QMI_UIM_CARD_PROTOCOL_UNKNOWN = 0,
    QMI_UIM_CARD_PROTOCOL_ICC     = 1,
    QMI_UIM_CARD_PROTOCOL_UICC    = 2,
} QmiUimCardProtocol;

/**
 * QmiUimConfiguration:
 * @QMI_UIM_CONFIGURATION_AUTOMATIC_SELECTION: Automatic selection.
 * @QMI_UIM_CONFIGURATION_PERSONALIZATION_STATUS: Personalization status.
 * @QMI_UIM_CONFIGURATION_HALT_SUBSCRIPTION: Halt publication of subscription.
 *
 * Requested configurations. If none explicitly requested, all configuration
 * items are returned.
 *
 * Since: 1.30
 */
typedef enum { /*< since=1.30 >*/
    QMI_UIM_CONFIGURATION_AUTOMATIC_SELECTION    = 1 << 0,
    QMI_UIM_CONFIGURATION_PERSONALIZATION_STATUS = 1 << 1,
    QMI_UIM_CONFIGURATION_HALT_SUBSCRIPTION      = 1 << 2,
} QmiUimConfiguration;

/**
 * QmiUimDepersonalizationOperation:
 * @QMI_UIM_DEPERSONALIZATION_OPERATION_DEACTIVATE: Deactivate personalization
 * @QMI_UIM_DEPERSONALIZATION_OPERATION_UNBLOCK: Unblock personalization
 *
 * Depersonalization operation to perform.
 *
 * Since: 1.30
 */
typedef enum { /*< since=1.30 >*/
    QMI_UIM_DEPERSONALIZATION_OPERATION_DEACTIVATE = 0,
    QMI_UIM_DEPERSONALIZATION_OPERATION_UNBLOCK    = 1,
} QmiUimDepersonalizationOperation;

#endif /* _LIBQMI_GLIB_QMI_ENUMS_UIM_H_ */
