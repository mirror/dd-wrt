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
 * Copyright (C) 2012 Google, Inc.
 * Copyright (C) 2012-2017 Aleksander Morgado <aleksander@aleksander.es>
 */

#ifndef _LIBQMI_GLIB_QMI_ENUMS_H_
#define _LIBQMI_GLIB_QMI_ENUMS_H_

/**
 * SECTION: qmi-enums
 *
 * This section defines common enumerations and flags used in the interface.
 */

/**
 * QmiService:
 * @QMI_SERVICE_UNKNOWN: Unknown service.
 * @QMI_SERVICE_CTL: Control service.
 * @QMI_SERVICE_WDS: Wireless Data Service.
 * @QMI_SERVICE_DMS: Device Management Service.
 * @QMI_SERVICE_NAS: Network Access Service.
 * @QMI_SERVICE_QOS: Quality Of Service service.
 * @QMI_SERVICE_WMS: Wireless Messaging Service.
 * @QMI_SERVICE_PDS: Position Determination Service.
 * @QMI_SERVICE_AUTH: Authentication service.
 * @QMI_SERVICE_AT: AT service.
 * @QMI_SERVICE_VOICE: Voice service.
 * @QMI_SERVICE_CAT2: Card Application Toolkit service (v2).
 * @QMI_SERVICE_UIM: User Identity Module service.
 * @QMI_SERVICE_PBM: Phonebook Management service.
 * @QMI_SERVICE_QCHAT: QCHAT service. Since: 1.8.
 * @QMI_SERVICE_RMTFS: Remote file system service.
 * @QMI_SERVICE_TEST: Test service. Since: 1.8.
 * @QMI_SERVICE_LOC: Location service (~ PDS v2).
 * @QMI_SERVICE_SAR: Service access proxy service.
 * @QMI_SERVICE_IMS: IMS settings service. Since: 1.8.
 * @QMI_SERVICE_ADC: Analog to digital converter driver service. Since: 1.8.
 * @QMI_SERVICE_CSD: Core sound driver service. Since: 1.8.
 * @QMI_SERVICE_MFS: Modem embedded file system service. Since: 1.8.
 * @QMI_SERVICE_TIME: Time service. Since: 1.8.
 * @QMI_SERVICE_TS: Thermal sensors service. Since: 1.8.
 * @QMI_SERVICE_TMD: Thermal mitigation device service. Since: 1.8.
 * @QMI_SERVICE_SAP: Service access proxy service. Since: 1.8.
 * @QMI_SERVICE_WDA: Wireless data administrative service. Since: 1.8.
 * @QMI_SERVICE_TSYNC: TSYNC control service. Since: 1.8.
 * @QMI_SERVICE_RFSA: Remote file system access service. Since: 1.8.
 * @QMI_SERVICE_CSVT: Circuit switched videotelephony service. Since: 1.8.
 * @QMI_SERVICE_QCMAP: Qualcomm mobile access point service. Since: 1.8.
 * @QMI_SERVICE_IMSP: IMS presence service. Since: 1.8.
 * @QMI_SERVICE_IMSVT: IMS videotelephony service. Since: 1.8.
 * @QMI_SERVICE_IMSA: IMS application service. Since: 1.8.
 * @QMI_SERVICE_COEX: Coexistence service. Since: 1.8.
 * @QMI_SERVICE_PDC: Persistent device configuration service. Since: 1.8.
 * @QMI_SERVICE_STX: Simultaneous transmit service. Since: 1.8.
 * @QMI_SERVICE_BIT: Bearer independent transport service. Since: 1.8.
 * @QMI_SERVICE_IMSRTP: IMS RTP service. Since: 1.8.
 * @QMI_SERVICE_RFRPE: RF radiated performance enhancement service. Since: 1.8.
 * @QMI_SERVICE_DSD: Data system determination service. Since: 1.8.
 * @QMI_SERVICE_SSCTL: Subsystem control service. Since: 1.8.
 * @QMI_SERVICE_DPM: Data Port Mapper service. Since: 1.30.
 * @QMI_SERVICE_CAT: Card Application Toolkit service (v1).
 * @QMI_SERVICE_RMS: Remote Management Service.
 * @QMI_SERVICE_OMA: Open Mobile Alliance device management service.
 * @QMI_SERVICE_FOTA: Firmware Over The Air service. Since: 1.24.
 * @QMI_SERVICE_GMS: Telit General Modem Service. Since: 1.24.
 * @QMI_SERVICE_GAS: Telit General Application Service. Since: 1.24.
 *
 * QMI services.
 *
 * Since: 1.0
 */
