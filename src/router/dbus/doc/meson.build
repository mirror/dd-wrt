# Copyright © 2019-2020 Salamandar <felix@piedallu.me>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# XML files used for Man and html doc
xmls_mans_htmls_names = [
    'dbus-cleanup-sockets.1',
    'dbus-daemon.1',
    'dbus-launch.1',
    'dbus-monitor.1',
    'dbus-run-session.1',
    'dbus-send.1',
    'dbus-test-tool.1',
    'dbus-update-activation-environment.1',
    'dbus-uuidgen.1',
]
xml_files = []
man_html_list = []
foreach xml_in : xmls_mans_htmls_names
    xml_file = configure_file(
        input : xml_in + '.xml.in',
        output: xml_in + '.xml',
        configuration: data_config,
    )
    man_html_list += {
        'xml'   : xml_file,
        'man'   : xml_in,
        'html'  : xml_in + '.html',
    }
    xml_files += xml_file
endforeach

# XML files used for html doc
xmls_names = [
    'dbus-faq',
    'dbus-specification',
    'dbus-test-plan',
    'dbus-tutorial',
]
html_list = []
foreach xml_in : xmls_names
    html_list += {
        'xml'   : xml_in + '.xml',
        'html'  : xml_in + '.html',
    }
endforeach

# uploaded and distributed, but not installed
static_docs = files(
    'dbus-api-design.duck',
    'dbus-faq.xml',
    'dbus-specification.xml',
    'dbus-test-plan.xml',
    'dbus-tutorial.xml',
    'dcop-howto.txt',
    'introspect.xsl',
)

###############################################################################
# Install man files

if build_xml_docs
    foreach man: man_html_list
        custom_target(man.get('man'),
            input: man.get('xml'),
            output: man.get('man'),
            command: [
                xsltproc,
                '--nonet',
                '--xinclude',
                '--stringparam', 'man.output.quietly', '1',
                '-o', '@OUTPUT@',
                'http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl',
                '@INPUT@',
            ],
            install: true,
            install_dir: get_option('mandir') / 'man1',
        )
    endforeach
endif

###############################################################################
# Install html doc files

html_files = []
if build_xml_docs
    foreach man: man_html_list + html_list
        html_files += custom_target(man.get('html'),
            input: man.get('xml'),
            output: man.get('html'),
            command: [
                xsltproc,
                '--nonet',
                '--xinclude',
                '--stringparam', 'generate.consistent.ids', '1',
                '-o', '@OUTPUT@',
                'http://docbook.sourceforge.net/release/xsl/current/html/docbook.xsl',
                '@INPUT@',
            ],
            install: true,
            install_dir: docs_dir,
        )
    endforeach
endif

docs_files = [
    'system-activation.txt',
    'diagram.png',
    'diagram.svg',
]

install_data(
    sources: docs_files,
    install_dir: docs_dir,
)

configure_file(
    input: 'index.html.in',
    output: 'index.html',
    configuration: data_config,
    install_dir: docs_dir,
)

###############################################################################
# Install dtd files


xml_dir = get_option('datadir') / 'xml' / 'dbus-1'
dtd_files = [
    'busconfig.dtd',
    'introspect.dtd',
]

install_data(
    sources: dtd_files,
    install_dir: xml_dir,
)

configure_file(
    input: 'catalog.xml.in',
    output: 'catalog.xml',
    install_dir: xml_dir,
    configuration: {'DBUS_DTD_DIR': get_option('prefix') / xml_dir},
)

###############################################################################
# Doxygen
if doxygen.found()
    qch_dir = get_option('qch_dir')
    if qch_dir == ''
        qch_dir = docs_dir
    endif

    doxyfile = configure_file(
        input: '../Doxyfile.in',
        output: 'Doxyfile',
        configuration: data_config,
    )

    dbus_srcs = run_command(
        python,
        files('list-doc-source.py'),
        meson.project_source_root() / 'dbus',
        check: true
    ).stdout().strip().split('\n')
    dbus_srcs += dbus_arch_deps_h

    doxygen_tgt = custom_target('doxygen',
        input: doxyfile,
        output: 'api',
        depend_files: dbus_srcs,
        command: [doxygen, doxyfile],
    )
    alias_target('doxygen', doxygen_tgt)

    meson.add_install_script(
        'meson_post_install.py',
        meson.current_build_dir(),
        docs_dir,
        meson.current_build_dir() / 'api/qch/dbus-@0@.qch'.format(version),
        qch_dir,
        # ignored further arguments, but for dependency
        doxygen_tgt,
    )

    if xsltproc.found()
        custom_target('dbus.devhelp2',
            input: 'doxygen_to_devhelp.xsl',
            output: 'dbus.devhelp2',
            depends: [doxygen_tgt],
            command: [ xsltproc, '-o', '@OUTPUT@', '@INPUT@', meson.current_build_dir() / 'api/xml/index.xml' ],
            install: true,
            install_dir: docs_dir,
        )
    endif
endif

if ducktype.found() and yelpbuild.found()
    design_page = custom_target('dbus-api-design.page',
        input: 'dbus-api-design.duck',
        output: 'dbus-api-design.page',
        command: [ ducktype, '-o', '@OUTPUT@', '@INPUT@' ],
    )
    html_files += custom_target('dbus-api-design.html',
        input: design_page,
        output: [
            'dbus-api-design.html',
            'yelp.js',
            'C.css',
            'highlight.pack.js',
        ],
        command: [
            yelpbuild, 'html',
            '-o', meson.current_build_dir(),
            '@INPUT@',
        ],
        install: true,
        install_dir: docs_dir,
    )
endif

if can_upload_docs
  run_target('maintainer-upload-docs',
      command: [
          find_program('maintainer-upload-docs.sh'),
          files(docs_files),
          files(dtd_files),
          static_docs,
          html_files,
          xml_files,
          bonus_files,
      ],
      depends: doxygen_tgt,
  )
endif
