# Known Issues

## Printer crash

There is a bug in Clang 3.4.2 that causes wrong data tree traversal, which results in a crash.
It can occur during any traversal but it was observed in XML printer while printing default
values. The solution is to compile using either GCC or newer version of Clang. Starting
from which version it works we do not exactly know. Clang 5.0.0 works fine.

## XPath Expressions

### Axes

libyang uses its own XPath implementation to evaluate XPath expressions. The implementation
completely lacks support for [axes](https://www.w3.org/TR/1999/REC-xpath-19991116/#axes).
Nevertheless, it should always be possible to write equivalent expressions without
the use of axes.

### Deviation Must

If there are any XPath expressions defined as part of a deviation, they include literals,
which are directly compared with identityref nodes (testing whether an identityref has a
specific identity value), and the identity literal used is from the deviation module
(meaning it could be written without a prefix), the prefix is mandatory for libyang
to evaluate the XPath expression correctly.
