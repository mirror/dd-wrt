#!/usr/bin/perl -w
use strict;
use warnings;
use Fatal qw/:void open opendir/;

foreach my $category (@ARGV) {
    $category =~ s/\.c$//;

    my $exports = get_exports($category);
    build_category($category, $exports);
}

sub build_category {
    my ($category, $exports) = @_;
    my %consolidated_requires;

    open(FILE, '>', $category . '.c');

    # Get the list of all requires needed for this shared object
    foreach my $file (sort keys %{$exports}) {
        foreach my $required (sort keys %{$exports->{$file}{requires}}) {

            # skip dupes
            next if(defined($consolidated_requires{$required}));
            $consolidated_requires{$required}++;

            print FILE "//REQUIRES:$required\n";
        }
    }

    # Standard includes
    foreach my $file ('sf_snort_plugin_api.h') {
        print FILE "#include \"$file\"\n";
    }

    # Dump function declarations
    foreach my $file (sort keys %{$exports}) {
        foreach my $rule (sort keys %{ $exports->{$file}{funcs} }) {
            print FILE "extern Rule $rule;\n";
        }
    }

    # Dump rule structure pointers
    print FILE "Rule *rules[] = {\n";

    foreach my $file (sort keys %{$exports}) {
        foreach my $rule (sort keys %{ $exports->{$file}{funcs} }) {
            print FILE "    &$rule,\n";
        }
    }
    print FILE "    NULL\n};\n";
    close FILE;
}

sub get_exports {
    my ($category) = @_;
    my %exports;

    opendir(DIR, '.');
    foreach my $file (grep { /^$category\_.*\.c$/ } readdir(DIR)) {
        my $code = strip_comments(slurp($file));

        # Extract required utility modules
        # Find lines that include so-util headers like #include "so-util_base64-decode.h"
        while ($code =~ s/^\s*#include\s+"so-util_([-a-zA-Z0-9]+)\.h//sm) {
            my $required = $1;
            $exports{$file}{requires}{$required}++;
        }

        # Extract function names
        while ($code =~ s/^\s*Rule\s+(\w+)\s*=\s*\{//sm) {
            my $func = $1;
            $exports{$file}{funcs}{$func}++;
        }
    }
    closedir DIR;
    return \%exports;
}

sub slurp {
    my ($file) = @_;
    open(FILE, '<', $file);
    local $/;
    return <FILE>;
}

# XXX - should be more robust...
sub strip_comments {
    my ($code) = @_;
    $code =~ s{
       /\* # Match the opening delimiter.
       .*? # Match a minimal number of characters.
       \*/ # Match the closing delimiter.
   }{}gsx;
    return $code;
}
