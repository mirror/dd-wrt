# hddtemp - Hard drive temperature reporting
# Pattern quality: great veryfast
# 
# Usually runs on port 7634
#
# You're a silly person if you use this pattern.
#
# This pattern has been tested and is believed to work well. If you have
# trouble with this pattern, or you believe it could be improved please
# post to l7-filter-developers@lists.sf.net You may subscribe to this
# list at http://lists.sourceforge.net/lists/listinfo/l7-filter-developers

hddtemp
^\|/dev/[a-z][a-z][a-z]\|[0-9a-z]*\|[0-9][0-9]\|[cfk]\|
