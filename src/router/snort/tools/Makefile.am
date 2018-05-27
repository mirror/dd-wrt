AUTOMAKE_OPTIONS=foreign no-dependencies

if BUILD_CONTROL_SOCKET
CONTROL_DIR = control
endif

if FEAT_FILE_INSPECT
FILE_INSPECT_SERVER=file_server
endif

SUBDIRS = u2boat u2spewfoo $(CONTROL_DIR) $(FILE_INSPECT_SERVER)
if FEAT_OPEN_APPID
SUBDIRS += u2openappid u2streamer
dist_bin_SCRIPTS = appid_detector_builder.sh
endif

INCLUDES = @INCLUDES@
