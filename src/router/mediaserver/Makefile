
#
# Intel's UPnP MicroStack/1.0, Sample MakeFile */
# Intel Device Builder - Build#1.0.2384
#

# List all object files here
OFILES = \
	CdsDidlSerializer.o \
CdsErrors.o \
CdsMediaClass.o \
CdsObject.o \
FileIoAbstraction.o \
DlnaHttp.o \
DlnaHttpClient.o \
DlnaHttpServer.o \
DLNAProtocolInfo.o \
MediaServerAbstraction.o \
DownloadController.o \
DmsIntegration.o \
FileSystemEnumerator.o \
MimeTypes.o \
UTF8Utils.o \
Main.o \
	ILibParsers.o\
	ILibAsyncSocket.o\
	ILibAsyncUDPSocket.o\
	ILibAsyncServerSocket.o\
	ILibWebClient.o\
	ILibWebServer.o\
	ILibThreadPool.o\
	MediaServer_MicroStack.o\


# Compiler command name
#CC = gcc

# Compiler flags applied to all files
CFLAGS =  -I./ -I../  -I./CdsObjects  -I./FileAbstractionFiles  -I./HttpFiles  -I./ProtocolInfoParser  -I./MediaServerAbstraction  -I./DownloadController  -I./DmsIntegration  -I./DmsIntegration/FileSystemEnumerator  -I./StringUtils $(COPTS) -Wall -D_POSIX -D_DEBUG -D_VERBOSE -DNEED_PRINTF
 
LIBS = -lpthread -lm 
# Name of the executable
VPATH =  ./ ../  ./CdsObjects  ./FileAbstractionFiles  ./HttpFiles  ./ProtocolInfoParser  ./MediaServerAbstraction  ./DownloadController  ./DmsIntegration  ./DmsIntegration/FileSystemEnumerator  ./StringUtils 
EXENAME = mserver

# Builds all object files and executable
${EXENAME} : $(OFILES)
	$(CC) $(CFLAGS) -o ${EXENAME} $(OFILES) $(LIBS)

# Macro rule for all object files.
$(OFILES) : \
	MediaServer_MicroStack.h\


# Clean up
clean :
	rm -f $(OFILES) ${EXENAME} ${EXENAME}.exe

install:
	install -d $(INSTALLDIR)/usr/sbin
	install mserver $(INSTALLDIR)/usr/sbin