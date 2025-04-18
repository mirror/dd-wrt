/*
 *
 * This file is generated automatically and part of nDPI
 *
 * nDPI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * nDPI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with nDPI.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/* ****************************************************** */


static ndpi_protocol_match teams_host_match[] = {
 { "adl.windows.com", "Teams", NDPI_PROTOCOL_MSTEAMS, NDPI_PROTOCOL_CATEGORY_COLLABORATIVE, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_DEFAULT_LEVEL },
 { "aka.ms", "Teams", NDPI_PROTOCOL_MSTEAMS, NDPI_PROTOCOL_CATEGORY_COLLABORATIVE, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_DEFAULT_LEVEL },
 { "compass-ssl.microsoft.com", "Teams", NDPI_PROTOCOL_MSTEAMS, NDPI_PROTOCOL_CATEGORY_COLLABORATIVE, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_DEFAULT_LEVEL },
 { "join.secure.skypeassets.com", "Teams", NDPI_PROTOCOL_MSTEAMS, NDPI_PROTOCOL_CATEGORY_COLLABORATIVE, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_DEFAULT_LEVEL },
 { "keydelivery.mediaservices.windows.net", "Teams", NDPI_PROTOCOL_MSTEAMS, NDPI_PROTOCOL_CATEGORY_COLLABORATIVE, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_DEFAULT_LEVEL },
 { "lync.com", "Teams", NDPI_PROTOCOL_MSTEAMS, NDPI_PROTOCOL_CATEGORY_COLLABORATIVE, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_DEFAULT_LEVEL },
 { "mlccdnprod.azureedge.net", "Teams", NDPI_PROTOCOL_MSTEAMS, NDPI_PROTOCOL_CATEGORY_COLLABORATIVE, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_DEFAULT_LEVEL },
 { "skype.com", "Teams", NDPI_PROTOCOL_MSTEAMS, NDPI_PROTOCOL_CATEGORY_COLLABORATIVE, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_DEFAULT_LEVEL },
 { "streaming.mediaservices.windows.net", "Teams", NDPI_PROTOCOL_MSTEAMS, NDPI_PROTOCOL_CATEGORY_COLLABORATIVE, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_DEFAULT_LEVEL },
 { "teams.cloud.microsoft", "Teams", NDPI_PROTOCOL_MSTEAMS, NDPI_PROTOCOL_CATEGORY_COLLABORATIVE, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_DEFAULT_LEVEL },
 { "teams.microsoft.com", "Teams", NDPI_PROTOCOL_MSTEAMS, NDPI_PROTOCOL_CATEGORY_COLLABORATIVE, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_DEFAULT_LEVEL },
 { "users.storage.live.com", "Teams", NDPI_PROTOCOL_MSTEAMS, NDPI_PROTOCOL_CATEGORY_COLLABORATIVE, NDPI_PROTOCOL_ACCEPTABLE, NDPI_PROTOCOL_DEFAULT_LEVEL },
 { NULL, NULL, NDPI_PROTOCOL_UNKNOWN, NDPI_PROTOCOL_CATEGORY_UNSPECIFIED, NDPI_PROTOCOL_SAFE, NDPI_PROTOCOL_DEFAULT_LEVEL }
};
