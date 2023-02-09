gnome = import('gnome')

if get_option('docs')
  configure_file(
    input : 'version.xml.in',
    output : 'version.xml',
    configuration : cdata)

  gnome.gtkdoc('libpsl',
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
endif
