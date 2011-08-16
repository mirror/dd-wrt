--  Copyright (C) 2000-2002 Carnegie Mellon University
--  Portions Copyright (C) 2000 Mike Andersen <mike@src.no>
--  Portions Copyright (C) 2001 Andrew Stubbs <andrews@stusoft.com>
--  Portions Copyright (C) 2001 Jed Pickel <jed@pickel.net>
--
--  Author(s): Mike Andersen <mike@src.no>
--             Thomas Stenhaug <thomas@src.no>
--
--  Maintainer: Roman Danyliw <rdd@cert.org>, <roman@danyliw.com>
--
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation.  You may not use, modify or distribute
-- this program under any other version of the GNU General Public License.
--
--  This program is distributed in the hope that it will be useful,
--  but WITHOUT ANY WARRANTY; without even the implied warranty of
--  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--  GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program; if not, write to the Free Software 
-- Foundation, Inc., 59 Temple Place - Suite 330, Boston, 
-- MA 02111-1307, USA.
--
--
-- This file was recently updated by Andrew Stubbs to fix some bugs
-- and make the script more user friendly.
--  
-- Comments from Andrew <andrews@stusoft.com> on his update:
--  
--    There's a trigger in place of the AUTO_INCREMENT-option for the
--    sensor.sid.  I don't fully understand how the NUMBER-type conversion
--    works at this point.
--
--    Oracles DATE seems "bit" more picky on the format than MySQL.
--
--    Rename it to : create_oracle.sql
--    to run type : sqlplus user/password@db_instance @ create_oracle.sql
--
--    The drop tables / sequences are a personal preference - remove if 
--    you wish the prompt merely echos the stuff after it - useful for
--    figuring out where you are when its running

prompt schema;
drop table schema;

CREATE TABLE schema ( vseq        INT          NOT NULL,
                      ctime       VARCHAR2(24) NOT NULL,
                      PRIMARY KEY (vseq));

INSERT INTO schema  (vseq, ctime) VALUES ('107', sysdate);

prompt event;
drop table event;
CREATE TABLE event  ( sid    INT         NOT NULL,
                      cid    INT         NOT NULL,
                      signature   INT    NOT NULL,
                      timestamp   DATE   NOT NULL,
                      PRIMARY KEY (sid,cid));

prompt signature;
drop table signature;
CREATE TABLE signature ( sig_id   INT           NOT NULL,
                         sig_name VARCHAR2(255),
                         sig_class_id INT,
                         sig_priority INT,
                         sig_rev      INT,
                         sig_sid      INT,
                         sig_gid      INT,
                         PRIMARY KEY (sig_id));

--
--  auto-increment the signature.sig_id
--
drop sequence seq_snort_signature_id ;
CREATE SEQUENCE seq_snort_signature_id START WITH 1 INCREMENT BY 1;

CREATE or replace TRIGGER tr_snort_signature_id
        BEFORE INSERT ON signature
        FOR EACH ROW
        BEGIN
                SELECT seq_snort_signature_id.nextval INTO :new.SIG_ID FROM
dual;
        END;
/
prompt sig_reference;
drop table sig_reference;
CREATE TABLE sig_reference (sig_id  INT    NOT NULL,
                            ref_seq INT    NOT NULL,
                            ref_id  INT    NOT NULL,
                            PRIMARY KEY(sig_id, ref_seq));

prompt reference;
drop table reference;
CREATE TABLE reference (  ref_id        INT          NOT NULL,
                          ref_system_id INT          NOT NULL,
                          ref_tag       VARCHAR2(100) NOT NULL,
                          PRIMARY KEY (ref_id));
--
--  auto-increment the reference.ref_id
--

drop sequence seq_snort_reference_id;
CREATE SEQUENCE seq_snort_reference_id START WITH 1 INCREMENT BY 1;

CREATE or replace TRIGGER tr_snort_reference_id
        BEFORE INSERT ON reference
        FOR EACH ROW
        BEGIN
                SELECT seq_snort_reference_id.nextval INTO :new.REF_ID FROM
dual;
        END;
/

prompt reference_system;
drop table reference_system ;
CREATE TABLE reference_system ( ref_system_id   INT          NOT NULL,
                                ref_system_name VARCHAR2(20),
                                PRIMARY KEY (ref_system_id));

drop sequence seq_snort_ref_system_id ;
CREATE SEQUENCE seq_snort_ref_system_id START WITH 1 INCREMENT BY 1;
CREATE or replace TRIGGER tr_snort_ref_system_id
        BEFORE INSERT ON reference_system
        FOR EACH ROW
        BEGIN
                SELECT seq_snort_ref_system_id.nextval INTO
:new.REF_SYSTEM_ID FROM dual;
        END;
/
prompt sig_class;
drop table sig_class;
CREATE TABLE sig_class ( sig_class_id   INT   NOT NULL,
                         sig_class_name VARCHAR(60) NOT NULL,
                         PRIMARY KEY (sig_class_id));

