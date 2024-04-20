# yanglint testing

Testing yanglint is divided into two ways.
It is either tested in interactive mode using the tcl command 'expect' or non-interactively, classically from the command line.
For both modes, unit testing was used using the tcl package tcltest.

## How to

The sample commands in this chapter using `tclsh` are called in the `interactive` or `non-interactive` directories.

### How to run all yanglint tests?

In the build directory designated for cmake, enter:

```
ctest -R yanglint
```

### How to run all yanglint tests that are in interactive mode?

In the interactive directory, run:

```
tclsh all.tcl
```

### How to run all yanglint tests that are in non-interactive mode?

In the non-interactive directory, run:

```
tclsh all.tcl
```

### How to run all unit-tests from .test file?

```
tclsh clear.test
```

or alternatively:

```
tclsh all.tcl -file clear.test
```

### How to run one unit-test?

```
tclsh clear.test -match clear_ietf_yang_library
```

or alternatively:

```
tclsh all.tcl -file clear.test -match clear_ietf_yang_library
```

### How to run unit-tests for a certain yanglint command?

Test names are assumed to consist of the command name:

```
tclsh all.tcl -match clear*
```

### How do I get more detailed information about 'expect' for a certain test?

In the interactive directory, run:

```
tclsh clear.test -match clear_ietf_yang_library -load "exp_internal 1"
```

### How do I get more detailed dialog between 'expect' and yanglint for a certain test?

In the interactive directory, run:

```
tclsh clear.test -match clear_ietf_yang_library -load "log_user 1"
```

### How do I suppress error message from tcltest?

Probably only possible to do via `-verbose ""`

### How can I also debug?

You can write commands `interact` and `interpreter` from 'Expect' package into some test.
However, the most useful are the `exp_internal` and `log_user`, which can also be written directly into the test.
See also the rlwrap tool.
You can also use other debugging methods used in tcl programming.

### Are the tests between interactive mode and non-interactive mode similar?

Sort of...
- regex \n must be changed to \r\n in the tests for interactive yanglint

### I would like to add a new "ly_" function.

Add it to the ly.tcl file.
If you need to call other subfunctions in it, add them to namespace ly::private.

### I would like to use function other than those prefixed with "ly_".

Look in the common.tcl file in the "uti" namespace,
which contains general tcl functions that can be used in both interactive and non-interactive tests.
