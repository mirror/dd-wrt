:osfchain;type filter hook input priority 0

*ip;osfip;osfchain
*ip6;osfip6;osfchain
*inet;osfinet;osfchain

osf name "Linux";ok
osf ttl loose name "Linux";ok
osf ttl skip name "Linux";ok
osf ttl skip version "Linux:3.0";ok
osf ttl skip version "morethan:sixteenbytes";fail
osf ttl nottl name "Linux";fail
osf name "morethansixteenbytes";fail
osf name ;fail
osf name { "Windows", "MacOs" };ok
osf version { "Windows:XP", "MacOs:Sierra" };ok
ct mark set osf name map { "Windows" : 0x00000001, "MacOs" : 0x00000002 };ok
ct mark set osf version map { "Windows:XP" : 0x00000003, "MacOs:Sierra" : 0x00000004 };ok
