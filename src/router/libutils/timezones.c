/*
 * timezones.c
 *
 * Copyright (C) 2014 Sebastian Gottschall <gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */

#include <utils.h>
#include <stdlib.h>
#include <bcmnvram.h>

TIMEZONE_TO_TZSTRING allTimezones[] = {
	{"Africa/Abidjan", "GMT0"}
	,
	{"Africa/Accra", "GMT0"}
	,
	{"Africa/Addis_Ababa", "EAT-3"}
	,
	{"Africa/Algiers", "CET-1"}
	,
	{"Africa/Asmara", "EAT-3"}
	,
	{"Africa/Asmera", "EAT-3"}
	,
	{"Africa/Bamako", "GMT0"}
	,
	{"Africa/Bangui", "WAT-1"}
	,
	{"Africa/Banjul", "GMT0"}
	,
	{"Africa/Bissau", "GMT0"}
	,
	{"Africa/Blantyre", "CAT-2"}
	,
	{"Africa/Brazzaville", "WAT-1"}
	,
	{"Africa/Bujumbura", "CAT-2"}
	,
	{"Africa/Cairo", "EET-2EEST,M4.5.5/0,M9.5.4/24"}
	,
	{"Africa/Casablanca", "WET0WEST,M3.5.0,M10.5.0/3"}
	,
	{"Africa/Ceuta", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Africa/Conakry", "GMT0"}
	,
	{"Africa/Dakar", "GMT0"}
	,
	{"Africa/Dar_es_Salaam", "EAT-3"}
	,
	{"Africa/Djibouti", "EAT-3"}
	,
	{"Africa/Douala", "WAT-1"}
	,
	{"Africa/El_Aaiun", "WET0WEST,M3.5.0,M10.5.0/3"}
	,
	{"Africa/Freetown", "GMT0"}
	,
	{"Africa/Gaborone", "CAT-2"}
	,
	{"Africa/Harare", "CAT-2"}
	,
	{"Africa/Johannesburg", "SAST-2"}
	,
	{"Africa/Juba", "EAT-3"}
	,
	{"Africa/Kampala", "EAT-3"}
	,
	{"Africa/Khartoum", "EAT-3"}
	,
	{"Africa/Kigali", "CAT-2"}
	,
	{"Africa/Kinshasa", "WAT-1"}
	,
	{"Africa/Lagos", "WAT-1"}
	,
	{"Africa/Libreville", "WAT-1"}
	,
	{"Africa/Lome", "GMT0"}
	,
	{"Africa/Luanda", "WAT-1"}
	,
	{"Africa/Lubumbashi", "CAT-2"}
	,
	{"Africa/Lusaka", "CAT-2"}
	,
	{"Africa/Malabo", "WAT-1"}
	,
	{"Africa/Maputo", "CAT-2"}
	,
	{"Africa/Maseru", "SAST-2"}
	,
	{"Africa/Mbabane", "SAST-2"}
	,
	{"Africa/Mogadishu", "EAT-3"}
	,
	{"Africa/Monrovia", "GMT0"}
	,
	{"Africa/Nairobi", "EAT-3"}
	,
	{"Africa/Ndjamena", "WAT-1"}
	,
	{"Africa/Niamey", "WAT-1"}
	,
	{"Africa/Nouakchott", "GMT0"}
	,
	{"Africa/Ouagadougou", "GMT0"}
	,
	{"Africa/Porto-Novo", "WAT-1"}
	,
	{"Africa/Sao_Tome", "GMT0"}
	,
	{"Africa/Timbuktu", "GMT0"}
	,
	{"Africa/Tripoli", "EET-2"}
	,
	{"Africa/Tunis", "CET-1"}
	,
	{"Africa/Windhoek", "WAT-1WAST,M9.1.0,M4.1.0"}
	,
	{"America/Adak", "HAST10HADT,M3.2.0,M11.1.0"}
	,
	{"America/Anchorage", "AKST9AKDT,M3.2.0,M11.1.0"}
	,
	{"America/Anguilla", "AST4"}
	,
	{"America/Antigua", "AST4"}
	,
	{"America/Araguaina", "BRT3"}
	,
	{"America/Argentina/Buenos_Aires", "ART3"}
	,
	{"America/Argentina/Catamarca", "ART3"}
	,
	{"America/Argentina/ComodRivadavia", "ART3"}
	,
	{"America/Argentina/Cordoba", "ART3"}
	,
	{"America/Argentina/Jujuy", "ART3"}
	,
	{"America/Argentina/La_Rioja", "ART3"}
	,
	{"America/Argentina/Mendoza", "ART3"}
	,
	{"America/Argentina/Rio_Gallegos", "ART3"}
	,
	{"America/Argentina/Salta", "ART3"}
	,
	{"America/Argentina/San_Juan", "ART3"}
	,
	{"America/Argentina/San_Luis", "ART3"}
	,
	{"America/Argentina/Tucuman", "ART3"}
	,
	{"America/Argentina/Ushuaia", "ART3"}
	,
	{"America/Aruba", "AST4"}
	,
	{"America/Asuncion", "PYT4PYST,M10.1.0/0,M3.4.0/0"}
	,
	{"America/Atikokan", "EST5"}
	,
	{"America/Atka", "HAST10HADT,M3.2.0,M11.1.0"}
	,
	{"America/Bahia", "BRT3"}
	,
	{"America/Bahia_Banderas", "CST6CDT,M4.1.0,M10.5.0"}
	,
	{"America/Barbados", "AST4"}
	,
	{"America/Belem", "BRT3"}
	,
	{"America/Belize", "CST6"}
	,
	{"America/Blanc-Sablon", "AST4"}
	,
	{"America/Boa_Vista", "AMT4"}
	,
	{"America/Bogota", "COT5"}
	,
	{"America/Boise", "MST7MDT,M3.2.0,M11.1.0"}
	,
	{"America/Buenos_Aires", "ART3"}
	,
	{"America/Cambridge_Bay", "MST7MDT,M3.2.0,M11.1.0"}
	,
	{"America/Campo_Grande", "AMT4AMST,M10.3.0/0,M2.3.0/0"}
	,
	{"America/Cancun", "CST6CDT,M4.1.0,M10.5.0"}
	,
	{"America/Caracas", "VET4:30"}
	,
	{"America/Catamarca", "ART3"}
	,
	{"America/Cayenne", "GFT3"}
	,
	{"America/Cayman", "EST5"}
	,
	{"America/Chicago", "CST6CDT,M3.2.0,M11.1.0"}
	,
	{"America/Chihuahua", "MST7MDT,M4.1.0,M10.5.0"}
	,
	{"America/Coral_Harbour", "EST5"}
	,
	{"America/Cordoba", "ART3"}
	,
	{"America/Costa_Rica", "CST6"}
	,
	{"America/Creston", "MST7"}
	,
	{"America/Cuiaba", "AMT4AMST,M10.3.0/0,M2.3.0/0"}
	,
	{"America/Curacao", "AST4"}
	,
	{"America/Danmarkshavn", "GMT0"}
	,
	{"America/Dawson", "PST8PDT,M3.2.0,M11.1.0"}
	,
	{"America/Dawson_Creek", "MST7"}
	,
	{"America/Denver", "MST7MDT,M3.2.0,M11.1.0"}
	,
	{"America/Detroit", "EST5EDT,M3.2.0,M11.1.0"}
	,
	{"America/Dominica", "AST4"}
	,
	{"America/Edmonton", "MST7MDT,M3.2.0,M11.1.0"}
	,
	{"America/Eirunepe", "ACT5"}
	,
	{"America/El_Salvador", "CST6"}
	,
	{"America/Ensenada", "PST8PDT,M3.2.0,M11.1.0"}
	,
	{"America/Fort_Wayne", "EST5EDT,M3.2.0,M11.1.0"}
	,
	{"America/Fortaleza", "BRT3"}
	,
	{"America/Glace_Bay", "AST4ADT,M3.2.0,M11.1.0"}
	,
	{"America/Godthab", "WGT3WGST,M3.5.0/-2,M10.5.0/-1"}
	,
	{"America/Goose_Bay", "AST4ADT,M3.2.0,M11.1.0"}
	,
	{"America/Grand_Turk", "AST4"}
	,
	{"America/Grenada", "AST4"}
	,
	{"America/Guadeloupe", "AST4"}
	,
	{"America/Guatemala", "CST6"}
	,
	{"America/Guayaquil", "ECT5"}
	,
	{"America/Guyana", "GYT4"}
	,
	{"America/Halifax", "AST4ADT,M3.2.0,M11.1.0"}
	,
	{"America/Havana", "CST5CDT,M3.2.0/0,M11.1.0/1"}
	,
	{"America/Hermosillo", "MST7"}
	,
	{"America/Indiana/Indianapolis", "EST5EDT,M3.2.0,M11.1.0"}
	,
	{"America/Indiana/Knox", "CST6CDT,M3.2.0,M11.1.0"}
	,
	{"America/Indiana/Marengo", "EST5EDT,M3.2.0,M11.1.0"}
	,
	{"America/Indiana/Petersburg", "EST5EDT,M3.2.0,M11.1.0"}
	,
	{"America/Indiana/Tell_City", "CST6CDT,M3.2.0,M11.1.0"}
	,
	{"America/Indiana/Vevay", "EST5EDT,M3.2.0,M11.1.0"}
	,
	{"America/Indiana/Vincennes", "EST5EDT,M3.2.0,M11.1.0"}
	,
	{"America/Indiana/Winamac", "EST5EDT,M3.2.0,M11.1.0"}
	,
	{"America/Indianapolis", "EST5EDT,M3.2.0,M11.1.0"}
	,
	{"America/Inuvik", "MST7MDT,M3.2.0,M11.1.0"}
	,
	{"America/Iqaluit", "EST5EDT,M3.2.0,M11.1.0"}
	,
	{"America/Jamaica", "EST5"}
	,
	{"America/Jujuy", "ART3"}
	,
	{"America/Juneau", "AKST9AKDT,M3.2.0,M11.1.0"}
	,
	{"America/Kentucky/Louisville", "EST5EDT,M3.2.0,M11.1.0"}
	,
	{"America/Kentucky/Monticello", "EST5EDT,M3.2.0,M11.1.0"}
	,
	{"America/Knox_IN", "CST6CDT,M3.2.0,M11.1.0"}
	,
	{"America/Kralendijk", "AST4"}
	,
	{"America/La_Paz", "BOT4"}
	,
	{"America/Lima", "PET5"}
	,
	{"America/Los_Angeles", "PST8PDT,M3.2.0,M11.1.0"}
	,
	{"America/Louisville", "EST5EDT,M3.2.0,M11.1.0"}
	,
	{"America/Lower_Princes", "AST4"}
	,
	{"America/Maceio", "BRT3"}
	,
	{"America/Managua", "CST6"}
	,
	{"America/Manaus", "AMT4"}
	,
	{"America/Marigot", "AST4"}
	,
	{"America/Martinique", "AST4"}
	,
	{"America/Matamoros", "CST6CDT,M3.2.0,M11.1.0"}
	,
	{"America/Mazatlan", "MST7MDT,M4.1.0,M10.5.0"}
	,
	{"America/Mendoza", "ART3"}
	,
	{"America/Menominee", "CST6CDT,M3.2.0,M11.1.0"}
	,
	{"America/Merida", "CST6CDT,M4.1.0,M10.5.0"}
	,
	{"America/Metlakatla", "PST8"}
	,
	{"America/Mexico_City", "CST6CDT,M4.1.0,M10.5.0"}
	,
	{"America/Miquelon", "PMST3PMDT,M3.2.0,M11.1.0"}
	,
	{"America/Moncton", "AST4ADT,M3.2.0,M11.1.0"}
	,
	{"America/Monterrey", "CST6CDT,M4.1.0,M10.5.0"}
	,
	{"America/Montevideo", "UYT3UYST,M10.1.0,M3.2.0"}
	,
	{"America/Montreal", "EST5EDT,M3.2.0,M11.1.0"}
	,
	{"America/Montserrat", "AST4"}
	,
	{"America/Nassau", "EST5EDT,M3.2.0,M11.1.0"}
	,
	{"America/New_York", "EST5EDT,M3.2.0,M11.1.0"}
	,
	{"America/Nipigon", "EST5EDT,M3.2.0,M11.1.0"}
	,
	{"America/Nome", "AKST9AKDT,M3.2.0,M11.1.0"}
	,
	{"America/Noronha", "FNT2"}
	,
	{"America/North_Dakota/Beulah", "CST6CDT,M3.2.0,M11.1.0"}
	,
	{"America/North_Dakota/Center", "CST6CDT,M3.2.0,M11.1.0"}
	,
	{"America/North_Dakota/New_Salem", "CST6CDT,M3.2.0,M11.1.0"}
	,
	{"America/Ojinaga", "MST7MDT,M3.2.0,M11.1.0"}
	,
	{"America/Panama", "EST5"}
	,
	{"America/Pangnirtung", "EST5EDT,M3.2.0,M11.1.0"}
	,
	{"America/Paramaribo", "SRT3"}
	,
	{"America/Phoenix", "MST7"}
	,
	{"America/Port-au-Prince", "EST5EDT,M3.2.0,M11.1.0"}
	,
	{"America/Port_of_Spain", "AST4"}
	,
	{"America/Porto_Acre", "ACT5"}
	,
	{"America/Porto_Velho", "AMT4"}
	,
	{"America/Puerto_Rico", "AST4"}
	,
	{"America/Rainy_River", "CST6CDT,M3.2.0,M11.1.0"}
	,
	{"America/Rankin_Inlet", "CST6CDT,M3.2.0,M11.1.0"}
	,
	{"America/Recife", "BRT3"}
	,
	{"America/Regina", "CST6"}
	,
	{"America/Resolute", "CST6CDT,M3.2.0,M11.1.0"}
	,
	{"America/Rio_Branco", "ACT5"}
	,
	{"America/Rosario", "ART3"}
	,
	{"America/Santa_Isabel", "PST8PDT,M4.1.0,M10.5.0"}
	,
	{"America/Santarem", "BRT3"}
	,
	{"America/Santiago", "CLT4CLST,M9.1.6/24,M4.4.6/24"}
	,
	{"America/Santo_Domingo", "AST4"}
	,
	{"America/Sao_Paulo", "BRT3BRST,M10.3.0/0,M2.3.0/0"}
	,
	{"America/Scoresbysund", "EGT1EGST,M3.5.0/0,M10.5.0/1"}
	,
	{"America/Shiprock", "MST7MDT,M3.2.0,M11.1.0"}
	,
	{"America/Sitka", "AKST9AKDT,M3.2.0,M11.1.0"}
	,
	{"America/St_Barthelemy", "AST4"}
	,
	{"America/St_Johns", "NST3:30NDT,M3.2.0,M11.1.0"}
	,
	{"America/St_Kitts", "AST4"}
	,
	{"America/St_Lucia", "AST4"}
	,
	{"America/St_Thomas", "AST4"}
	,
	{"America/St_Vincent", "AST4"}
	,
	{"America/Swift_Current", "CST6"}
	,
	{"America/Tegucigalpa", "CST6"}
	,
	{"America/Thule", "AST4ADT,M3.2.0,M11.1.0"}
	,
	{"America/Thunder_Bay", "EST5EDT,M3.2.0,M11.1.0"}
	,
	{"America/Tijuana", "PST8PDT,M3.2.0,M11.1.0"}
	,
	{"America/Toronto", "EST5EDT,M3.2.0,M11.1.0"}
	,
	{"America/Tortola", "AST4"}
	,
	{"America/Vancouver", "PST8PDT,M3.2.0,M11.1.0"}
	,
	{"America/Virgin", "AST4"}
	,
	{"America/Whitehorse", "PST8PDT,M3.2.0,M11.1.0"}
	,
	{"America/Winnipeg", "CST6CDT,M3.2.0,M11.1.0"}
	,
	{"America/Yakutat", "AKST9AKDT,M3.2.0,M11.1.0"}
	,
	{"America/Yellowknife", "MST7MDT,M3.2.0,M11.1.0"}
	,
	{"Antarctica/Casey", "AWST-8"}
	,
	{"Antarctica/Davis", "DAVT-7"}
	,
	{"Antarctica/DumontDUrville", "DDUT-10"}
	,
	{"Antarctica/Macquarie", "MIST-11"}
	,
	{"Antarctica/Mawson", "MAWT-5"}
	,
	{"Antarctica/McMurdo", "NZST-12NZDT,M9.5.0,M4.1.0/3"}
	,
	{"Antarctica/Palmer", "CLT4CLST,M9.1.6/24,M4.4.6/24"}
	,
	{"Antarctica/Rothera", "ROTT3"}
	,
	{"Antarctica/South_Pole", "NZST-12NZDT,M9.5.0,M4.1.0/3"}
	,
	{"Antarctica/Syowa", "SYOT-3"}
	,
	{"Antarctica/Troll", "UTC0CEST-2,M3.5.0/1,M10.5.0/3"}
	,
	{"Antarctica/Vostok", "VOST-6"}
	,
	{"Arctic/Longyearbyen", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Asia/Aden", "AST-3"}
	,
	{"Asia/Almaty", "ALMT-6"}
	,
	{"Asia/Amman", "EET-2EEST,M3.5.4/24,M10.5.5/1"}
	,
	{"Asia/Anadyr", "ANAT-12"}
	,
	{"Asia/Aqtau", "AQTT-5"}
	,
	{"Asia/Aqtobe", "AQTT-5"}
	,
	{"Asia/Ashgabat", "TMT-5"}
	,
	{"Asia/Ashkhabad", "TMT-5"}
	,
	{"Asia/Baghdad", "AST-3"}
	,
	{"Asia/Bahrain", "AST-3"}
	,
	{"Asia/Baku", "AZT-4AZST,M3.5.0/4,M10.5.0/5"}
	,
	{"Asia/Bangkok", "ICT-7"}
	,
	{"Asia/Beijing", "CST-8"}
	,
	{"Asia/Beirut", "EET-2EEST,M3.5.0/0,M10.5.0/0"}
	,
	{"Asia/Bishkek", "KGT-6"}
	,
	{"Asia/Brunei", "BNT-8"}
	,
	{"Asia/Calcutta", "IST-5:30"}
	,
	{"Asia/Chita", "IRKT-8"}
	,
	{"Asia/Choibalsan", "CHOT-8"}
	,
	{"Asia/Chongqing", "CST-8"}
	,
	{"Asia/Chungking", "CST-8"}
	,
	{"Asia/Colombo", "IST-5:30"}
	,
	{"Asia/Dacca", "BDT-6"}
	,
	{"Asia/Damascus", "EET-2EEST,M3.5.5/0,M10.5.5/0"}
	,
	{"Asia/Dhaka", "BDT-6"}
	,
	{"Asia/Dili", "TLT-9"}
	,
	{"Asia/Dubai", "GST-4"}
	,
	{"Asia/Dushanbe", "TJT-5"}
	,
	{"Asia/Gaza", "EET-2EEST,M3.5.4/24,M9.3.6/144"}
	,
	{"Asia/Harbin", "CST-8"}
	,
	{"Asia/Hebron", "EET-2EEST,M3.5.4/24,M9.3.6/144"}
	,
	{"Asia/Ho_Chi_Minh", "ICT-7"}
	,
	{"Asia/Hong_Kong", "HKT-8"}
	,
	{"Asia/Hovd", "HOVT-7"}
	,
	{"Asia/Irkutsk", "IRKT-8"}
	,
	{"Asia/Istanbul", "EET-2EEST,M3.5.0/3,M10.5.0/4"}
	,
	{"Asia/Jakarta", "WIB-7"}
	,
	{"Asia/Jayapura", "WIT-9"}
	,
	{"Asia/Jerusalem", "IST-2IDT,M3.4.4/26,M10.5.0"}
	,
	{"Asia/Kabul", "AFT-4:30"}
	,
	{"Asia/Kamchatka", "PETT-12"}
	,
	{"Asia/Karachi", "PKT-5"}
	,
	{"Asia/Kashgar", "XJT-6"}
	,
	{"Asia/Kathmandu", "NPT-5:45"}
	,
	{"Asia/Katmandu", "NPT-5:45"}
	,
	{"Asia/Khandyga", "YAKT-9"}
	,
	{"Asia/Kolkata", "IST-5:30"}
	,
	{"Asia/Krasnoyarsk", "KRAT-7"}
	,
	{"Asia/Kuala_Lumpur", "MYT-8"}
	,
	{"Asia/Kuching", "MYT-8"}
	,
	{"Asia/Kuwait", "AST-3"}
	,
	{"Asia/Macao", "CST-8"}
	,
	{"Asia/Macau", "CST-8"}
	,
	{"Asia/Magadan", "MAGT-10"}
	,
	{"Asia/Makassar", "WITA-8"}
	,
	{"Asia/Manila", "PHT-8"}
	,
	{"Asia/Muscat", "GST-4"}
	,
	{"Asia/Nicosia", "EET-2EEST,M3.5.0/3,M10.5.0/4"}
	,
	{"Asia/Novokuznetsk", "KRAT-7"}
	,
	{"Asia/Novosibirsk", "NOVT-6"}
	,
	{"Asia/Omsk", "OMST-6"}
	,
	{"Asia/Oral", "ORAT-5"}
	,
	{"Asia/Phnom_Penh", "ICT-7"}
	,
	{"Asia/Pontianak", "WIB-7"}
	,
	{"Asia/Pyongyang", "KST-9"}
	,
	{"Asia/Qatar", "AST-3"}
	,
	{"Asia/Qyzylorda", "QYZT-6"}
	,
	{"Asia/Rangoon", "MMT-6:30"}
	,
	{"Asia/Riyadh", "AST-3"}
	,
	{"Asia/Saigon", "ICT-7"}
	,
	{"Asia/Sakhalin", "SAKT-10"}
	,
	{"Asia/Samarkand", "UZT-5"}
	,
	{"Asia/Seoul", "KST-9"}
	,
	{"Asia/Shanghai", "CST-8"}
	,
	{"Asia/Singapore", "SGT-8"}
	,
	{"Asia/Srednekolymsk", "SRET-11"}
	,
	{"Asia/Taipei", "CST-8"}
	,
	{"Asia/Tashkent", "UZT-5"}
	,
	{"Asia/Tbilisi", "GET-4"}
	,
	{"Asia/Tehran", "IRDT"}
	,
	{"Asia/Tel_Aviv", "IST-2IDT,M3.4.4/26,M10.5.0"}
	,
	{"Asia/Thimbu", "BTT-6"}
	,
	{"Asia/Thimphu", "BTT-6"}
	,
	{"Asia/Tokyo", "JST-9"}
	,
	{"Asia/Ujung_Pandang", "WITA-8"}
	,
	{"Asia/Ulaanbaatar", "ULAT-8"}
	,
	{"Asia/Ulan_Bator", "ULAT-8"}
	,
	{"Asia/Urumqi", "XJT-6"}
	,
	{"Asia/Ust-Nera", "VLAT-10"}
	,
	{"Asia/Vientiane", "ICT-7"}
	,
	{"Asia/Vladivostok", "VLAT-10"}
	,
	{"Asia/Yakutsk", "YAKT-9"}
	,
	{"Asia/Yekaterinburg", "YEKT-5"}
	,
	{"Asia/Yerevan", "AMT-4"}
	,
	{"Atlantic/Azores", "AZOT1AZOST,M3.5.0/0,M10.5.0/1"}
	,
	{"Atlantic/Bermuda", "AST4ADT,M3.2.0,M11.1.0"}
	,
	{"Atlantic/Canary", "WET0WEST,M3.5.0/1,M10.5.0"}
	,
	{"Atlantic/Cape_Verde", "CVT1"}
	,
	{"Atlantic/Faeroe", "WET0WEST,M3.5.0/1,M10.5.0"}
	,
	{"Atlantic/Faroe", "WET0WEST,M3.5.0/1,M10.5.0"}
	,
	{"Atlantic/Jan_Mayen", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Atlantic/Madeira", "WET0WEST,M3.5.0/1,M10.5.0"}
	,
	{"Atlantic/Reykjavik", "GMT0"}
	,
	{"Atlantic/South_Georgia", "GST2"}
	,
	{"Atlantic/St_Helena", "GMT0"}
	,
	{"Atlantic/Stanley", "FKST3"}
	,
	{"Australia/ACT", "AEST-10AEDT,M10.1.0,M4.1.0/3"}
	,
	{"Australia/Adelaide", "ACST-9:30ACDT,M10.1.0,M4.1.0/3"}
	,
	{"Australia/Brisbane", "AEST-10"}
	,
	{"Australia/Broken_Hill", "ACST-9:30ACDT,M10.1.0,M4.1.0/3"}
	,
	{"Australia/Canberra", "AEST-10AEDT,M10.1.0,M4.1.0/3"}
	,
	{"Australia/Currie", "AEST-10AEDT,M10.1.0,M4.1.0/3"}
	,
	{"Australia/Darwin", "ACST-9:30"}
	,
	{"Australia/Eucla", "ACWST-8:45"}
	,
	{"Australia/Hobart", "AEST-10AEDT,M10.1.0,M4.1.0/3"}
	,
	{"Australia/LHI", "LHST-10:30LHDT-11,M10.1.0,M4.1.0"}
	,
	{"Australia/Lindeman", "AEST-10"}
	,
	{"Australia/Lord_Howe", "LHST-10:30LHDT-11,M10.1.0,M4.1.0"}
	,
	{"Australia/Melbourne", "AEST-10AEDT,M10.1.0,M4.1.0/3"}
	,
	{"Australia/NSW", "AEST-10AEDT,M10.1.0,M4.1.0/3"}
	,
	{"Australia/North", "ACST-9:30"}
	,
	{"Australia/Perth", "AWST-8"}
	,
	{"Australia/Queensland", "AEST-10"}
	,
	{"Australia/South", "ACST-9:30ACDT,M10.1.0,M4.1.0/3"}
	,
	{"Australia/Sydney", "AEST-10AEDT,M10.1.0,M4.1.0/3"}
	,
	{"Australia/Tasmania", "AEST-10AEDT,M10.1.0,M4.1.0/3"}
	,
	{"Australia/Victoria", "AEST-10AEDT,M10.1.0,M4.1.0/3"}
	,
	{"Australia/West", "AWST-8"}
	,
	{"Australia/Yancowinna", "ACST-9:30ACDT,M10.1.0,M4.1.0/3"}
	,
	{"Brazil/Acre", "ACT5"}
	,
	{"Brazil/DeNoronha", "FNT2"}
	,
	{"Brazil/East", "BRT3BRST,M10.3.0/0,M2.3.0/0"}
	,
	{"Brazil/West", "AMT4"}
	,
	{"Canada/Atlantic", "AST4ADT,M3.2.0,M11.1.0"}
	,
	{"Canada/Central", "CST6CDT,M3.2.0,M11.1.0"}
	,
	{"Canada/East-Saskatchewan", "CST6"}
	,
	{"Canada/Eastern", "EST5EDT,M3.2.0,M11.1.0"}
	,
	{"Canada/Mountain", "MST7MDT,M3.2.0,M11.1.0"}
	,
	{"Canada/Newfoundland", "NST3:30NDT,M3.2.0,M11.1.0"}
	,
	{"Canada/Pacific", "PST8PDT,M3.2.0,M11.1.0"}
	,
	{"Canada/Saskatchewan", "CST6"}
	,
	{"Canada/Yukon", "PST8PDT,M3.2.0,M11.1.0"}
	,
	{"Chile/Continental", "CLT4CLST,M9.1.6/24,M4.4.6/24"}
	,
	{"Chile/EasterIsland", "EAST6EASST,M9.1.6/22,M4.4.6/22"}
	,
	{"Cuba", "CST5CDT,M3.2.0/0,M11.1.0/1"}
	,
	{"Egypt", "EET-2EEST,M4.5.5/0,M9.5.4/24"}
	,
	{"Eire", "GMT0IST,M3.5.0/1,M10.5.0"}
	,
	{"Etc/GMT+1", "<GMT+1>1"}
	,
	{"Etc/GMT+10", "<GMT+10>10"}
	,
	{"Etc/GMT+11", "<GMT+11>11"}
	,
	{"Etc/GMT+12", "<GMT+12>12"}
	,
	{"Etc/GMT+2", "<GMT+2>2"}
	,
	{"Etc/GMT+3", "<GMT+3>3"}
	,
	{"Etc/GMT+4", "<GMT+4>4"}
	,
	{"Etc/GMT+5", "<GMT+5>5"}
	,
	{"Etc/GMT+6", "<GMT+6>6"}
	,
	{"Etc/GMT+7", "<GMT+7>7"}
	,
	{"Etc/GMT+8", "<GMT+8>8"}
	,
	{"Etc/GMT+9", "<GMT+9>9"}
	,
	{"Etc/GMT-1", "<GMT-1>-1"}
	,
	{"Etc/GMT-10", "<GMT-10>-10"}
	,
	{"Etc/GMT-11", "<GMT-11>-11"}
	,
	{"Etc/GMT-12", "<GMT-12>-12"}
	,
	{"Etc/GMT-13", "<GMT-13>-13"}
	,
	{"Etc/GMT-14", "<GMT-14>-14"}
	,
	{"Etc/GMT-2", "<GMT-2>-2"}
	,
	{"Etc/GMT-3", "<GMT-3>-3"}
	,
	{"Etc/GMT-4", "<GMT-4>-4"}
	,
	{"Etc/GMT-5", "<GMT-5>-5"}
	,
	{"Etc/GMT-6", "<GMT-6>-6"}
	,
	{"Etc/GMT-7", "<GMT-7>-7"}
	,
	{"Etc/GMT-8", "<GMT-8>-8"}
	,
	{"Etc/GMT-9", "<GMT-9>-9"}
	,
	{"Etc/Greenwich", "GMT0"}
	,
	{"Etc/Zulu", "UTC0"}
	,
	{"Europe/Amsterdam", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Europe/Andorra", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Europe/Athens", "EET-2EEST,M3.5.0/3,M10.5.0/4"}
	,
	{"Europe/Belfast", "GMT0BST,M3.5.0/1,M10.5.0"}
	,
	{"Europe/Belgrade", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Europe/Berlin", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Europe/Bratislava", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Europe/Brussels", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Europe/Bucharest", "EET-2EEST,M3.5.0/3,M10.5.0/4"}
	,
	{"Europe/Budapest", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Europe/Busingen", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Europe/Chisinau", "EET-2EEST,M3.5.0/3,M10.5.0/4"}
	,
	{"Europe/Copenhagen", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Europe/Dublin", "GMT0IST,M3.5.0/1,M10.5.0"}
	,
	{"Europe/Gibraltar", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Europe/Guernsey", "GMT0BST,M3.5.0/1,M10.5.0"}
	,
	{"Europe/Helsinki", "EET-2EEST,M3.5.0/3,M10.5.0/4"}
	,
	{"Europe/Isle_of_Man", "GMT0BST,M3.5.0/1,M10.5.0"}
	,
	{"Europe/Istanbul", "EET-2EEST,M3.5.0/3,M10.5.0/4"}
	,
	{"Europe/Jersey", "GMT0BST,M3.5.0/1,M10.5.0"}
	,
	{"Europe/Kaliningrad", "EET-2"}
	,
	{"Europe/Kiev", "EET-2EEST,M3.5.0/3,M10.5.0/4"}
	,
	{"Europe/Lisbon", "WET0WEST,M3.5.0/1,M10.5.0"}
	,
	{"Europe/Ljubljana", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Europe/London", "GMT0BST,M3.5.0/1,M10.5.0"}
	,
	{"Europe/Luxembourg", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Europe/Madrid", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Europe/Malta", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Europe/Mariehamn", "EET-2EEST,M3.5.0/3,M10.5.0/4"}
	,
	{"Europe/Minsk", "MSK-3"}
	,
	{"Europe/Monaco", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Europe/Moscow", "MSK-3"}
	,
	{"Europe/Nicosia", "EET-2EEST,M3.5.0/3,M10.5.0/4"}
	,
	{"Europe/Oslo", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Europe/Paris", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Europe/Podgorica", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Europe/Prague", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Europe/Riga", "EET-2EEST,M3.5.0/3,M10.5.0/4"}
	,
	{"Europe/Rome", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Europe/Samara", "SAMT-4"}
	,
	{"Europe/San_Marino", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Europe/Sarajevo", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Europe/Simferopol", "MSK-3"}
	,
	{"Europe/Skopje", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Europe/Sofia", "EET-2EEST,M3.5.0/3,M10.5.0/4"}
	,
	{"Europe/Stockholm", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Europe/Tallinn", "EET-2EEST,M3.5.0/3,M10.5.0/4"}
	,
	{"Europe/Tirane", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Europe/Tiraspol", "EET-2EEST,M3.5.0/3,M10.5.0/4"}
	,
	{"Europe/Uzhgorod", "EET-2EEST,M3.5.0/3,M10.5.0/4"}
	,
	{"Europe/Vaduz", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Europe/Vatican", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Europe/Vienna", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Europe/Vilnius", "EET-2EEST,M3.5.0/3,M10.5.0/4"}
	,
	{"Europe/Volgograd", "MSK-3"}
	,
	{"Europe/Warsaw", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Europe/Zagreb", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Europe/Zaporozhye", "EET-2EEST,M3.5.0/3,M10.5.0/4"}
	,
	{"Europe/Zurich", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"GB", "GMT0BST,M3.5.0/1,M10.5.0"}
	,
	{"GB-Eire", "GMT0BST,M3.5.0/1,M10.5.0"}
	,
	{"Greenwich", "GMT0"}
	,
	{"HST", "HST10"}
	,
	{"Hongkong", "HKT-8"}
	,
	{"Iceland", "GMT0"}
	,
	{"Indian/Antananarivo", "EAT-3"}
	,
	{"Indian/Chagos", "IOT-6"}
	,
	{"Indian/Christmas", "CXT-7"}
	,
	{"Indian/Cocos", "CCT-6:30"}
	,
	{"Indian/Comoro", "EAT-3"}
	,
	{"Indian/Kerguelen", "TFT-5"}
	,
	{"Indian/Mahe", "SCT-4"}
	,
	{"Indian/Maldives", "MVT-5"}
	,
	{"Indian/Mauritius", "MUT-4"}
	,
	{"Indian/Mayotte", "EAT-3"}
	,
	{"Indian/Reunion", "RET-4"}
	,
	{"Iran", "UTC-3:30,M3.5.0,M9.5.0"}
	,
	{"Israel", "IST-2IDT,M3.4.4/26,M10.5.0"}
	,
	{"Jamaica", "EST5"}
	,
	{"Japan", "JST-9"}
	,
	{"Kwajalein", "MHT-12"}
	,
	{"Libya", "EET-2"}
	,
	{"Mexico/BajaNorte", "PST8PDT,M3.2.0,M11.1.0"}
	,
	{"Mexico/BajaSur", "MST7MDT,M4.1.0,M10.5.0"}
	,
	{"Mexico/General", "CST6CDT,M4.1.0,M10.5.0"}
	,
	{"NZ", "NZST-12NZDT,M9.5.0,M4.1.0/3"}
	,
	{"NZ-CHAT", "CHAST-12:45CHADT,M9.5.0/2:45,M4.1.0/3:45"}
	,
	{"Navajo", "MST7MDT,M3.2.0,M11.1.0"}
	,
	{"PRC", "CST-8"}
	,
	{"Pacific/Apia", "WSST-13WSDT,M9.5.0/3,M4.1.0/4"}
	,
	{"Pacific/Auckland", "NZST-12NZDT,M9.5.0,M4.1.0/3"}
	,
	{"Pacific/Bougainville", "BST-11"}
	,
	{"Pacific/Chatham", "CHAST-12:45CHADT,M9.5.0/2:45,M4.1.0/3:45"}
	,
	{"Pacific/Chuuk", "CHUT-10"}
	,
	{"Pacific/Easter", "EAST6EASST,M9.1.6/22,M4.4.6/22"}
	,
	{"Pacific/Efate", "VUT-11"}
	,
	{"Pacific/Enderbury", "PHOT-13"}
	,
	{"Pacific/Fakaofo", "TKT-13"}
	,
	{"Pacific/Fiji", "FJT-12FJST,M11.1.0,M1.3.4/75"}
	,
	{"Pacific/Funafuti", "TVT-12"}
	,
	{"Pacific/Galapagos", "GALT6"}
	,
	{"Pacific/Gambier", "GAMT9"}
	,
	{"Pacific/Guadalcanal", "SBT-11"}
	,
	{"Pacific/Guam", "ChST-10"}
	,
	{"Pacific/Honolulu", "HST10"}
	,
	{"Pacific/Johnston", "HST10"}
	,
	{"Pacific/Kiritimati", "LINT-14"}
	,
	{"Pacific/Kosrae", "KOST-11"}
	,
	{"Pacific/Kwajalein", "MHT-12"}
	,
	{"Pacific/Majuro", "MHT-12"}
	,
	{"Pacific/Marquesas", "MART9:30"}
	,
	{"Pacific/Midway", "SST11"}
	,
	{"Pacific/Nauru", "NRT-12"}
	,
	{"Pacific/Niue", "NUT11"}
	,
	{"Pacific/Norfolk", "NFT-11:30"}
	,
	{"Pacific/Noumea", "NCT-11"}
	,
	{"Pacific/Pago_Pago", "SST11"}
	,
	{"Pacific/Palau", "PWT-9"}
	,
	{"Pacific/Pitcairn", "PST8"}
	,
	{"Pacific/Pohnpei", "PONT-11"}
	,
	{"Pacific/Ponape", "PONT-11"}
	,
	{"Pacific/Port_Moresby", "PGT-10"}
	,
	{"Pacific/Rarotonga", "CKT10"}
	,
	{"Pacific/Saipan", "ChST-10"}
	,
	{"Pacific/Samoa", "SST11"}
	,
	{"Pacific/Tahiti", "TAHT10"}
	,
	{"Pacific/Tarawa", "GILT-12"}
	,
	{"Pacific/Tongatapu", "TOT-13"}
	,
	{"Pacific/Truk", "CHUT-10"}
	,
	{"Pacific/Wake", "WAKT-12"}
	,
	{"Pacific/Wallis", "WFT-12"}
	,
	{"Pacific/Yap", "CHUT-10"}
	,
	{"Poland", "CET-1CEST,M3.5.0,M10.5.0/3"}
	,
	{"Portugal", "WET0WEST,M3.5.0/1,M10.5.0"}
	,
	{"ROC", "CST-8"}
	,
	{"ROK", "KST-9"}
	,
	{"Singapore", "SGT-8"}
	,
	{"Turkey", "EET-2EEST,M3.5.0/3,M10.5.0/4"}
	,
	{"US/Alaska", "AKST9AKDT,M3.2.0,M11.1.0"}
	,
	{"US/Aleutian", "HAST10HADT,M3.2.0,M11.1.0"}
	,
	{"US/Arizona", "MST7"}
	,
	{"US/Central", "CST6CDT,M3.2.0,M11.1.0"}
	,
	{"US/East-Indiana", "EST5EDT,M3.2.0,M11.1.0"}
	,
	{"US/Eastern", "EST5EDT,M3.2.0,M11.1.0"}
	,
	{"US/Hawaii", "HST10"}
	,
	{"US/Indiana-Starke", "CST6CDT,M3.2.0,M11.1.0"}
	,
	{"US/Michigan", "EST5EDT,M3.2.0,M11.1.0"}
	,
	{"US/Mountain", "MST7MDT,M3.2.0,M11.1.0"}
	,
	{"US/Pacific", "PST8PDT,M3.2.0,M11.1.0"}
	,
	{"US/Pacific-New", "PST8PDT,M3.2.0,M11.1.0"}
	,
	{"US/Samoa", "SST11"}
	,
	{"Zulu", "UTC0"}
	,
	{NULL, NULL}
};

void update_timezone(void)
{
	char *tz;
	tz = nvram_safe_get("time_zone");	//e.g. EUROPE/BERLIN

	int i;
	int found = 0;
	const char *zone = "Europe/Berlin";
	for (i = 0; allTimezones[i].tz_name != NULL; i++) {
		if (!strcmp(allTimezones[i].tz_name, tz)) {
			zone = allTimezones[i].tz_string;
			found = 1;
			break;
		}
	}
	if (!found)
		nvram_set("time_zone", zone);
	FILE *fp = fopen("/tmp/TZ", "wb");
	fprintf(fp, "%s\n", zone);
	fclose(fp);
	setenv("TZ", zone, 1);
}
