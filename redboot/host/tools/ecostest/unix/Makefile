common_sources = \
	../../Utils/common/Properties.cpp       \
	../../Utils/common/Subprocess.cpp       \
	../../Utils/common/Collections.cpp      \
	../../Utils/common/eCosStd.cpp          \
	../../Utils/common/eCosThreadUtils.cpp  \
	../../Utils/common/eCosTrace.cpp        \
	../../Utils/common/eCosSerial.cpp	\
	../../Utils/common/eCosSocket.cpp	\
	../common/eCosTestUtils.cpp		\
	../common/TestResource.cpp		\
	../common/ResetAttributes.cpp		\
	../common/eCosTest.cpp			\
	../common/eCosTestPlatform.cpp			\
	../common/eCosTestDownloadFilter.cpp	\
	../common/eCosTestMonitorFilter.cpp	\
	../common/eCosTestSerialFilter.cpp

common_objects = $(notdir $(common_sources:.cpp=.o))

all_sources = $(common_sources) ../common/ser_filter.cpp

programs = ser_filter

