-- Copyright (C) 2000-2002 Carnegie Mellon University
--
--  Author(s): Chris Reid <Chris.Reid@CodeCraftConsultants.com>
--  
--  Based on the create_mysql file from:
--             Jed Pickel <jpickel@cert.org>, <jed@pickel.net>
--             Roman Danyliw <rdd@cert.org>, <roman@danyliw.com>
--             Todd Schrubb <tls@cert.org>
--
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License Version 2 as
-- published by the Free Software Foundation.  You may not use, modify or
-- distribute this program under any other version of the GNU General
-- Public License.
-- 
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
-- 
-- You should have received a copy of the GNU General Public License
-- along with this program; if not, write to the Free Software
-- Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.



-- Note that Roman Danyliw graciously provides an Entity Relationship diagram
-- for the Snort / ACID database schema.  This ERD is available from the ACID
-- website:
--
--    http://www.cert.org/kb/acid/
--
-- In the comments below, all fields marked as "FK" (foreign key) should
-- be interpreted as "implied" foreign key, not "enforced" foreign key.
-- These comments are intended to be used to help a database administrator
-- construct their own diagram showing relationships between tables.



CREATE TABLE [schema] ( vseq        NUMERIC(10,0) NOT NULL,
                        ctime       DATETIME      NOT NULL,
                        PRIMARY KEY (vseq))
INSERT INTO [schema]  (vseq, ctime) VALUES ('107', GETDATE())

CREATE TABLE event  ( sid         NUMERIC(10,0)   NOT NULL ,  -- FK to sensor.sid
                      cid         NUMERIC(10,0)   NOT NULL ,
                      signature   NUMERIC(10,0)   NOT NULL ,  -- FK to signature.sig_id
                      timestamp   DATETIME        NOT NULL ,
                      PRIMARY KEY (sid,cid))
CREATE INDEX IX_event_signature  ON event(signature)
CREATE INDEX IX_event_timestamp  ON event(timestamp)

CREATE TABLE signature ( sig_id       NUMERIC(10,0) IDENTITY(1,1) NOT NULL ,
                         sig_name     VARCHAR(255)                NOT NULL,
                         sig_class_id NUMERIC(10,0),          -- FK to sig_class.sig_class_id
                         sig_priority NUMERIC(10,0),
                         sig_rev      NUMERIC(10,0),
                         sig_sid      NUMERIC(10,0),
                         sig_gid      NUMERIC(10,0),
                         PRIMARY KEY (sig_id))
CREATE INDEX IX_signature_signame    ON signature(sig_name)
CREATE INDEX IX_signature_sigclassid ON signature(sig_class_id)

CREATE TABLE sig_reference ( sig_id  NUMERIC(10,0) NOT NULL,  -- FK to signature.sig_id
                             ref_seq NUMERIC(10,0) NOT NULL,
                             ref_id  NUMERIC(10,0) NOT NULL,  -- FK to reference.ref_id
                             PRIMARY KEY(sig_id, ref_seq))

CREATE TABLE reference (  ref_id        NUMERIC(10,0) IDENTITY(1,1) NOT NULL,
                          ref_system_id NUMERIC(10,0)               NOT NULL,  -- FK to reference_system.ref_system_id
                          ref_tag       VARCHAR(8000)               NOT NULL,
                          PRIMARY KEY (ref_id))

CREATE TABLE reference_system ( ref_system_id   NUMERIC(10,0) IDENTITY(1,1) NOT NULL,
                                ref_system_name VARCHAR(20),
                                PRIMARY KEY (ref_system_id))

CREATE TABLE sig_class ( sig_class_id        NUMERIC(10,0) IDENTITY(1,1) NOT NULL,
                         sig_class_name      VARCHAR(60)                 NOT NULL,
                         PRIMARY KEY (sig_class_id))
CREATE INDEX IX_sigclass_sigclassid    ON sig_class(sig_class_id)
CREATE INDEX IX_sigclass_sigclassname  ON sig_class(sig_class_name)


-- store info about the sensor supplying data
CREATE TABLE sensor ( sid         NUMERIC(10,0) IDENTITY(1,1) NOT NULL ,
                      hostname    VARCHAR(100) ,
                      interface   VARCHAR(100) ,
                      filter      VARCHAR(100) ,
                      detail      INT ,                         -- FK to detail.detail_type
                      encoding    INT ,                         -- FK to encoding.encoding_type
                      last_cid    NUMERIC(10,0) NOT NULL,
                      PRIMARY KEY (sid))

-- All of the fields of an ip header
CREATE TABLE iphdr  ( sid         NUMERIC(10,0)      NOT NULL ,  -- FK to event.sid, event.cid
                      cid         NUMERIC(10,0)      NOT NULL ,
                      ip_src      NUMERIC(10,0)      NOT NULL ,
                      ip_dst      NUMERIC(10,0)      NOT NULL ,
                      ip_ver      TINYINT ,
                      ip_hlen     TINYINT ,
                      ip_tos      TINYINT ,
                      ip_len      INT ,
                      ip_id       INT ,
                      ip_flags    TINYINT ,
                      ip_off      INT ,
                      ip_ttl      TINYINT ,
                      ip_proto    TINYINT            NOT NULL ,
                      ip_csum     INT ,
                      PRIMARY KEY (sid,cid) )
