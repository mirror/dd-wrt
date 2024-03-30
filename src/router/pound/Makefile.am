# Pound - the reverse-proxy load-balancer                -*- automake -*-
# Copyright (C) 2002-2010 Apsis GmbH
# Copyright (C) 2018-2024 Sergey Poznyakoff
#
# Pound is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# Pound is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with pound.  If not, see <http://www.gnu.org/licenses/>.
SUBDIRS = src tests doc

EXTRA_DIST = ChangeLog.apsis

ACLOCAL_AMFLAGS = -I am

if FROM_GIT
README: README.md
	perl md2txt.pl -o README -l 4 README.md

.PHONY: ChangeLog
ChangeLog:
	@echo "Creating ChangeLog";                                       \
	 (git log --pretty='format:%H#%ad  %cn  <%ae>%n%n%w(72,8,8)%s%n%n%b' \
		  --date=short | sed -r -f ChangeLog.sed;                 \
	  echo "";                                                        \
	  echo "Local Variables:";                                        \
	  echo "mode: change-log";                                        \
	  echo "version-control: never";                                  \
	  echo "buffer-read-only: t";                                     \
	  echo "End:") > ChangeLog
endif
