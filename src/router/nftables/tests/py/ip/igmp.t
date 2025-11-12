# *inet;test-inet
:input;type filter hook input priority 0

*ip;test-ip4;input

igmp type membership-query;ok
igmp type membership-report-v1;ok
igmp type membership-report-v2;ok
igmp type membership-report-v3;ok
igmp type leave-group;ok

igmp type { membership-report-v1, membership-report-v2, membership-report-v3};ok
igmp type != { membership-report-v1, membership-report-v2, membership-report-v3};ok

igmp checksum 12343;ok
igmp checksum != 12343;ok
igmp checksum 11-343;ok
igmp checksum != 11-343;ok
igmp checksum { 1111, 222, 343};ok
igmp checksum != { 1111, 222, 343};ok

igmp mrt 10;ok
igmp mrt != 10;ok
