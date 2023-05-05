AUTOMAKE_OPTIONS=foreign no-dependencies

noinst_LIBRARIES = libfileAPI.a

libfileAPI_a_SOURCES = \
	file_service.c \
	file_service.h \
	file_service_config.c \
	file_service_config.h \
	file_api.h \
	file_mime_process.c \
	file_mime_process.h \
	file_resume_block.c \
	file_resume_block.h \
	file_mime_config.c \
	file_mime_config.h \
	file_capture.c \
	file_capture.h \
	file_stats.c \
	file_stats.h \
    file_segment_process.c \
	file_segment_process.h \
	circular_buffer.c \
	circular_buffer.h \
	file_mempool.c \
	file_mempool.h \
	../sfutil/sf_email_attach_decode.c \
	../sfutil/sf_email_attach_decode.h \
	file_mail_common.h\ 
	file_ss.c\
	file_ss.h

INCLUDES = @INCLUDES@

SUBDIRS = libs