drop sequence seq_snort_sig_class_id ;
CREATE SEQUENCE seq_snort_sig_class_id START WITH 1 INCREMENT BY 1;
CREATE or REPLACE TRIGGER tr_snort_sig_class_id
        BEFORE INSERT ON sig_class
        FOR EACH ROW
        BEGIN
           select seq_snort_sig_class_id.nextval into :new.sig_class_id from
dual;
        END;
/
--
--  store info about the sensor supplying data
--
prompt sensor;
drop table sensor;
CREATE TABLE sensor (
        sid             INT NOT NULL,
        hostname        VARCHAR2(100),
        interface       VARCHAR2(100),
        filter          VARCHAR2(100),
        detail          INT,
        encoding        INT,
        last_cid        INT NOT NULL,
        PRIMARY KEY (sid));

--
--  auto-increment the sensor.sid
--
drop sequence seq_snort_sensor_id ;
CREATE SEQUENCE seq_snort_sensor_id START WITH 1 INCREMENT BY 1;

CREATE OR REPLACE TRIGGER tr_snort_sensor_id
        BEFORE INSERT ON sensor
        FOR EACH ROW
        BEGIN
                SELECT seq_snort_sensor_id.nextval INTO :new.SID FROM dual;
        END;
/

--  All of the fields of an ip header
prompt iphdr;
drop table iphdr;
CREATE TABLE iphdr (
        sid             INT NOT NULL,
        cid             INT NOT NULL,
        ip_src          INT NOT NULL,
        ip_dst          INT NOT NULL,
        ip_ver          INT,
        ip_hlen         INT,
        ip_tos          INT,
        ip_len          INT,
        ip_id           INT,
        ip_flags        INT,
        ip_off          INT,
        ip_ttl          INT,
        ip_proto        INT NOT NULL,
        ip_csum         INT,
        PRIMARY KEY (sid,cid));


--  All of the fields of a tcp header
prompt tcphdr;
drop table tcphdr;
CREATE TABLE tcphdr (
        sid             INT NOT NULL,
        cid             INT NOT NULL,
        tcp_sport       INT NOT NULL,
        tcp_dport       INT NOT NULL,
        tcp_seq         INT,
        tcp_ack         INT,
        tcp_off         INT,
        tcp_res         INT,
        tcp_flags       INT NOT NULL,
        tcp_win         INT,
        tcp_csum        INT,
        tcp_urp         INT,
        PRIMARY KEY (sid,cid));


--  All of the fields of a udp header
prompt udphdr;
drop table udphdr;
CREATE TABLE udphdr (
        sid             INT NOT NULL,
        cid             INT NOT NULL,
        udp_sport       INT NOT NULL,
        udp_dport       INT NOT NULL,
        udp_len         INT,
        udp_csum        INT,
        PRIMARY KEY (sid,cid));


--  All of the fields of an icmp header
prompt icmphdr;
drop table icmphdr;
CREATE TABLE icmphdr(
        sid             INT NOT NULL,
        cid             INT NOT NULL,
        icmp_type       INT NOT NULL,
        icmp_code       INT NOT NULL,
        icmp_csum       INT,
        icmp_id         INT,
        icmp_seq        INT,
        PRIMARY KEY (sid,cid));


--  Protocol options
prompt opt;
drop table opt;
CREATE TABLE opt (
        sid             INT NOT NULL,
        cid             INT NOT NULL,
        optid           INT NOT NULL,
        opt_proto       INT NOT NULL,
        opt_code        INT NOT NULL,
        opt_len         INT,
        opt_data        BLOB,
        PRIMARY KEY (sid,cid,optid));


--  Packet payload
prompt data;
drop table data;
CREATE TABLE data (
        sid             INT NOT NULL,
        cid             INT NOT NULL,
        data_payload    BLOB,
        PRIMARY KEY (sid,cid));


--  encoding is a lookup table for storing encoding types
prompt encoding
drop table encoding;
CREATE TABLE encoding (
        encoding_type   INT NOT NULL,
        encoding_text   VARCHAR2(50) NOT NULL,
        PRIMARY KEY (encoding_type));

INSERT INTO encoding (encoding_type, encoding_text) VALUES (0, 'hex');
INSERT INTO encoding (encoding_type, encoding_text) VALUES (1, 'base64');
INSERT INTO encoding (encoding_type, encoding_text) VALUES (2, 'ascii');


--  detail is a lookup table for storing different detail levels
prompt detail;
drop table detail;
CREATE TABLE detail (
        detail_type     INT NOT NULL,
        detail_text     VARCHAR2(50) NOT NULL,
        PRIMARY KEY (detail_type));

INSERT INTO detail (detail_type, detail_text) VALUES (0, 'fast');
INSERT INTO detail (detail_type, detail_text) VALUES (1, 'full');
