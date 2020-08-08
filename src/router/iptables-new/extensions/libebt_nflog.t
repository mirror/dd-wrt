:INPUT,FORWARD,OUTPUT
--nflog;=;OK
--nflog-group 42;=;OK
--nflog-range 42;--nflog-group 1 --nflog-range 42 -j CONTINUE;OK
--nflog-threshold 100 --nflog-prefix foo;--nflog-prefix "foo" --nflog-group 1 --nflog-threshold 100 -j CONTINUE;OK
