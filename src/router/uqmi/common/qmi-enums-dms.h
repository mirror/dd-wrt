/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * uqmi -- tiny QMI support implementation
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
 * Copyright (C) 2012 Lanedo GmbH.
 * Copyright (C) 2012-2017 Aleksander Morgado <aleksander@aleksander.es>
 */

#ifndef _LIBQMI_GLIB_QMI_ENUMS_DMS_H_
#define _LIBQMI_GLIB_QMI_ENUMS_DMS_H_

/**
 * SECTION: qmi-enums-dms
 *
 * This section defines enumerations and flags used in the DMS service
 * interface.
 */

/*****************************************************************************/
/* Helper enums for the 'QMI DMS Get Capabilities' message */

/**
 * QmiDmsDataServiceCapability:
 * @QMI_DMS_DATA_SERVICE_CAPABILITY_NONE: No data services supported.
 * @QMI_DMS_DATA_SERVICE_CAPABILITY_CS: Only CS supported.
 * @QMI_DMS_DATA_SERVICE_CAPABILITY_PS: Only PS supported.
 * @QMI_DMS_DATA_SERVICE_CAPABILITY_SIMULTANEOUS_CS_PS: Simultaneous CS and PS supported.
 * @QMI_DMS_DATA_SERVICE_CAPABILITY_NON_SIMULTANEOUS_CS_PS: Non simultaneous CS and PS supported.
 *
 * Data service capability.
 *
 * Since: 1.0
 */
typedef enum { /*< since=1.0 >*/
    QMI_DMS_DATA_SERVICE_CAPABILITY_NONE                   = 0,
    QMI_DMS_DATA_SERVICE_CAPABILITY_CS                     = 1,
    QMI_DMS_DATA_SERVICE_CAPABILITY_PS                     = 2,
    QMI_DMS_DATA_SERVICE_CAPABILITY_SIMULTANEOUS_CS_PS     = 3,
    QMI_DMS_DATA_SERVICE_CAPABILITY_NON_SIMULTANEOUS_CS_PS = 4
} QmiDmsDataServiceCapability;

/**
 * QmiDmsSimCapability:
 * @QMI_DMS_SIM_CAPABILITY_NOT_SUPPORTED: SIM not supported.
 * @QMI_DMS_SIM_CAPABILITY_SUPPORTED: SIM is supported.
 *
 * SIM capability.
 *
 * Since: 1.0
 */
typedef enum { /*< since=1.0 >*/
    QMI_DMS_SIM_CAPABILITY_NOT_SUPPORTED = 1,
    QMI_DMS_SIM_CAPABILITY_SUPPORTED     = 2
} QmiDmsSimCapability;

/**
 * QmiDmsRadioInterface:
 * @QMI_DMS_RADIO_INTERFACE_CDMA20001X: CDMA2000 1x.
 * @QMI_DMS_RADIO_INTERFACE_EVDO: CDMA2000 HRPD (1xEV-DO)
 * @QMI_DMS_RADIO_INTERFACE_GSM: GSM.
 * @QMI_DMS_RADIO_INTERFACE_UMTS: UMTS.
 * @QMI_DMS_RADIO_INTERFACE_LTE: LTE.
 * @QMI_DMS_RADIO_INTERFACE_TDS: TDS. Since 1.32.
 * @QMI_DMS_RADIO_INTERFACE_5GNR: 5G NR. Since 1.26.
 *
 * Radio interface type.
 *
 * Since: 1.0
 */
typedef enum { /*< since=1.0 >*/
    QMI_DMS_RADIO_INTERFACE_CDMA20001X = 1,
    QMI_DMS_RADIO_INTERFACE_EVDO       = 2,
    QMI_DMS_RADIO_INTERFACE_GSM        = 4,
    QMI_DMS_RADIO_INTERFACE_UMTS       = 5,
    QMI_DMS_RADIO_INTERFACE_LTE        = 8,
    QMI_DMS_RADIO_INTERFACE_TDS        = 9,
    QMI_DMS_RADIO_INTERFACE_5GNR       = 10,
} QmiDmsRadioInterface;

/*****************************************************************************/
/* Helper enums for the 'QMI DMS Get Power State' message */

