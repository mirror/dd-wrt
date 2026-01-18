gnome = import('gnome')

if get_option('docs')
  configure_file(
    input : 'version.xml.in',
    output : 'version.xml',
    configuration : cdata)

  docs = gnome.gtkdoc('libpsl',
    main_sgml: 'libpsl-docs.sgml',
    src_dir: [
      'src',
      'include',
    ],
    dependencies: libpsl,
    scan_args: [
      '--ignore-decorators=G_GNUC_PSL_UNUSED',
    ],
    install: true)

  man = custom_target(
    output: 'libpsl.3',
    command: [
      'xsltproc', '-nonet',
      '--output', '@OUTPUT@',
      'http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl',
      meson.current_build_dir() / 'xml/libpsl.xml',
    ],
    depends: docs,
    install: true,
    install_dir: get_option('mandir') / 'man3')
endif
