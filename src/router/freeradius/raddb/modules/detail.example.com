# -*- text -*-
#
#  Detail file writer, used in the following examples:
#
#	raddb/sites-available/robust-proxy-accounting
#	raddb/sites-available/decoupled-accounting
#
#  Note that this module can write detail files that are read by
#  only ONE "listen" section.  If you use BOTH of the examples
#  above, you will need to define TWO "detail" modules.
#
#  e.g. detail1.example.com && detail2.example.com
#
#
#  We write *multiple* detail files here.  They will be processed by
#  the detail "listen" section in the order that they were created.
#  The directory containing these files should NOT be used for any
#  other purposes.  i.e. It should have NO other files in it.
#
#  Writing multiple detail enables the server to process the pieces
#  in smaller chunks.  This helps in certain catastrophic corner cases.
#
#  $Id: af7e3452fdd49ed6a3cd379c2a4d90e17f34532f $
#
detail detail.example.com {
	detailfile = ${radacctdir}/detail.example.com/detail-%Y%m%d:%H:%G
}