/**
 * QmiDmsPowerState:
 * @QMI_DMS_POWER_STATE_EXTERNAL_SOURCE: Powered by an external source.
 * @QMI_DMS_POWER_STATE_BATTERY_CONNECTED: Battery is connected.
 * @QMI_DMS_POWER_STATE_BATTERY_CHARGING: Battery is currently being charged.
 * @QMI_DMS_POWER_STATE_FAULT: Recognized power fault.
 *
 * Flags specifying the current power state.
 *
 * If @QMI_DMS_POWER_STATE_EXTERNAL_SOURCE is set, the device is powerered by an
 * external source; otherwise it is powered by a battery.
 *
 * If @QMI_DMS_POWER_STATE_BATTERY_CONNECTED is set, the battery is connected;
 * otherwise the battery is not connected.
 *
 * If @QMI_DMS_POWER_STATE_BATTERY_CHARGING is set, the battery is being charged;
 * otherwise the battery is not being charged.
 *
 * If @QMI_DMS_POWER_STATE_FAULT is set, a power fault has been detected.
 *
 * Since: 1.0
 */
typedef enum { /*< since=1.0 >*/
    QMI_DMS_POWER_STATE_EXTERNAL_SOURCE   = 1 << 0,
    QMI_DMS_POWER_STATE_BATTERY_CONNECTED = 1 << 1,
    QMI_DMS_POWER_STATE_BATTERY_CHARGING  = 1 << 2,
    QMI_DMS_POWER_STATE_FAULT             = 1 << 3,
} QmiDmsPowerState;

/*****************************************************************************/
/* Helper enums for the 'QMI DMS UIM Set PIN Protection' message */

/**
 * QmiDmsUimPinId:
 * @QMI_DMS_UIM_PIN_ID_PIN: PIN.
 * @QMI_DMS_UIM_PIN_ID_PIN2: PIN2.
 *
 * The PIN identifier.
 *
 * Since: 1.0
 */
typedef enum { /*< since=1.0 >*/
    QMI_DMS_UIM_PIN_ID_PIN  = 1,
    QMI_DMS_UIM_PIN_ID_PIN2 = 2
} QmiDmsUimPinId;

/*****************************************************************************/
/* Helper enums for the 'QMI DMS UIM Get PIN Status' message */

/**
 * QmiDmsUimPinStatus:
 * @QMI_DMS_UIM_PIN_STATUS_NOT_INITIALIZED: Not initialized.
 * @QMI_DMS_UIM_PIN_STATUS_ENABLED_NOT_VERIFIED: Enabled, not verified.
 * @QMI_DMS_UIM_PIN_STATUS_ENABLED_VERIFIED: Enabled, verified.
 * @QMI_DMS_UIM_PIN_STATUS_DISABLED: Disabled.
 * @QMI_DMS_UIM_PIN_STATUS_BLOCKED: Blocked.
 * @QMI_DMS_UIM_PIN_STATUS_PERMANENTLY_BLOCKED: Permanently Blocked.
 * @QMI_DMS_UIM_PIN_STATUS_UNBLOCKED: Unblocked.
 * @QMI_DMS_UIM_PIN_STATUS_CHANGED: Changed.
 *
 * The PIN status.
 *
 * Since: 1.0
 */
typedef enum { /*< since=1.0 >*/
    QMI_DMS_UIM_PIN_STATUS_NOT_INITIALIZED      = 0,
    QMI_DMS_UIM_PIN_STATUS_ENABLED_NOT_VERIFIED = 1,
    QMI_DMS_UIM_PIN_STATUS_ENABLED_VERIFIED     = 2,
    QMI_DMS_UIM_PIN_STATUS_DISABLED             = 3,
    QMI_DMS_UIM_PIN_STATUS_BLOCKED              = 4,
    QMI_DMS_UIM_PIN_STATUS_PERMANENTLY_BLOCKED  = 5,
    QMI_DMS_UIM_PIN_STATUS_UNBLOCKED            = 6,
    QMI_DMS_UIM_PIN_STATUS_CHANGED              = 7,
} QmiDmsUimPinStatus;

/*****************************************************************************/
/* Helper enums for the 'QMI DMS Get Operating Mode' message */

