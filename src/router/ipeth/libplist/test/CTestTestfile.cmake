# CMake generated Testfile for 
# Source directory: /home/seg/DEV/pb42/src/router/ipeth/libplist/test
# Build directory: /home/seg/DEV/pb42/src/router/ipeth/libplist/test
# 
# This file includes the relevent testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
ADD_TEST(Empty "plist_test" "/home/seg/DEV/pb42/src/router/ipeth/libplist/test/data/1.plist")
ADD_TEST(Small "plist_test" "/home/seg/DEV/pb42/src/router/ipeth/libplist/test/data/2.plist")
ADD_TEST(Medium "plist_test" "/home/seg/DEV/pb42/src/router/ipeth/libplist/test/data/3.plist")
ADD_TEST(Large "plist_test" "/home/seg/DEV/pb42/src/router/ipeth/libplist/test/data/4.plist")
ADD_TEST(Huge "plist_test" "/home/seg/DEV/pb42/src/router/ipeth/libplist/test/data/5.plist")
ADD_TEST(Big_Array "plist_test" "/home/seg/DEV/pb42/src/router/ipeth/libplist/test/data/6.plist")
ADD_TEST(EmptyCmp "plist_cmp" "/home/seg/DEV/pb42/src/router/ipeth/libplist/test/data/1.plist" "/home/seg/DEV/pb42/src/router/ipeth/libplist/test/data/1.plist.out")
ADD_TEST(SmallCmp "plist_cmp" "/home/seg/DEV/pb42/src/router/ipeth/libplist/test/data/2.plist" "/home/seg/DEV/pb42/src/router/ipeth/libplist/test/data/2.plist.out")
ADD_TEST(MediumCmp "plist_cmp" "/home/seg/DEV/pb42/src/router/ipeth/libplist/test/data/3.plist" "/home/seg/DEV/pb42/src/router/ipeth/libplist/test/data/3.plist.out")
ADD_TEST(LargeCmp "plist_cmp" "/home/seg/DEV/pb42/src/router/ipeth/libplist/test/data/4.plist" "/home/seg/DEV/pb42/src/router/ipeth/libplist/test/data/4.plist.out")
ADD_TEST(HugeCmp "plist_cmp" "/home/seg/DEV/pb42/src/router/ipeth/libplist/test/data/5.plist" "/home/seg/DEV/pb42/src/router/ipeth/libplist/test/data/5.plist.out")
ADD_TEST(Big_ArrayCmp "plist_cmp" "/home/seg/DEV/pb42/src/router/ipeth/libplist/test/data/6.plist" "/home/seg/DEV/pb42/src/router/ipeth/libplist/test/data/6.plist.out")
