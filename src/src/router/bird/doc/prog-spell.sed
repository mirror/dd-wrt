s%<type>[^<]*</type>%%g
s%<param>[^<]*</param>%%g
s%<func>[^<]*</func>%%g
s%<funcdef>[^<]*</funcdef>%%g
s%<type/[^</]*/%%g
s%<param/[^</]*/%%g
s%<func/[^</]*/%%g
s%<funcdef/[^</]*/%%g
s%<struct/[^</]*/%%g
s%<const/[^</]*/%%g
s%<[^<>/ ]\+/%%g
s%</\?[^<>/ ]*>%%g
s%&[^;]*;%%g