/**
 * QmiDmsOperatingMode:
 * @QMI_DMS_OPERATING_MODE_ONLINE: Device can acquire a system and make calls.
 * @QMI_DMS_OPERATING_MODE_LOW_POWER: Device has temporarily disabled RF.
 * @QMI_DMS_OPERATING_MODE_PERSISTENT_LOW_POWER: Device has disabled RF and state persists even after a reset.
 * @QMI_DMS_OPERATING_MODE_FACTORY_TEST: Special mode for manufacturer tests.
 * @QMI_DMS_OPERATING_MODE_OFFLINE: Device has deactivated RF and is partially shutdown.
 * @QMI_DMS_OPERATING_MODE_RESET: Device is in the process of power cycling.
 * @QMI_DMS_OPERATING_MODE_SHUTTING_DOWN: Device is in the process of shutting down.
 * @QMI_DMS_OPERATING_MODE_MODE_ONLY_LOW_POWER: Mode-only Low Power.
 * @QMI_DMS_OPERATING_MODE_UNKNOWN: Unknown.
 *
 * Operating mode of the device.
 *
 * Since: 1.0
 */
typedef enum { /*< since=1.0 >*/
    QMI_DMS_OPERATING_MODE_ONLINE                = 0,
    QMI_DMS_OPERATING_MODE_LOW_POWER             = 1,
    QMI_DMS_OPERATING_MODE_FACTORY_TEST          = 2,
    QMI_DMS_OPERATING_MODE_OFFLINE               = 3,
    QMI_DMS_OPERATING_MODE_RESET                 = 4,
    QMI_DMS_OPERATING_MODE_SHUTTING_DOWN         = 5,
    QMI_DMS_OPERATING_MODE_PERSISTENT_LOW_POWER  = 6,
    QMI_DMS_OPERATING_MODE_MODE_ONLY_LOW_POWER   = 7,
    QMI_DMS_OPERATING_MODE_UNKNOWN               = 0xFF
} QmiDmsOperatingMode;

/**
 * QmiDmsOfflineReason:
 * @QMI_DMS_OFFLINE_REASON_HOST_IMAGE_MISCONFIGURATION: Host image misconfiguration.
 * @QMI_DMS_OFFLINE_REASON_PRI_IMAGE_MISCONFIGURATION: PRI image misconfiguration.
 * @QMI_DMS_OFFLINE_REASON_PRI_VERSION_INCOMPATIBLE: PRI version incompatible.
 * @QMI_DMS_OFFLINE_REASON_DEVICE_MEMORY_FULL: Memory full, cannot copy PRI information.
 *
 * Reasons for being in Offline (@QMI_DMS_OPERATING_MODE_OFFLINE) state.
 *
 * Since: 1.0
 */
typedef enum { /*< since=1.0 >*/
    QMI_DMS_OFFLINE_REASON_HOST_IMAGE_MISCONFIGURATION = 1 << 0,
    QMI_DMS_OFFLINE_REASON_PRI_IMAGE_MISCONFIGURATION  = 1 << 1,
    QMI_DMS_OFFLINE_REASON_PRI_VERSION_INCOMPATIBLE    = 1 << 2,
    QMI_DMS_OFFLINE_REASON_DEVICE_MEMORY_FULL          = 1 << 3
} QmiDmsOfflineReason;

/*****************************************************************************/
/* Helper enums for the 'QMI DMS Get Time' message */

/**
 * QmiDmsTimeSource:
 * @QMI_DMS_TIME_SOURCE_DEVICE: 32 kHz device clock.
 * @QMI_DMS_TIME_SOURCE_CDMA_NETWORK: CDMA network.
 * @QMI_DMS_TIME_SOURCE_HDR_NETWORK: HDR network.
 *
 * Source of the timestamp.
 *
 * Since: 1.0
 */
typedef enum { /*< since=1.0 >*/
    QMI_DMS_TIME_SOURCE_DEVICE        = 0,
    QMI_DMS_TIME_SOURCE_CDMA_NETWORK  = 1,
    QMI_DMS_TIME_SOURCE_HDR_NETWORK   = 2,
} QmiDmsTimeSource;

/*****************************************************************************/
/* Helper enums for the 'QMI DMS Get Activation State' message */

