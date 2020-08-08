:INPUT,FORWARD,OUTPUT
-m recent --set;=;OK
-m recent --rcheck --hitcount 8 --name foo --mask 255.255.255.255 --rsource;=;OK
-m recent --rcheck --hitcount 12 --name foo --mask 255.255.255.255 --rsource;=;OK
-m recent --update --rttl;=;OK
-m recent --set --rttl;;FAIL
-m recent --rcheck --hitcount 999 --name foo --mask 255.255.255.255 --rsource;;FAIL
# nonsensical, but all should load successfully:
-m recent --rcheck --hitcount 3 --name foo --mask 255.255.255.255 --rsource -m recent --rcheck --hitcount 4 --name foo --mask 255.255.255.255 --rsource;=;OK
-m recent --rcheck --hitcount 4 --name foo --mask 255.255.255.255 --rsource -m recent --rcheck --hitcount 4 --name foo --mask 255.255.255.255 --rsource;=;OK
-m recent --rcheck --hitcount 8 --name foo --mask 255.255.255.255 --rsource -m recent --rcheck --hitcount 12 --name foo --mask 255.255.255.255 --rsource;=;OK
