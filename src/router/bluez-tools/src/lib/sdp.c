/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2001-2002  Ricky Yuen <ryuen@qualcomm.com>
 *  Copyright (C) 2003-2007  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2010       Alexander Orlenko <zxteam@gmail.com>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "sdp.h"

/* UUID name lookup table */
typedef struct {
	int   uuid;
	char* name;
} sdp_uuid_nam_lookup_table_t;

static sdp_uuid_nam_lookup_table_t sdp_uuid_nam_lookup_table[] = {
	{ SDP_UUID_SDP,                      "SDP"          },
	{ SDP_UUID_UDP,                      "UDP"          },
	{ SDP_UUID_RFCOMM,                   "RFCOMM"       },
	{ SDP_UUID_TCP,                      "TCP"          },
	{ SDP_UUID_TCS_BIN,                  "TCS-BIN"      },
	{ SDP_UUID_TCS_AT,                   "TCS-AT"       },
	{ SDP_UUID_OBEX,                     "OBEX"         },
	{ SDP_UUID_IP,                       "IP"           },
	{ SDP_UUID_FTP,                      "FTP"          },
	{ SDP_UUID_HTTP,                     "HTTP"         },
	{ SDP_UUID_WSP,                      "WSP"          },
	{ SDP_UUID_L2CAP,                    "L2CAP"        },
	{ SDP_UUID_BNEP,                     "BNEP"         }, /* PAN */
	{ SDP_UUID_HIDP,                     "HIDP"         }, /* HID */
	{ SDP_UUID_AVCTP,                    "AVCTP"        }, /* AVCTP */
	{ SDP_UUID_AVDTP,                    "AVDTP"        }, /* AVDTP */
	{ SDP_UUID_CMTP,                     "CMTP"         }, /* CIP */
	{ SDP_UUID_UDI_C_PLANE,              "UDI_C-Plane"  }, /* UDI */
	{ SDP_UUID_SERVICE_DISCOVERY_SERVER, "SDServer"     },
	{ SDP_UUID_BROWSE_GROUP_DESCRIPTOR,  "BrwsGrpDesc"  },
	{ SDP_UUID_PUBLIC_BROWSE_GROUP,      "PubBrwsGrp"   },
	{ SDP_UUID_SERIAL_PORT,              "SP"           },
	{ SDP_UUID_LAN_ACCESS_PPP,           "LAN"          },
	{ SDP_UUID_DIALUP_NETWORKING,        "DUN"          },
	{ SDP_UUID_IR_MC_SYNC,               "IRMCSync"     },
	{ SDP_UUID_OBEX_OBJECT_PUSH,         "OBEXObjPush"  },
	{ SDP_UUID_OBEX_FILE_TRANSFER,       "OBEXObjTrnsf" },
	{ SDP_UUID_IR_MC_SYNC_COMMAND,       "IRMCSyncCmd"  },
	{ SDP_UUID_HEADSET,                  "Headset"      },
	{ SDP_UUID_CORDLESS_TELEPHONY,       "CordlessTel"  },
	{ SDP_UUID_AUDIO_SOURCE,             "AudioSource"  }, /* A2DP */
	{ SDP_UUID_AUDIO_SINK,               "AudioSink"    }, /* A2DP */
	{ SDP_UUID_AV_REMOTE_TARGET,         "AVRemTarget"  }, /* AVRCP */
	{ SDP_UUID_ADVANCED_AUDIO,           "AdvAudio"     }, /* A2DP */
	{ SDP_UUID_AV_REMOTE,                "AVRemote"     }, /* AVRCP */
	{ SDP_UUID_VIDEO_CONFERENCING,       "VideoConf"    }, /* VCP */
	{ SDP_UUID_INTERCOM,                 "Intercom"     },
	{ SDP_UUID_FAX,                      "Fax"          },
	{ SDP_UUID_HEADSET_AUDIO_GATEWAY,    "Headset AG"   },
	{ SDP_UUID_WAP,                      "WAP"          },
	{ SDP_UUID_WAP_CLIENT,               "WAP Client"   },
	{ SDP_UUID_PANU,                     "PANU"         }, /* PAN */
	{ SDP_UUID_NAP,                      "NAP"          }, /* PAN */
	{ SDP_UUID_GN,                       "GN"           }, /* PAN */
	{ SDP_UUID_DIRECT_PRINTING,          "DirectPrint"  }, /* BPP */
	{ SDP_UUID_REFERENCE_PRINTING,       "RefPrint"     }, /* BPP */
	{ SDP_UUID_IMAGING,                  "Imaging"      }, /* BIP */
	{ SDP_UUID_IMAGING_RESPONDER,        "ImagingResp"  }, /* BIP */
	{ SDP_UUID_HANDSFREE,                "Handsfree"    },
	{ SDP_UUID_HANDSFREE_AUDIO_GATEWAY,  "Handsfree AG" },
	{ SDP_UUID_DIRECT_PRINTING_REF_OBJS, "RefObjsPrint" }, /* BPP */
	{ SDP_UUID_REFLECTED_UI,             "ReflectedUI"  }, /* BPP */
	{ SDP_UUID_BASIC_PRINTING,           "BasicPrint"   }, /* BPP */
	{ SDP_UUID_PRINTING_STATUS,          "PrintStatus"  }, /* BPP */
	{ SDP_UUID_HUMAN_INTERFACE_DEVICE,   "HID"          }, /* HID */
	{ SDP_UUID_HARDCOPY_CABLE_REPLACE,   "HCRP"         }, /* HCRP */
	{ SDP_UUID_HCR_PRINT,                "HCRPrint"     }, /* HCRP */
	{ SDP_UUID_HCR_SCAN,                 "HCRScan"      }, /* HCRP */
	{ SDP_UUID_COMMON_ISDN_ACCESS,       "CIP"          }, /* CIP */
	{ SDP_UUID_VIDEO_CONFERENCING_GW,    "VideoConf GW" }, /* VCP */
	{ SDP_UUID_UDI_MT,                   "UDI MT"       }, /* UDI */
	{ SDP_UUID_UDI_TA,                   "UDI TA"       }, /* UDI */
	{ SDP_UUID_AUDIO_VIDEO,              "AudioVideo"   }, /* VCP */
	{ SDP_UUID_SIM_ACCESS,               "SAP"          }, /* SAP */
	{ SDP_UUID_PHONEBOOK_ACCESS_PCE,     "PBAP PCE"     }, /* PBAP */
	{ SDP_UUID_PHONEBOOK_ACCESS_PSE,     "PBAP PSE"     }, /* PBAP */
	{ SDP_UUID_PHONEBOOK_ACCESS,         "PBAP"         }, /* PBAP */
	{ SDP_UUID_PNP_INFORMATION,          "PNPInfo"      },
	{ SDP_UUID_GENERIC_NETWORKING,       "Networking"   },
	{ SDP_UUID_GENERIC_FILE_TRANSFER,    "FileTrnsf"    },
	{ SDP_UUID_GENERIC_AUDIO,            "Audio"        },
	{ SDP_UUID_GENERIC_TELEPHONY,        "Telephony"    },
	{ SDP_UUID_UPNP_SERVICE,             "UPNP"         }, /* ESDP */
	{ SDP_UUID_UPNP_IP_SERVICE,          "UPNP IP"      }, /* ESDP */
	{ SDP_UUID_ESDP_UPNP_IP_PAN,         "UPNP PAN"     }, /* ESDP */
	{ SDP_UUID_ESDP_UPNP_IP_LAP,         "UPNP LAP"     }, /* ESDP */
	{ SDP_UUID_ESDP_UPNP_L2CAP,          "UPNP L2CAP"   }, /* ESDP */
	{ SDP_UUID_VIDEO_SOURCE,             "VideoSource"  }, /* VDP */
	{ SDP_UUID_VIDEO_SINK,               "VideoSink"    }, /* VDP */
	{ SDP_UUID_VIDEO_DISTRIBUTION,       "VideoDist"    }, /* VDP */
	{ SDP_UUID_APPLE_AGENT,              "AppleAgent"   },
};