/**
 * QmiDmsActivationState:
 * @QMI_DMS_ACTIVATION_STATE_NOT_ACTIVATED: Service not activated.
 * @QMI_DMS_ACTIVATION_STATE_ACTIVATED: Service is activated.
 * @QMI_DMS_ACTIVATION_STATE_CONNECTING: Connection in progress for automatic activation.
 * @QMI_DMS_ACTIVATION_STATE_CONNECTED: Connection connected for automatic activation.
 * @QMI_DMS_ACTIVATION_STATE_OTASP_AUTHENTICATED: OTASP security authenticated.
 * @QMI_DMS_ACTIVATION_STATE_OTASP_NAM: OTASP NAM downloaded.
 * @QMI_DMS_ACTIVATION_STATE_OTASP_MDN: OTASP MDN downloaded.
 * @QMI_DMS_ACTIVATION_STATE_OTASP_IMSI: OTASP IMSI downloaded.
 * @QMI_DMS_ACTIVATION_STATE_OTASP_PRL: OTASP PRL downloaded.
 * @QMI_DMS_ACTIVATION_STATE_OTASP_SPC: OTASP SPC downloaded.
 * @QMI_DMS_ACTIVATION_STATE_OTASP_COMMITED: OTASP settings committed.
 *
 * State of the service activation.
 *
 * Since: 1.0
 */
typedef enum { /*< since=1.0 >*/
    QMI_DMS_ACTIVATION_STATE_NOT_ACTIVATED       = 0x00,
    QMI_DMS_ACTIVATION_STATE_ACTIVATED           = 0x01,
    QMI_DMS_ACTIVATION_STATE_CONNECTING          = 0x02,
    QMI_DMS_ACTIVATION_STATE_CONNECTED           = 0x03,
    QMI_DMS_ACTIVATION_STATE_OTASP_AUTHENTICATED = 0x04,
    QMI_DMS_ACTIVATION_STATE_OTASP_NAM           = 0x05,
    QMI_DMS_ACTIVATION_STATE_OTASP_MDN           = 0x06,
    QMI_DMS_ACTIVATION_STATE_OTASP_IMSI          = 0x07,
    QMI_DMS_ACTIVATION_STATE_OTASP_PRL           = 0x08,
    QMI_DMS_ACTIVATION_STATE_OTASP_SPC           = 0x09,
    QMI_DMS_ACTIVATION_STATE_OTASP_COMMITED      = 0x0A
} QmiDmsActivationState;

/*****************************************************************************/
/* Helper enums for the 'QMI DMS UIM Get CK Status' message */

/**
 * QmiDmsUimFacility:
 * @QMI_DMS_UIM_FACILITY_PN: Network personalization facility.
 * @QMI_DMS_UIM_FACILITY_PU: Network subset personalization facility.
 * @QMI_DMS_UIM_FACILITY_PP: Service provider facility.
 * @QMI_DMS_UIM_FACILITY_PC: Corporate personalization facility.
 * @QMI_DMS_UIM_FACILITY_PF: UIM personalization facility.
 *
 * UIM personalization facilities.
 *
 * Since: 1.0
 */
typedef enum { /*< since=1.0 >*/
    QMI_DMS_UIM_FACILITY_PN = 0,
    QMI_DMS_UIM_FACILITY_PU = 1,
    QMI_DMS_UIM_FACILITY_PP = 2,
    QMI_DMS_UIM_FACILITY_PC = 3,
    QMI_DMS_UIM_FACILITY_PF = 4
} QmiDmsUimFacility;

/**
 * QmiDmsUimFacilityState:
 * @QMI_DMS_UIM_FACILITY_STATE_DEACTIVATED: Facility is deactivated.
 * @QMI_DMS_UIM_FACILITY_STATE_ACTIVATED: Facility is activated.
 * @QMI_DMS_UIM_FACILITY_STATE_BLOCKED: Facility is blocked.
 *
 * State of the UIM facility.
 *
 * Since: 1.0
 */
typedef enum { /*< since=1.0 >*/
    QMI_DMS_UIM_FACILITY_STATE_DEACTIVATED = 0,
    QMI_DMS_UIM_FACILITY_STATE_ACTIVATED   = 1,
    QMI_DMS_UIM_FACILITY_STATE_BLOCKED     = 2
} QmiDmsUimFacilityState;