CREATE INDEX IX_iphdr_ipsrc ON iphdr(ip_src)
CREATE INDEX IX_iphdr_ipdst ON iphdr(ip_dst)

-- All of the fields of a tcp header
CREATE TABLE tcphdr(  sid         NUMERIC(10,0)      NOT NULL ,  -- FK to event.sid, event.cid
                      cid         NUMERIC(10,0)      NOT NULL ,
                      tcp_sport   INT                NOT NULL ,
                      tcp_dport   INT                NOT NULL ,
                      tcp_seq     NUMERIC(10,0) ,
                      tcp_ack     NUMERIC(10,0) ,
                      tcp_off     TINYINT ,
                      tcp_res     TINYINT ,
                      tcp_flags   TINYINT            NOT NULL ,  -- FK to protocols (see snortdb-extra)
                      tcp_win     INT ,
                      tcp_csum    INT ,
                      tcp_urp     INT ,
                      PRIMARY KEY (sid,cid))
CREATE INDEX IX_tcphdr_sport    ON tcphdr(tcp_sport)
CREATE INDEX IX_tcphdr_dport    ON tcphdr(tcp_dport)
CREATE INDEX IX_tcphdr_tcpflags ON tcphdr(tcp_flags)

-- All of the fields of a udp header
CREATE TABLE udphdr(  sid         NUMERIC(10,0)      NOT NULL ,  -- FK to event.sid, event.cid
                      cid         NUMERIC(10,0)      NOT NULL ,
                      udp_sport   INT                NOT NULL ,
                      udp_dport   INT                NOT NULL ,
                      udp_len     INT ,
                      udp_csum    INT ,
                      PRIMARY KEY (sid,cid))
CREATE INDEX IX_udphdr_sport    ON udphdr(udp_sport)
CREATE INDEX IX_udphdr_dport    ON udphdr(udp_dport)

-- All of the fields of an icmp header
CREATE TABLE icmphdr( sid         NUMERIC(10,0)      NOT NULL ,  -- FK to event.sid, event.cid
                      cid         NUMERIC(10,0)      NOT NULL ,
                      icmp_type   TINYINT            NOT NULL ,
                      icmp_code   TINYINT            NOT NULL ,
                      icmp_csum   INT ,
                      icmp_id     INT ,
                      icmp_seq    INT ,
                      PRIMARY KEY (sid,cid))
CREATE INDEX IX_icmphdr_icmptype ON icmphdr(icmp_type)

-- Protocol options
CREATE TABLE opt    ( sid         NUMERIC(10,0)      NOT NULL ,  -- FK to iphdr.sid, iphdr.cid
                      cid         NUMERIC(10,0)      NOT NULL ,  -- or to tcphdr.sid, tcphdr.cid
                      optid       NUMERIC(10,0)      NOT NULL ,
                      opt_proto   TINYINT            NOT NULL ,
                      opt_code    TINYINT            NOT NULL ,
                      opt_len     INT ,
                      opt_data    VARCHAR(8000) ,
                      PRIMARY KEY (sid,cid,optid))

-- Packet payload
CREATE TABLE data   ( sid           NUMERIC(10,0)    NOT NULL ,  -- FK to event.sid, event.cid
                      cid           NUMERIC(10,0)    NOT NULL ,
                      data_payload  VARCHAR(8000) ,
                      PRIMARY KEY (sid,cid))

-- encoding is a lookup table for storing encoding types
CREATE TABLE encoding(encoding_type TINYINT          NOT NULL ,
                      encoding_text VARCHAR(50)      NOT NULL ,
                      PRIMARY KEY (encoding_type))
INSERT INTO encoding (encoding_type, encoding_text) VALUES (0, 'hex')
INSERT INTO encoding (encoding_type, encoding_text) VALUES (1, 'base64')
INSERT INTO encoding (encoding_type, encoding_text) VALUES (2, 'ascii')

-- detail is a lookup table for storing different detail levels
CREATE TABLE detail  (detail_type TINYINT        NOT NULL ,
                      detail_text VARCHAR(50)    NOT NULL ,
                      PRIMARY KEY (detail_type))
INSERT INTO detail (detail_type, detail_text) VALUES (0, 'fast')
INSERT INTO detail (detail_type, detail_text) VALUES (1, 'full')

-- be sure to also use the snortdb-extra tables if you want
-- mappings for tcp flags, protocols, and ports


grant select, insert on [schema]         to public
grant select, insert on signature        to public
grant select, insert on sig_reference    to public
grant select, insert on reference        to public
grant select, insert on reference_system to public
grant select, insert on sig_class        to public
grant select, insert on data             to public
grant select, insert on detail           to public
grant select, insert on encoding         to public
grant select, insert on event            to public
grant select, insert on icmphdr          to public
grant select, insert on iphdr            to public
grant select, insert on opt              to public
grant select, insert on sensor           to public
grant select, insert on tcphdr           to public
grant select, insert on udphdr           to public