#define SDP_UUID_NAM_LOOKUP_TABLE_SIZE \
	(sizeof(sdp_uuid_nam_lookup_table)/sizeof(sdp_uuid_nam_lookup_table_t))

/* AttrID name lookup table */
typedef struct {
	int   attr_id;
	char* name;
} sdp_attr_id_nam_lookup_table_t;

static sdp_attr_id_nam_lookup_table_t sdp_attr_id_nam_lookup_table[] = {
	{ SDP_ATTR_ID_SERVICE_RECORD_HANDLE,             "SrvRecHndl"         },
	{ SDP_ATTR_ID_SERVICE_CLASS_ID_LIST,             "SrvClassIDList"     },
	{ SDP_ATTR_ID_SERVICE_RECORD_STATE,              "SrvRecState"        },
	{ SDP_ATTR_ID_SERVICE_SERVICE_ID,                "SrvID"              },
	{ SDP_ATTR_ID_PROTOCOL_DESCRIPTOR_LIST,          "ProtocolDescList"   },
	{ SDP_ATTR_ID_BROWSE_GROUP_LIST,                 "BrwGrpList"         },
	{ SDP_ATTR_ID_LANGUAGE_BASE_ATTRIBUTE_ID_LIST,   "LangBaseAttrIDList" },
	{ SDP_ATTR_ID_SERVICE_INFO_TIME_TO_LIVE,         "SrvInfoTimeToLive"  },
	{ SDP_ATTR_ID_SERVICE_AVAILABILITY,              "SrvAvail"           },
	{ SDP_ATTR_ID_BLUETOOTH_PROFILE_DESCRIPTOR_LIST, "BTProfileDescList"  },
	{ SDP_ATTR_ID_DOCUMENTATION_URL,                 "DocURL"             },
	{ SDP_ATTR_ID_CLIENT_EXECUTABLE_URL,             "ClientExeURL"       },
	{ SDP_ATTR_ID_ICON_10,                           "Icon10"             },
	{ SDP_ATTR_ID_ICON_URL,                          "IconURL"            },
	{ SDP_ATTR_ID_SERVICE_NAME,                      "SrvName"            },
	{ SDP_ATTR_ID_SERVICE_DESCRIPTION,               "SrvDesc"            },
	{ SDP_ATTR_ID_PROVIDER_NAME,                     "ProviderName"       },
	{ SDP_ATTR_ID_VERSION_NUMBER_LIST,               "VersionNumList"     },
	{ SDP_ATTR_ID_GROUP_ID,                          "GrpID"              },
	{ SDP_ATTR_ID_SERVICE_DATABASE_STATE,            "SrvDBState"         },
	{ SDP_ATTR_ID_SERVICE_VERSION,                   "SrvVersion"         },
	{ SDP_ATTR_ID_SECURITY_DESCRIPTION,              "SecurityDescription"}, /* PAN */
	{ SDP_ATTR_ID_SUPPORTED_DATA_STORES_LIST,        "SuppDataStoresList" }, /* Synchronization */
	{ SDP_ATTR_ID_SUPPORTED_FORMATS_LIST,            "SuppFormatsList"    }, /* OBEX Object Push */
	{ SDP_ATTR_ID_NET_ACCESS_TYPE,                   "NetAccessType"      }, /* PAN */
	{ SDP_ATTR_ID_MAX_NET_ACCESS_RATE,               "MaxNetAccessRate"   }, /* PAN */
	{ SDP_ATTR_ID_IPV4_SUBNET,                       "IPv4Subnet"         }, /* PAN */
	{ SDP_ATTR_ID_IPV6_SUBNET,                       "IPv6Subnet"         }, /* PAN */
	{ SDP_ATTR_ID_SUPPORTED_CAPABILITIES,            "SuppCapabilities"   }, /* Imaging */
	{ SDP_ATTR_ID_SUPPORTED_FEATURES,                "SuppFeatures"       }, /* Imaging and Hansfree */
	{ SDP_ATTR_ID_SUPPORTED_FUNCTIONS,               "SuppFunctions"      }, /* Imaging */
	{ SDP_ATTR_ID_TOTAL_IMAGING_DATA_CAPACITY,       "SuppTotalCapacity"  }, /* Imaging */
	{ SDP_ATTR_ID_SUPPORTED_REPOSITORIES,            "SuppRepositories"   }, /* PBAP */
};

#define SDP_ATTR_ID_NAM_LOOKUP_TABLE_SIZE \
	(sizeof(sdp_attr_id_nam_lookup_table)/sizeof(sdp_attr_id_nam_lookup_table_t))

const char* sdp_get_uuid_name(int uuid)
{
	for (int i = 0; i < SDP_UUID_NAM_LOOKUP_TABLE_SIZE; i++) {
		if (sdp_uuid_nam_lookup_table[i].uuid == uuid)
			return sdp_uuid_nam_lookup_table[i].name;
	}

	return NULL;
}

const char* sdp_get_attr_id_name(int attr_id)
{
	for (int i = 0; i < SDP_ATTR_ID_NAM_LOOKUP_TABLE_SIZE; i++)
		if (sdp_attr_id_nam_lookup_table[i].attr_id == attr_id)
			return sdp_attr_id_nam_lookup_table[i].name;

	return NULL;
}