/*****************************************************************************/
/* Helper enums for the 'QMI DMS UIM Get State' message */

/**
 * QmiDmsUimState:
 * @QMI_DMS_UIM_STATE_INITIALIZATION_COMPLETED: UIM initialization completed.
 * @QMI_DMS_UIM_STATE_LOCKED_OR_FAILED: UIM is locked or failed.
 * @QMI_DMS_UIM_STATE_NOT_PRESENT: No UIM in the device.
 * @QMI_DMS_UIM_STATE_RESERVED: Reserved, unknown.
 * @QMI_DMS_UIM_STATE_UNKNOWN: UIM state currently unavailable.
 *
 * State of the UIM.
 *
 * Since: 1.0
 */
typedef enum { /*< since=1.0 >*/
    QMI_DMS_UIM_STATE_INITIALIZATION_COMPLETED = 0x00,
    QMI_DMS_UIM_STATE_LOCKED_OR_FAILED         = 0x01,
    QMI_DMS_UIM_STATE_NOT_PRESENT              = 0x02,
    QMI_DMS_UIM_STATE_RESERVED                 = 0x03,
    QMI_DMS_UIM_STATE_UNKNOWN                  = 0xFF
} QmiDmsUimState;

/*****************************************************************************/
/* Helper enums for the 'QMI DMS Set Time' message */

/**
 * QmiDmsTimeReferenceType:
 * @QMI_DMS_TIME_REFERENCE_TYPE_USER: User time.
 *
 * Time reference type.
 *
 * Since: 1.0
 */
typedef enum { /*< since=1.0 >*/
    QMI_DMS_TIME_REFERENCE_TYPE_USER = 0
} QmiDmsTimeReferenceType;

/*****************************************************************************/
/* Helper enums for the 'QMI DMS Get Firmware Preference' message */

/**
 * QmiDmsFirmwareImageType:
 * @QMI_DMS_FIRMWARE_IMAGE_TYPE_MODEM: Modem image.
 * @QMI_DMS_FIRMWARE_IMAGE_TYPE_PRI: PRI image.
 *
 * Type of firmware image.
 *
 * Since: 1.0
 */
typedef enum { /*< since=1.0 >*/
    QMI_DMS_FIRMWARE_IMAGE_TYPE_MODEM = 0,
    QMI_DMS_FIRMWARE_IMAGE_TYPE_PRI   = 1
} QmiDmsFirmwareImageType;

/*****************************************************************************/
/* Helper enums for the 'QMI DMS Get Boot Image Download Mode' message */

/**
 * QmiDmsBootImageDownloadMode:
 * @QMI_DMS_BOOT_IMAGE_DOWNLOAD_MODE_NORMAL: Normal operation.
 * @QMI_DMS_BOOT_IMAGE_DOWNLOAD_MODE_BOOT_AND_RECOVERY: Boot and recovery image download mode.
 *
 * Specifies the mode for the next boot.
 *
 * Since: 1.18
 */
typedef enum { /*< since=1.18 >*/
    QMI_DMS_BOOT_IMAGE_DOWNLOAD_MODE_NORMAL            = 0,
    QMI_DMS_BOOT_IMAGE_DOWNLOAD_MODE_BOOT_AND_RECOVERY = 1,
} QmiDmsBootImageDownloadMode;

/*****************************************************************************/
/* Helper enums for the 'QMI DMS Get MAC Address' message */

/**
 * QmiDmsMacType:
 * @QMI_DMS_MAC_TYPE_WLAN: WLAN MAC address.
 * @QMI_DMS_MAC_TYPE_BT: Bluetooth MAC address.
 *
 * Specifies the device from which the MAC address should be queried.
 *
 * Since: 1.26
 */
typedef enum { /*< since=1.26 >*/
    QMI_DMS_MAC_TYPE_WLAN = 0,
    QMI_DMS_MAC_TYPE_BT = 1,
} QmiDmsMacType;

/*****************************************************************************/
/* Helper enums for the 'QMI DMS HP Change Device Mode' message */