typedef enum { /*< since=1.0 >*/
    QMI_SERVICE_UNKNOWN = -1,
    QMI_SERVICE_CTL     = 0x00,
    QMI_SERVICE_WDS     = 0x01,
    QMI_SERVICE_DMS     = 0x02,
    QMI_SERVICE_NAS     = 0x03,
    QMI_SERVICE_QOS     = 0x04,
    QMI_SERVICE_WMS     = 0x05,
    QMI_SERVICE_PDS     = 0x06,
    QMI_SERVICE_AUTH    = 0x07,
    QMI_SERVICE_AT      = 0x08,
    QMI_SERVICE_VOICE   = 0x09,
    QMI_SERVICE_CAT2    = 0x0A,
    QMI_SERVICE_UIM     = 0x0B,
    QMI_SERVICE_PBM     = 0x0C,
    QMI_SERVICE_QCHAT   = 0x0D,
    QMI_SERVICE_RMTFS   = 0x0E,
    QMI_SERVICE_TEST    = 0x0F,
    QMI_SERVICE_LOC     = 0x10,
    QMI_SERVICE_SAR     = 0x11,
    QMI_SERVICE_IMS     = 0x12,
    QMI_SERVICE_ADC     = 0x13,
    QMI_SERVICE_CSD     = 0x14,
    QMI_SERVICE_MFS     = 0x15,
    QMI_SERVICE_TIME    = 0x16,
    QMI_SERVICE_TS      = 0x17,
    QMI_SERVICE_TMD     = 0x18,
    QMI_SERVICE_SAP     = 0x19,
    QMI_SERVICE_WDA     = 0x1A,
    QMI_SERVICE_TSYNC   = 0x1B,
    QMI_SERVICE_RFSA    = 0x1C,
    QMI_SERVICE_CSVT    = 0x1D,
    QMI_SERVICE_QCMAP   = 0x1E,
    QMI_SERVICE_IMSP    = 0x1F,
    QMI_SERVICE_IMSVT   = 0x20,
    QMI_SERVICE_IMSA    = 0x21,
    QMI_SERVICE_COEX    = 0x22,
    /* 0x23, reserved */
    QMI_SERVICE_PDC     = 0x24,
    /* 0x25, reserved */
    QMI_SERVICE_STX     = 0x26,
    QMI_SERVICE_BIT     = 0x27,
    QMI_SERVICE_IMSRTP  = 0x28,
    QMI_SERVICE_RFRPE   = 0x29,
    QMI_SERVICE_DSD     = 0x2A,
    QMI_SERVICE_SSCTL   = 0x2B,
    QMI_SERVICE_DPM     = 0x2F,
    QMI_SERVICE_CAT     = 0xE0,
    QMI_SERVICE_RMS     = 0xE1,
    QMI_SERVICE_OMA     = 0xE2,
    QMI_SERVICE_FOTA    = 0xE6,
    QMI_SERVICE_GMS     = 0xE7,
    QMI_SERVICE_GAS     = 0xE8,
} QmiService;

/**
 * QmiEndian:
 * @QMI_ENDIAN_LITTLE: Little endian.
 * @QMI_ENDIAN_BIG: Big endian.
 *
 * Type of endianness.
 *
 * Since: 1.0
 */
typedef enum { /*< since=1.28 >*/   /* the get_string() helper and QmiEndian type added in 1.28 */
    QMI_ENDIAN_LITTLE = 0,
    QMI_ENDIAN_BIG    = 1
} QmiEndian;

