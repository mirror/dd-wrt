ARG from=DOCKER_IMAGE
FROM ${from} as build

ARG DEBIAN_FRONTEND=noninteractive

#
#  Install add-apt-repository
#
RUN apt-get update && \
    apt-get install -y software-properties-common gnupg2 procps && \
    apt-get clean && \
    rm -r /var/lib/apt/lists/*

define(`CLANG_PKGS', `llvm clang lldb')dnl
ifelse(D_NAME, `debian10', `dnl
define(`CLANG_VER', `8')dnl
define(`CLANG_PKGS', `llvm-CLANG_VER clang-CLANG_VER lldb-CLANG_VER')dnl
#  For clang
RUN add-apt-repository -y "deb http://apt.llvm.org/OS_CODENAME/ llvm-toolchain-OS_CODENAME-CLANG_VER main" && \
    apt-key adv --fetch-keys http://apt.llvm.org/llvm-snapshot.gpg.key
')dnl

RUN apt-get update && \
#  Development utilities
    apt-get install -y devscripts equivs git quilt rsync fakeroot && \
#  Compilers
    apt-get install -y g++ CLANG_PKGS && \
#  eapol_test dependencies
    apt-get install -y libnl-3-dev libnl-genl-3-dev

#
#  Documentation build dependecies
#
define(`NODE_VER', ifelse(D_NAME, `ubuntu18', `16', `20'))dnl

#  - doxygen & JSON.pm
RUN apt-get install -y doxygen graphviz libjson-perl
#  - antora (npm needed)
RUN curl -sL https://deb.nodesource.com/setup_`'NODE_VER.x | bash -
RUN apt-get install -y nodejs
RUN npm i -g @antora/cli@3.1.7 @antora/site-generator-default@3.1.7
#  - pandoc
WORKDIR /tmp
RUN curl -OL $(curl -s https://api.github.com/repos/jgm/pandoc/releases/latest | grep "browser_download_url.*deb" | cut -d '"' -f 4)
RUN apt-get install -y ./pandoc-*.deb
#  - asciidoctor
RUN apt-get install -y ruby-dev
RUN gem install asciidoctor

ifelse(D_NAME, `debian10', `dnl
#
#  Set defaults
#
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-CLANG_VER 60 && \
    update-alternatives --config clang

RUN update-alternatives --install /usr/bin/lldb lldb /usr/bin/lldb-CLANG_VER 60 && \
    update-alternatives --config lldb
')

#
#  Setup a src dir in /usr/local
#
RUN mkdir -p /usr/local/src/repositories
WORKDIR /usr/local/src/repositories


#
#  Shallow clone the FreeRADIUS source
#
WORKDIR /usr/local/src/repositories
ARG source=https://github.com/FreeRADIUS/freeradius-server.git
RUN git clone --depth 1 --no-single-branch ${source}

#
#  Install build dependencies for all branches from v3 onwards
#
WORKDIR freeradius-server
RUN for i in $(git for-each-ref --format='%(refname:short)' refs/remotes/origin 2>/dev/null | sed -e 's#origin/##' | egrep "^(v[3-9]*\.[0-9x]*\.x|master)$");\
	do \
		git checkout $i; \
		if [ -e ./debian/control.in ] ; then \
			debian/rules debian/control ; \
		fi ; \
		echo 'y' | \
		mk-build-deps -irt'apt-get -yV' debian/control ; \
	done