/**
 * QmiDmsHpDeviceMode:
 * @QMI_DMS_HP_DEVICE_MODE_FASTBOOT: Fastboot download mode.
 *
 * HP specific device modes.
 *
 * Since: 1.18
 */
typedef enum { /*< since=1.18 >*/
    QMI_DMS_HP_DEVICE_MODE_FASTBOOT = 5,
} QmiDmsHpDeviceMode;

/*****************************************************************************/
/* Helper enums for the 'QMI DMS Swi Get USB Composition' message */

/**
 * QmiDmsSwiUsbComposition:
 * @QMI_DMS_SWI_USB_COMPOSITION_UNKNOWN: Unknown.
 * @QMI_DMS_SWI_USB_COMPOSITION_0: HIP, DM, NMEA, AT, MDM1, MDM2, MDM3, MS.
 * @QMI_DMS_SWI_USB_COMPOSITION_1: HIP, DM, NMEA, AT, MDM1, MS.
 * @QMI_DMS_SWI_USB_COMPOSITION_2: HIP, DM, NMEA, AT, NIC1, MS.
 * @QMI_DMS_SWI_USB_COMPOSITION_3: HIP, DM, NMEA, AT, MDM1, NIC1, MS.
 * @QMI_DMS_SWI_USB_COMPOSITION_4: HIP, DM, NMEA, AT, NIC1, NIC2, NIC3, MS.
 * @QMI_DMS_SWI_USB_COMPOSITION_5: HIP, DM, NMEA, AT, ECM1, MS.
 * @QMI_DMS_SWI_USB_COMPOSITION_6: DM, NMEA, AT, QMI.
 * @QMI_DMS_SWI_USB_COMPOSITION_7: DM, NMEA, AT, RMNET1, RMNET2, RMNET3.
 * @QMI_DMS_SWI_USB_COMPOSITION_8: DM, NMEA, AT, MBIM.
 * @QMI_DMS_SWI_USB_COMPOSITION_9: MBIM.
 * @QMI_DMS_SWI_USB_COMPOSITION_10: NMEA, MBIM.
 * @QMI_DMS_SWI_USB_COMPOSITION_11: DM, MBIM.
 * @QMI_DMS_SWI_USB_COMPOSITION_12: DM, NMEA, MBIM.
 * @QMI_DMS_SWI_USB_COMPOSITION_13: Dual configuration: USB composition 6 and USB composition 8.
 * @QMI_DMS_SWI_USB_COMPOSITION_14: Dual configuration: USB composition 6 and USB composition 9.
 * @QMI_DMS_SWI_USB_COMPOSITION_15: Dual configuration: USB composition 6 and USB composition 10.
 * @QMI_DMS_SWI_USB_COMPOSITION_16: Dual configuration: USB composition 6 and USB composition 11.
 * @QMI_DMS_SWI_USB_COMPOSITION_17: Dual configuration: USB composition 6 and USB composition 12.
 * @QMI_DMS_SWI_USB_COMPOSITION_18: Dual configuration: USB composition 7 and USB composition 8.
 * @QMI_DMS_SWI_USB_COMPOSITION_19: Dual configuration: USB composition 7 and USB composition 9.
 * @QMI_DMS_SWI_USB_COMPOSITION_20: Dual configuration: USB composition 7 and USB composition 10.
 * @QMI_DMS_SWI_USB_COMPOSITION_21: Dual configuration: USB composition 7 and USB composition 11.
 * @QMI_DMS_SWI_USB_COMPOSITION_22: Dual configuration: USB composition 7 and USB composition 12.
 *
 * Sierra Wireless USB composition modes.
 *
 * Since: 1.20
 */