/**
 * QmiDataEndpointType:
 * @QMI_DATA_ENDPOINT_TYPE_UNKNOWN: Unknown. Since 1.30.
 * @QMI_DATA_ENDPOINT_TYPE_HSIC: High-speed inter-chip interface. Since 1.30.
 * @QMI_DATA_ENDPOINT_TYPE_HSUSB: High-speed USB.
 * @QMI_DATA_ENDPOINT_TYPE_PCIE: PCIe. Since: 1.28.
 * @QMI_DATA_ENDPOINT_TYPE_EMBEDDED: Embedded. Since 1.28.
 * @QMI_DATA_ENDPOINT_TYPE_BAM_DMUX: BAM/DMUX. Since 1.30.
 * @QMI_DATA_ENDPOINT_TYPE_UNDEFINED: Undefined.
 *
 * Data Endpoint Type.
 *
 * Since: 1.18
 */
typedef enum { /*< since=1.18 >*/
    QMI_DATA_ENDPOINT_TYPE_UNKNOWN   = 0x00,
    QMI_DATA_ENDPOINT_TYPE_HSIC      = 0x01,
    QMI_DATA_ENDPOINT_TYPE_HSUSB     = 0x02,
    QMI_DATA_ENDPOINT_TYPE_PCIE      = 0x03,
    QMI_DATA_ENDPOINT_TYPE_EMBEDDED  = 0x04,
    QMI_DATA_ENDPOINT_TYPE_BAM_DMUX  = 0x05,
    QMI_DATA_ENDPOINT_TYPE_UNDEFINED = 0xFF,
} QmiDataEndpointType;

/**
 * QmiSioPort:
 * @QMI_SIO_PORT_NONE: Invalid port number.
 * @QMI_SIO_PORT_A2_MUX_RMNET0: A2 MUX (BAM-DMUX) port for rmnet0.
 * @QMI_SIO_PORT_A2_MUX_RMNET1: A2 MUX (BAM-DMUX) port for rmnet1.
 * @QMI_SIO_PORT_A2_MUX_RMNET2: A2 MUX (BAM-DMUX) port for rmnet2.
 * @QMI_SIO_PORT_A2_MUX_RMNET3: A2 MUX (BAM-DMUX) port for rmnet3.
 * @QMI_SIO_PORT_A2_MUX_RMNET4: A2 MUX (BAM-DMUX) port for rmnet4.
 * @QMI_SIO_PORT_A2_MUX_RMNET5: A2 MUX (BAM-DMUX) port for rmnet5.
 * @QMI_SIO_PORT_A2_MUX_RMNET6: A2 MUX (BAM-DMUX) port for rmnet6.
 * @QMI_SIO_PORT_A2_MUX_RMNET7: A2 MUX (BAM-DMUX) port for rmnet7.
 *
 * SIO (serial I/O) port numbers. All ports available in the modem have a SIO
 * port number. This enum is incomplete, only few port numbers are publicly
 * known.
 *
 * Since: 1.28
 */
typedef enum { /*< since=1.28 >*/
    QMI_SIO_PORT_NONE          = 0x0000,
    QMI_SIO_PORT_A2_MUX_RMNET0 = 0x0e04,
    QMI_SIO_PORT_A2_MUX_RMNET1 = 0x0e05,
    QMI_SIO_PORT_A2_MUX_RMNET2 = 0x0e06,
    QMI_SIO_PORT_A2_MUX_RMNET3 = 0x0e07,
    QMI_SIO_PORT_A2_MUX_RMNET4 = 0x0e08,
    QMI_SIO_PORT_A2_MUX_RMNET5 = 0x0e09,
    QMI_SIO_PORT_A2_MUX_RMNET6 = 0x0e0a,
    QMI_SIO_PORT_A2_MUX_RMNET7 = 0x0e0b,
} QmiSioPort;

#endif /* _LIBQMI_GLIB_QMI_ENUMS_H_ */
