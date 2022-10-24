Contributing to libConfuse
==========================

We welcome any and all help in the form of bug reports, fixes, patches
for new features -- *preferably as [GitHub][github] pull requests* --
submitting a pull request practically guarantees inclusion ...


Test Your Feature
-----------------

For all new features we want to encourage developers to not only submit
the feature (and documentation), but also at least a simple test that
can be run from `make check`.

The tests serve both as examples and help prevent regressions.


Coding Style
------------

First of all, lines are allowed to be longer than 72 characters these
days.  In fact, there exist no enforced maximum, but keeping it around
100/132 characters is OK.

The coding style in libConfuse is Linux [KNF][], detailed [here][style].
To aid developers contributing to the project, source files contain a
trailer of `Local Variables:` used to instruct Emacs to use the correct
indentation mode.  There is also a silly little script `indent.sh`,
which is only supposed to serve as a help.

> Always submit code that follows the style of surrounding code!


Commit Messages
---------------

Commit messages exist to track *why* a change was made.  Try to be as
clear and concise as possible in your commit messages.  Example from
the [Pro Git][gitbook] online book:

    Brief, but clear and concise summary of changes
    
    More detailed explanatory text, if necessary.  Wrap it to about 72
    characters or so.  In some contexts, the first line is treated as
    the subject of an email and the rest of the text as the body.  The
    blank line separating the ummary from the body is critical (unless
    you omit the body entirely); tools like rebase can get confused if
    you run the two together.
    
    Further paragraphs come after blank lines.
    
     - Bullet points are okay, too
    
     - Typically a hyphen or asterisk is used for the bullet, preceded
       by a single space, with blank lines in between, but conventions
       vary here
    
    Signed-off-by: First Lastname <some.email@example.com>


[github]:   https://github.com/libconfuse/libconfuse/
[KNF]:      https://en.wikipedia.org/wiki/Kernel_Normal_Form
[style]:    https://www.kernel.org/doc/html/latest/process/coding-style.html
[gitbook]:  https://git-scm.com/book/ch5-2.html
