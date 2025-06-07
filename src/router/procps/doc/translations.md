Translations
============

There is a three-step process for translating man pages. Most
of the work happens in the po-man directory.

> make -C po-man translate-templates
Creates the translation templates (the .pot files) for translators
to use as a base. These, along with the tar file, should be sent
to the tp-coorindator before release.

> make get-trans
rsyncs the latest translated (.po) files for both the programs and
man pages.

> make -C po-man translate-mans
This is also called in the dist-hook and is where the translation
magic happens. Take the original man page, the relevant .po file
and produce a translated man page in that language.
All of the man pages generated are found in
po-man/(LANG)/man(SECTION)/