typedef enum { /*< since=1.20 >*/
    QMI_DMS_SWI_USB_COMPOSITION_UNKNOWN = -1,
    QMI_DMS_SWI_USB_COMPOSITION_0  =  0,
    QMI_DMS_SWI_USB_COMPOSITION_1  =  1,
    QMI_DMS_SWI_USB_COMPOSITION_2  =  2,
    QMI_DMS_SWI_USB_COMPOSITION_3  =  3,
    QMI_DMS_SWI_USB_COMPOSITION_4  =  4,
    QMI_DMS_SWI_USB_COMPOSITION_5  =  5,
    QMI_DMS_SWI_USB_COMPOSITION_6  =  6,
    QMI_DMS_SWI_USB_COMPOSITION_7  =  7,
    QMI_DMS_SWI_USB_COMPOSITION_8  =  8,
    QMI_DMS_SWI_USB_COMPOSITION_9  =  9,
    QMI_DMS_SWI_USB_COMPOSITION_10 = 10,
    QMI_DMS_SWI_USB_COMPOSITION_11 = 11,
    QMI_DMS_SWI_USB_COMPOSITION_12 = 12,
    QMI_DMS_SWI_USB_COMPOSITION_13 = 13,
    QMI_DMS_SWI_USB_COMPOSITION_14 = 14,
    QMI_DMS_SWI_USB_COMPOSITION_15 = 15,
    QMI_DMS_SWI_USB_COMPOSITION_16 = 16,
    QMI_DMS_SWI_USB_COMPOSITION_17 = 17,
    QMI_DMS_SWI_USB_COMPOSITION_18 = 18,
    QMI_DMS_SWI_USB_COMPOSITION_19 = 19,
    QMI_DMS_SWI_USB_COMPOSITION_20 = 20,
    QMI_DMS_SWI_USB_COMPOSITION_21 = 21,
    QMI_DMS_SWI_USB_COMPOSITION_22 = 22,
} QmiDmsSwiUsbComposition;

/**
 * qmi_dms_swi_usb_composition_get_description:
 * @value: a #QmiDmsSwiUsbComposition.
 *
 * Gets a text description of the Sierra Wireless USB composition.
 *
 * Since: 1.20
 * Returns: a string.
 */
const char *qmi_dms_swi_usb_composition_get_description (QmiDmsSwiUsbComposition value);

/*****************************************************************************/
/* Helper enums for the 'QMI DMS Foxconn Change Device Mode' message */

/**
 * QmiDmsFoxconnDeviceMode:
 * @QMI_DMS_FOXCONN_DEVICE_MODE_UNKNOWN: Unknown mode.
 * @QMI_DMS_FOXCONN_DEVICE_MODE_FASTBOOT_ONLINE: Fastboot download mode for full partition files.
 * @QMI_DMS_FOXCONN_DEVICE_MODE_FASTBOOT_OTA: Fastboot download mode for OTA files.
 *
 * Foxconn specific device modes.
 *
 * Since: 1.26
 */
typedef enum { /*< since=1.26 >*/
    QMI_DMS_FOXCONN_DEVICE_MODE_UNKNOWN         = 0,
    QMI_DMS_FOXCONN_DEVICE_MODE_FASTBOOT_ONLINE = 0x05,
    QMI_DMS_FOXCONN_DEVICE_MODE_FASTBOOT_OTA    = 0x0A,
} QmiDmsFoxconnDeviceMode;

/*****************************************************************************/
/* Helper enums for the 'QMI DMS Foxconn Get Firmware Version' message */

/**
 * QmiDmsFoxconnFirmwareVersionType:
 * @QMI_DMS_FOXCONN_FIRMWARE_VERSION_TYPE_FIRMWARE_MCFG: E.g. T77W968.F0.0.0.2.3.GC.004.
 * @QMI_DMS_FOXCONN_FIRMWARE_VERSION_TYPE_FIRMWARE_MCFG_APPS: E.g. T77W968.F0.0.0.2.3.GC.004.011.
 * @QMI_DMS_FOXCONN_FIRMWARE_VERSION_TYPE_APPS: E.g. 011.
 *
 * Foxconn specific firmware version types.
 *
 * Since: 1.26
 */
typedef enum { /*< since=1.26 >*/
    QMI_DMS_FOXCONN_FIRMWARE_VERSION_TYPE_FIRMWARE_MCFG      = 0x00,
    QMI_DMS_FOXCONN_FIRMWARE_VERSION_TYPE_FIRMWARE_MCFG_APPS = 0x01,
    QMI_DMS_FOXCONN_FIRMWARE_VERSION_TYPE_APPS               = 0x02,
} QmiDmsFoxconnFirmwareVersionType;

#endif /* _LIBQMI_GLIB_QMI_ENUMS_DMS_H_ */
