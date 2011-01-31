#!/usr/bin/perl
while (<>) {
    next if !/ATM_CV_/;
    chop;
    chop($_ = $_.<>) unless /\*\//;
    s/\s+/ /g;
    /ATM_CV_\S+\s+(\d+)\s+\/\*\s*(.*\S)\s*\*\//;
    $map[$1] = $2;
}
print "/* THIS IS A MACHINE-GENERATED FILE. DO NOT EDIT ! */\n\n";
print "#if HAVE_CONFIG_H\n";
print "#include <config.h>\n";
print "#endif\n\n";
print "const char *cause_text[] = {\n";
for ($i = 0; $i < 128; $i++) {
    print "    \"".(defined $map[$i] ? $map[$i] : "unknown cause $i")."\"".
      ($i == 127 ? "\n" : ",\n");
}
print "};\n";
