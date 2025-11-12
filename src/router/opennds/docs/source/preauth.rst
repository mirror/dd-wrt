PreAuth Option
==============

Overview
********

**PreAuth** as a separate option was deprecated with the introduction of openNDS version 9 and removed with the introduction of version 10. It is replaced entirely with the ThemeSpec script support.

.. note::
 ThemeSpec is a wrapper around the underlying PreAuth technology. It allows greatly simplified configuration and removes the requirements for complicated authentication code to be written for a custom splash page sequence, replacing it with the single library function call, *auth_log()*.
