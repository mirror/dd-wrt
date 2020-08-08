:INPUT,FORWARD,OUTPUT
-m tcpmss --mss 42;;FAIL
-p tcp -m tcpmss --mss 42;=;OK
-p tcp -m tcpmss --mss 42:12345;=;OK
-p tcp -m tcpmss --mss 42:65536;;FAIL
-p tcp -m tcpmss --mss 65535:1000;;FAIL
