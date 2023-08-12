.. _securitybugs:

Security bugs
=============

Linux kernel developers take security very seriously.  As such, we'd
like to know when a security bug is found so that it can be fixed and
disclosed as quickly as possible.  Please report security bugs to the
Linux kernel security team.

Contact
-------

The Linux kernel security team can be contacted by email at
<security@kernel.org>.  This is a private list of security officers
who will help verify the bug report and develop and release a fix.
If you already have a fix, please include it with your report, as
that can speed up the process considerably.  It is possible that the
security team will bring in extra help from area maintainers to
understand and fix the security vulnerability.

As it is with any bug, the more information provided the easier it
will be to diagnose and fix.  Please review the procedure outlined in
admin-guide/reporting-bugs.rst if you are unclear about what
information is helpful.  Any exploit code is very helpful and will not
be released without consent from the reporter unless it has already been
made public.

Disclosure
----------

The goal of the Linux kernel security team is to work with the
bug submitter to bug resolution as well as disclosure.  We prefer
to fully disclose the bug as soon as possible.  It is reasonable to
delay disclosure when the bug or the fix is not yet fully understood,
the solution is not well-tested or for vendor coordination.  However, we
expect these delays to be short, measurable in days, not weeks or months.
A disclosure date is negotiated by the security team working with the
bug submitter as well as vendors.  However, the kernel security team
holds the final say when setting a disclosure date.  The timeframe for
disclosure is from immediate (esp. if it's already publicly known)
to a few weeks.  As a basic default policy, we expect report date to
disclosure date to be on the order of 7 days.

Coordination with other groups
------------------------------

The kernel security team strongly recommends that reporters of potential
security issues NEVER contact the "linux-distros" mailing list until
AFTER discussing it with the kernel security team.  Do not Cc: both
lists at once.  You may contact the linux-distros mailing list after a
fix has been agreed on and you fully understand the requirements that
doing so will impose on you and the kernel community.

The different lists have different goals and the linux-distros rules do
not contribute to actually fixing any potential security problems.

CVE assignment
--------------

The security team does not normally assign CVEs, nor do we require them
for reports or fixes, as this can needlessly complicate the process and
may delay the bug handling. If a reporter wishes to have a CVE identifier
assigned ahead of public disclosure, they will need to contact the private
linux-distros list, described above. When such a CVE identifier is known
before a patch is provided, it is desirable to mention it in the commit
message, though.

Non-disclosure agreements
-------------------------

The Linux kernel security team is not a formal body and therefore unable
to enter any non-disclosure agreements.
