Open Travel Request Parser (TREP)
=================================

[![Build Status](https://travis-ci.com/trep/opentrep.svg?branch=master)](https://travis-ci.com/trep/opentrep)
[![Docker Repository on Quay](https://quay.io/repository/trep/opentrep/status "Docker Repository on Quay")](https://quay.io/repository/trep/opentrep)

# Table of content
- [Overview](#overview)
- [Docker images](#docker-images)
  * [OpenTREP Docker images](#opentrep-docker-images)
  * [General purpose C++/Python Docker images](#general-purpose-c---python-docker-images)
    + [CentOS 7](#centos-7)
    + [Ubuntu 18.04 LTS Bionic Beaver](#ubuntu-1804-lts-bionic-beaver)
    + [Debian 9 Stretch](#debian-9-stretch)
    + [Common to all the above-mentioned Linux distributions](#common-to-all-the-above-mentioned-linux-distributions)
- [Native installation (without Docker)](#native-installation--without-docker-)
  * [RPM-based distributions (eg, Fedora/CentOS/RedHat)](#rpm-based-distributions--eg--fedora-centos-redhat-)
  * [Installation from the sources](#installation-from-the-sources)
    + [Clone the Git repository](#clone-the-git-repository)
    + [Alternatively, download and extract the tar-ball](#alternatively--download-and-extract-the-tar-ball)
    + [Installation of the dependencies](#installation-of-the-dependencies)
      - [On Linux](#on-linux)
        * [Fedora](#fedora)
        * [CentOS](#centos)
        * [Debian/Ubuntu](#debian-ubuntu)
      - [On MacOS](#on-macos)
    + [ICU](#icu)
    + [Boost](#boost)
      - [MacOS](#macos)
      - [CentOS](#centos-1)
    + [SOCI](#soci)
      - [General Unix/Linux](#general-unix-linux)
      - [Debian](#debian)
      - [MacOS](#macos-1)
    + [Building the library and test binary](#building-the-library-and-test-binary)
  * [Underlying (relational) database, SQLite or MySQL/MariaDB, if any](#underlying--relational--database--sqlite-or-mysql-mariadb--if-any)
- [Indexing the POR data](#indexing-the-por-data)
  * [Filling the (relational) database, SQLite or MySQL/MariaDB, if any](#filling-the--relational--database--sqlite-or-mysql-mariadb--if-any)
  * [Xapian indexing with standard installation](#xapian-indexing-with-standard-installation)
  * [Xapian indexing for an ad hoc deployed Web application](#xapian-indexing-for-an-ad-hoc-deployed-web-application)
- [Searching](#searching)
- [Deployment stages](#deployment-stages)
- [Index, or not, non-IATA POR](#index--or-not--non-iata-por)
- [Installing a Python virtual environment](#installing-a-python-virtual-environment)
- [Checking that the Python module works](#checking-that-the-python-module-works)
- [(Optional) Running the Django-based application server](#-optional--running-the-django-based-application-server)

<small><i><a href='http://ecotrust-canada.github.io/markdown-toc/'>Table of contents generated with markdown-toc</a></i></small>

# Overview
[OpenTREP](https://github.com/trep/opentrep)
aims at providing a clean API, and the corresponding C++
implementation, for parsing travel-/transport-focused requests.
It powers the https://transport-search.org Web site (as well
as its newer version, https://www2.transport-search.org).

OpenTREP uses Xapian (https://www.xapian.org) for the Information Retrieval
part, on freely available transport-/travel-related data (_e.g._, country
names and codes, city names and codes, airline names and codes, _etc_),
mainly to be found in the
[OpenTravelData (OPTD)
project](https://github.com/opentraveldata/opentraveldata):
http://github.com/opentraveldata/opentraveldata/tree/master/opentraveldata

OpenTREP exposes a simple, clean and object-oriented, API. For instance,
the
[`OPENTREP::interpretTravelRequest()`
method](https://github.com/trep/opentrep/blob/master/opentrep/OPENTREP_Service.hpp#L58):
* takes, as input, a character string containing the travel request;
* and yields, as output, the list of the recognized terms,
  as well as their corresponding types.

As an example, the travel request
`Washington DC Beijing Monday a/r +AA -UA 1 week 2 adults 1 dog` would yield
the following list:
 * Origin airport: Washington, DC, United States (US)
 * Destination airport: Beijing, China (CN)
 * Date of travel: next Monday
 * Date of return: 1 week after next Monday
 * Preferred airline: American Airlines (AA);
   non-preferred airline: United Airlines
 * Number of travelers: 2 adults and a dog

The output may then be used by other systems, for instance to book the
corresponding travel, or to visualize it on a map and calendar and to
share it with others.

Note that the current version of OpenTREP recognizes only geographical POR
(points of reference), whatever their number in the request. For instance,
the request `lviv rivne jitomir kbp kharkiv dnk ods lwo` yields the
following list of POR: `LWO`, `RWN`, `ZTR`, `KBP`, `HRK`, `DNK`,
`ODS` and `LWO` again.
See
[that request in action on the transport-search.org site](https://transport-search.org/search/?q=lwo+rwn+ztr+kbp+hrk+dnk+ods+lwo)
or [through the API](https://www2.transport-search.org/api/v1/geo?query=lwo+rwn+ztr+kbp+hrk+dnk+ods+lwo)
(enable [JSONView](https://github.com/gildas-lormeau/JSONView-for-Chrome)
or similar for a more confortable reading).

OpenTREP also deals with transport-related requests. For instance,
`cnshg deham nlrtm uslbg brssz cnshg` correspond to a world tour
of famous ports:
* Shanghai, China (CN)
* Hamburg, Germany (DE)
* Rotterdam, Netherlands (NL)
* Los Angeles/Long Beach, California (CA), United States (US)
* Santos, Brazil (BR)
* (Back to) Shanghai, China (CN)
See
[that request in action on the transport-search.org site](https://transport-search.org/search/?q=sha+ham+rtm+lgb+ssz+sha)
or [through the API](https://www2.transport-search.org/api/v1/geo?query=sha+ham+rtm+lgb+ssz+sha).

The underlying data for the POR is the
[OpenTravelData `optd_por_public_all.csv` file](http://github.com/opentraveldata/opentraveldata/tree/master/opentraveldata/optd_por_public_all.csv).
A good complementary tool is
[GeoBase](http://opentraveldata.github.io/geobases/),
a Python-based software able to access any travel-related data source.

OpenTREP makes an extensive use of existing open-source libraries for
increased functionality, speed and accuracy. In particular the
[Boost](http://www.boost.org) (C++ Standard Extensions),
[Xapian](http://xapian.org) and [SOCI](http://soci.sourceforge.net)
libraries are used.

# Docker images

## OpenTREP Docker images
Docker images provide ready-to-use environments, and are available on
[Docker Cloud](https://cloud.docker.com/u/opentrep/repository/docker/opentrep/search-travel)
and [Quay.io](https://quay.io/repository/trep/opentrep):
```bash
$ docker pull opentrep/search-travel:legacy # for Docker.io
$ docker pull quay.io/trep/opentrep # for Quay.io
$ docker run --rm -it opentrep/search-travel:legacy bash
```

See https://github.com/trep/opentrep/tree/master/gui/legacy for more details.

## General purpose C++/Python Docker images
General purpose Docker images for C++/Python development are also available from
[Docker Cloud](https://cloud.docker.com/u/cpppythondevelopment/repository/docker/cpppythondevelopment/base/general).
Those Docker images allow to develop on the major Linux distributions, _i.e._,
CentOS, Debian and Ubuntu.

### CentOS 7
```bash
$ docker pull cpppythondevelopment/base:centos7
$ docker run -t cpppythondevelopment/base:centos7 bash
[build@2..c ~]$ $ mkdir -p ~/dev/geo && cd ~/dev/geo
[build@2..c geo]$ git clone https://github.com/trep/opentrep.git
[build@2..c geo]$ cd opentrep && mkdir build && cd build
[build@2..c build (master)]$ cmake3 -DCMAKE_INSTALL_PREFIX=${HOME}/dev/deliveries/opentrep-99.99.99 -DCMAKE_BUILD_TYPE:STRING=Debug -DINSTALL_DOC:BOOL=ON -DRUN_GCOV:BOOL=OFF -DLIB_SUFFIX= ..
```

### Ubuntu 18.04 LTS Bionic Beaver
```bash
$ docker pull cpppythondevelopment/base:ubuntu1804
$ docker run -t cpppythondevelopment/base:ubuntu1804 bash
[build@2..c ~]$ $ mkdir -p ~/dev/geo && cd ~/dev/geo
[build@2..c geo]$ git clone https://github.com/trep/opentrep.git
[build@2..c geo]$ cd opentrep && mkdir build && cd build
[build@2..c build (master)]$ cmake -DCMAKE_INSTALL_PREFIX=${HOME}/dev/deliveries/opentrep-99.99.99 -DCMAKE_BUILD_TYPE:STRING=Debug -DINSTALL_DOC:BOOL=ON -DRUN_GCOV:BOOL=OFF -DLIB_SUFFIX= ..
```

### Debian 9 Stretch
```bash
$ docker pull cpppythondevelopment/base:debian9
$ docker run -t cpppythondevelopment/base:debian9 bash
[build@2..c ~]$ $ mkdir -p ~/dev/geo && cd ~/dev/geo
[build@2..c geo]$ git clone https://github.com/trep/opentrep.git
[build@2..c geo]$ cd opentrep && mkdir build && cd build
[build@2..c build (master)]$ cmake -DCMAKE_INSTALL_PREFIX=${HOME}/dev/deliveries/opentrep-99.99.99 -DCMAKE_BUILD_TYPE:STRING=Debug -DINSTALL_DOC:BOOL=ON -DRUN_GCOV:BOOL=OFF -DLIB_SUFFIX= ..
```

### Common to all the above-mentioned Linux distributions
```bash
[build@2..c build (master)]$ make install
[build@2..c build (master)]$ ./opentrep/opentrep-indexer
[build@2..c build (master)]$ ./opentrep/opentrep-searcher -q "nice san francisco"
```

# Native installation (without Docker)

## RPM-based distributions (eg, Fedora/CentOS/RedHat)
Since OpenTREP has been approved as an official package of Fedora/CentOS/RedHat
(see the
[review request on Bugzilla for further details](https://bugzilla.redhat.com/show_bug.cgi?id=866265)),
just use [DNF](https://fedoraproject.org/wiki/DNF) (or
[Yum](https://fedoraproject.org/wiki/Yum) for the older distributions):
```bash
$ dnf -y install opentrep opentrep-doc
```

## Installation from the sources
### Clone the Git repository
The GitHub repository may be cloned as following:
```bash
$ mkdir -p ~/dev/geo && cd ~/dev/geo
$ git clone https://github.com/trep/opentrep.git
$ cd opentrep
$ git checkout master
```

### Alternatively, download and extract the tar-ball
GitHub generates
[tar-balls on the fly for every tagged release](https://github.com/trep/opentrep/releases).
For instance:
```bash
$ wget https://github.com/trep/opentrep/archive/opentrep-0.07.3.tar.gz
```

Note that SourceForge also stores some
[older archived tar-balls](https://sourceforge.net/projects/opentrep/files/opentrep/).

### Installation of the dependencies
#### On Linux
The following packages may be needed (Fedora/RedHat/CentOS names on the left
hand side, Debian/Ubuntu names on the right hand side; names for other
Linux distributions may vary):
* `cmake` (or `cmake3` on CentOS)
* `gcc-c++` / `gcc`, `g++`
* `boost-devel` (or `boost169-devel` on CentOS) / `libboost-all-dev`
* `xapian-core-devel` / `libxapian-dev`
* `python-devel` / `python`, `libpython-dev`
* `libicu-devel` / `libicu-dev`
* `soci-mysql-devel`, `soci-sqlite3-devel` / SOCI needs to be installed from
  the sources on Debian, Ubuntu and MacOS (see the dedicated section below)
* `sqlite3-devel` / `sqlite3`, `libsqlite3-dev`
* `mariadb-devel` / `libmariadb-dev`, `libmysql++-dev`
* `protobuf-devel`, `protobuf-compiler` / `libprotobuf-dev`, `protobuf-compiler`
* `readline-devel` / `libreadline-dev`, `libncurses5-dev`
* `ghostscript`
* `doxygen`
* (optional) `tetex-latex` / `texlive-latex-recommended`
* (optional) `rpm-build`

For instance, the following subsections show respective installation commands
for a few famous Linux distributions.

##### Fedora
* General C++-base stack:
```bash
$ dnf -y install git-all bash-completion gcc-c++ cmake boost-devel \
 xapian-core-devel soci-mysql-devel soci-sqlite3-devel readline-devel \
 sqlite-devel mariadb-devel libicu-devel protobuf-devel protobuf-compiler
```
* Python development (the package names may vary):
```bash
$ dnf -y install python-devel python-pip
```
* Documentation tools:
```bash
$ dnf -y install doxygen ghostscript "tex(latex)"
```

##### CentOS
* General C++-based stack:
```bash
$ yum -y install git-all bash-completion gcc-c++ cmake boost169-devel \
 xapian-core-devel soci-mysql-devel soci-sqlite3-devel readline-devel \
 sqlite-devel mariadb-devel libicu-devel protobuf-devel protobuf-compiler
```
* Python development (the package names may vary):
```bash
$ yum -y install python-devel python-pip
```
* Documentation tools:
```bash
$ yum -y install doxygen ghostscript "tex(latex)"
```

##### Debian/Ubuntu
* Various C++ and Python packages:
```bash
$ apt-get -y install locales && locale-gen "en_US.UTF-8"
$ apt-get -y install zlib1g-dev libbz2-dev lsb-release libgmp-dev \
   libgmp-dev gcc g++ clang cppcheck cmake libboost-all-dev libxapian-dev graphviz \
   libreadline-dev libncurses5-dev libczmq-dev libzmq3-dev libssl-dev libffi-dev \
   sqlite3 libsqlite3-dev libmariadb-dev libmysql++-dev postgresql-server-dev-all \
   libicu-dev libprotobuf-dev protobuf-compiler
```
* Python development (the package names may vary)
```bash
$ apt-get -y install python libpython-dev python3 libpython3-dev
```
* Documentation tools
```bash
$ apt-get -y install doxygen ghostscript texlive-latex-recommended
```

#### On MacOS
* With [Homebrew](http://brew.sh):
```bash
$ brew install boost boost-python boost-python3 cmake libedit \
  xapian sqlite mysql icu4c protobuf protobuf-c doxygen
$ brew install homebrew/portable-ruby/portable-readline
```

### ICU
* Install ICU with Homebrew
```bash
$ brew install icu4c
```

### Boost

#### MacOS
* Prepare the build and install directories:
```bash
$ BOOST_VER="1_71_0"
$ BOOST_VER_DOT="$(echo ${BOOST_VER}|sed -e s/_/./g)"
$ sudo mkdir -p /opt/boost && chown -R ${USER} /opt/boost
$ sudo mkdir -p /usr/local/boost_${BOOST_VER} && chown -R ${USER} /usr/local/boost_${BOOST_VER}
$ cd /opt/boost
$ sudo wget https://dl.bintray.com/boostorg/release/${BOOST_VER_DOT}/source/boost_${BOOST_VER}.tar.bz2
```

```bash
$ cmake -DCMAKE_BUILD_TYPE=Debug -DSOCI_ASAN=ON -DCMAKE_VERBOSE_MAKEFILE=OFF \
  -DSOCI_TESTS=OFF -DSOCI_STATIC=OFF -DSOCI_DB2=OFF -DSOCI_EMPTY=ON \
  -DSOCI_FIREBIRD=OFF -DSOCI_MYSQL=ON -DSOCI_ODBC=OFF -DSOCI_ORACLE=OFF \
  -DSOCI_POSTGRESQL=ON -DSOCI_SQLITE3=ON ../..
```

#### CentOS
* On CentOS, the version of Boost is often outdated, for instance
  [Boost 1.53 on CentOS 7](https://git.centos.org/rpms/boost/releases).
  It may have adversarial effects, for instance on CentOS 7, where
  [Python 3 libraries have been upgraded from Python 3.4 to
  Python 3.6](https://lists.fedoraproject.org/archives/list/epel-announce@lists.fedoraproject.org/thread/EGUMKAIMPK2UD5VSHXM53BH2MBDGDWMO/),
  but where Boost 1.53 Python libraries have not been upgraded accordingly,
  leading for the least to [linking errors](https://bugzilla.redhat.com/show_bug.cgi?id=1702242).
* Hopefully, [EPEL](https://fedoraproject.org/wiki/EPEL) provides
  more recent Boost releases, for instance
  [Boost 1.69 on RHEL/CentOS 7](https://src.fedoraproject.org/rpms/boost169/tree/epel7).
* Those additional specific Boost packages may be installed in parallel
  to the officiial Boost packages.
  CMake has however to be made aware of that additional Boost package and,
  equally importantly, of its location on the file-system.
* Example of additional CMake configuration variables for
  [Boost 1.69 on CentOS 7](https://src.fedoraproject.org/rpms/boost169/tree/epel7):
```bash
$ cmake3 [...] \
  -DBOOST_LIBRARYDIR=/usr/lib64/boost169 -DBOOST_INCLUDEDIR=/usr/include/boost169 \
  -DBoost_ADDITIONAL_VERSIONS="1.69 1.69.0" \
  [...]
```
 
### SOCI
* On Debian, Ubuntu and MacOS, as of mid-2019, SOCI 4.0 has still
  not been released, and `soci-mysql` is no longer available.
  Hence, SOCI must be built from the sources. The following shows
  how to do that on MacOS (on Debian/Ubuntu, one can have a look at
  the part installing SOCI on the
  [C++/Python Docker files](https://github.com/cpp-projects-showcase/docker-images))

* Download and prepare SOCI:
```bash
$ sudo mkdir -p /opt/soci && chown -R ${USER} /opt/soci
$ git clone https://github.com/SOCI/soci.git /opt/soci/socigit
```

#### General Unix/Linux
```bash
$ cd /opt/soci/socigit
$ mkdir -p build/head && cd build/head
$ cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -DSOCI_CXX_C11=ON -DSOCI_TESTS=OFF ..
```

#### Debian
```bash
$ wget https://github.com/trep/opentrep/raw/master/ci-scripts/soci-debian-cmake.patch -O /opt/soci/soci-debian-cmake.patch
$ cd /opt/soci/socigit
$ patch -p1 < ./soci-debian-cmake.patch
$ mkdir -p build/head && cd build/head
$ cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -DSOCI_CXX_C11=ON -DSOCI_TESTS=OFF ..
```

#### MacOS
* Configure SOCI:
```bash
$ mkdir /opt/soci/build/head && cd /opt/soci/build/head
$ cmake -DCMAKE_INSTALL_PREFIX=/usr \
  -DCMAKE_BUILD_TYPE=Debug -DSOCI_CXX_C11=ON \
  -DSOCI_ASAN=ON -DCMAKE_VERBOSE_MAKEFILE=OFF \
  -DSOCI_TESTS=OFF -DSOCI_STATIC=OFF -DSOCI_DB2=OFF -DSOCI_EMPTY=ON \
  -DSOCI_FIREBIRD=OFF -DSOCI_MYSQL=ON -DSOCI_ODBC=OFF -DSOCI_ORACLE=OFF \
  -DSOCI_POSTGRESQL=ON -DSOCI_SQLITE3=ON ../..
$ make
$ sudo make install
```

### Building the library and test binary
To customize OpenTREP to your environment, you can alter
the installation directory:
```bash
export INSTALL_BASEDIR="${HOME}/dev/deliveries"
export TREP_VER="0.07.3"
if [ -d /usr/lib64 ]; then LIBSUFFIX="64"; fi
export LIBSUFFIX_4_CMAKE="-DLIB_SUFFIX=$LIBSUFFIX"
```

Then, as usual:
* To configure the project, type something like:
```bash
$ mkdir build && cd build
$ cmake -DCMAKE_INSTALL_PREFIX=${INSTALL_BASEDIR}/opentrep-${TREP_VER} \
   -DCMAKE_BUILD_TYPE:STRING=Debug -DENABLE_TEST:BOOL=ON \
   -DINSTALL_DOC:BOOL=ON -DRUN_GCOV:BOOL=OFF ${LIBSUFFIX_4_CMAKE} ..
```

* With a specific version of Boost, say
  [Boost 1.69](https://src.fedoraproject.org/rpms/boost169)
  installed in a parallel way (for instance, as an optional module):
```bash
$ cmake -DCMAKE_INSTALL_PREFIX=${INSTALL_BASEDIR}/opentrep-${TREP_VER} \
        -DCMAKE_BUILD_TYPE:STRING=Debug -DENABLE_TEST:BOOL=ON \
        -DBOOST_LIBRARYDIR=/usr/lib64/boost169 -DBOOST_INCLUDEDIR=/usr/include/boost169 \
        -DBoost_ADDITIONAL_VERSIONS="1.69 1.69.0" \
        -DINSTALL_DOC:BOOL=ON -DRUN_GCOV:BOOL=OFF ${LIBSUFFIX_4_CMAKE} ..
```

* For an _ad hoc_ installation:
```bash
INSTALL_DIR=/var/www/webapps/opentrep/trep

cmake -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
   -DCMAKE_BUILD_TYPE:STRING=Debug -DINSTALL_DOC:BOOL=OFF \
   -DRUN_GCOV:BOOL=OFF ${LIBSUFFIX_4_CMAKE} ..
```

* On MacOS, a few software (_e.g._, ICU and Readline) are not in
the standard place. So, the `cmake` command becomes:
```bash
$ export CMAKE_CXX_FLAGS="-Wno-mismatched-new-delete"; \
  cmake -DCMAKE_INSTALL_PREFIX=${INSTALL_BASEDIR}/opentrep-$TREP_VER \
   -DREADLINE_ROOT=/usr/local/opt/portable-readline \
   -DREADLINE_INCLUDE_DIR=/usr/local/opt/portable-readline/include \
   -DREADLINE_LIBRARY=/usr/local/opt/libedit/lib/libedit.dylib \
   -DICU_ROOT=/usr/local/opt/icu4c \
   -DCMAKE_BUILD_TYPE:STRING=Debug -DINSTALL_DOC:BOOL=ON \
   -DRUN_GCOV:BOOL=OFF ${LIBSUFFIX_4_CMAKE} ..
```

* To build the project, type:
```bash
$ make
```
* To test the project, type:
```bash
$ make check
```
* To install the library (`libopentrep*.so*`) and the binary (`opentrep`),
  just type:
```bash
  make install
  cd ${INSTALL_BASEDIR}
  rm -f opentrep-stable && ln -s opentrep-${TREP_VER} opentrep-stable
  cd -
```
* To package the source files, type:
```bash
$ make dist
```
* To package the binary and the documentation:
```bash
$ make package
```

* Install the latest
  [OpenTravelData (OPTD) POR data file](https://github.com/opentraveldata/opentraveldata/tree/master/opentraveldata/optd_por_public_all.csv).
  Note that OpenTREP no longer ships with (full) OPTD data files;
  only [test files](https://github.com/trep/opentrep/raw/master/data/por/test_optd_por_public.csv)
  are shipped.
  The OPTD POR data file (`optd_por_public_all.csv`) has therefore
  to be downloaded aside, usually renamed as `optd_por_public.csv`,
  and the OpenTREP binaries have then to be referred to the file-path
  of that POR data file.
```bash
$ wget \
  https://github.com/opentraveldata/opentraveldata/raw/master/opentraveldata/optd_por_public_all.csv \
  -O ${INSTALL_BASEDIR}/share/opentrep/data/por/optd_por_public.csv
```

* To run the local binary version:
```bash
$ ./opentrep/opentrep-{dbmgr,indexer,searcher}
```
* To run the installed version (the first following line must be done once
  per session):
```bash
$ export TREP_LIB="${INSTALL_BASEDIR}/opentrep-${TREP_VER}/lib${LIBSUFFIX}"
$ export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${TREP_LIB}"
$ ${INSTALL_BASEDIR}/opentrep-${TREP_VER}/bin/opentrep-{dbmgr,indexer,searcher}
```

## Underlying (relational) database, SQLite or MySQL/MariaDB, if any
OpenTREP may use, if so configured, a relational database. For now,
two database products are supported, SQLite3 and MySQL/MariaDB.
The database accelerates the look up of POR by (IATA, ICAO, FAA) codes
and of Geonames ID. When OpenTREP is configured to run without database,
those codes and Geonames ID are full-text searched directly with Xapian.
Note that the database can be managed directly, _i.e._, without the
OpenTREP search interface on top of it, thanks to the `opentrep-dbmgr`
utility, which is detailed below.

# Indexing the POR data
## Filling the (relational) database, SQLite or MySQL/MariaDB, if any
Here, for clarity reason, we use the local version. It is easy (see above)
to derive the same commands with the installed version.

* The following command prompts a Shell:
```bash
$ ./opentrep/opentrep-dbmgr -t sqlite -p ${INSTALL_BASEDIR}/share/opentrep/data/por/optd_por_public.csv
```

* Then, within the `opentrep>` shell, a typical sequence for SQLite would be:
```bash
 info
 create_user
 fill_from_por_file
 list_nb
 list_by_iata nce
 list_by_icao lfmn
 list_by_faa afm
 list_by_geonameid 6299418
```

* The following command prompts a shell:
```bash
./opentrep/opentrep-dbmgr -t mysql -p ${INSTALL_BASEDIR}/share/opentrep/data/por/optd_por_public.csv
```

* Then, within the `opentrep>` shell, a typical sequence for MySQL/MariaDB
  would be:
```bash
 reset_connection_string db=mysql user=root password=<passwd>
 create_user
 reset_connection_string db=trep_trep user=trep password=trep
 fill_from_por_file
 list_nb
 list_by_iata nce
 list_by_icao lfmn
 list_by_faa afm
 list_by_geonameid 6299418
```

## Xapian indexing with standard installation
By default, the Xapian indexer runs without filling any relational database,
as that step can be performed independantly by `opentrep-dbmgr`,
as seen above.

* Xapian indexing without any relational database:
```bash
$ ./opentrep/opentrep-indexer -p ${INSTALL_BASEDIR}/share/opentrep/data/por/optd_por_public.csv
```

* Xapian indexing and filling and indexing the SQLite database:
```bash
$ ./opentrep/opentrep-indexer -t sqlite -p ${INSTALL_BASEDIR}/share/opentrep/data/por/optd_por_public.csv
```

* Xapian indexing and filling and indexing the MySQL/MariaDB database:
```bash
$ ./opentrep/opentrep-indexer -t mysql -p ${INSTALL_BASEDIR}/share/opentrep/data/por/optd_por_public.csv
```

* There is an option to not even touch Xapian at all, for instance to check
  that the
  [OpenTravelData (OPTD) POR data file](http://github.com/opentraveldata/opentraveldata/tree/master/opentraveldata/optd_por_public_all.csv)
  is well-formed:
```bash
$ ./opentrep/opentrep-indexer -x 0 -p ${INSTALL_BASEDIR}/share/opentrep/data/por/optd_por_public.csv
```

## Xapian indexing for an ad hoc deployed Web application
* Xapian indexing without any relational database:
```bash
$ ./opentrep/opentrep-indexer -d /var/www/webapps/opentrep/trep/traveldb -p ${INSTALL_BASEDIR}/share/opentrep/data/por/optd_por_public.csv
```

* (Optional) Filling and indexing the SQLite database:
```bash
$ ./opentrep/opentrep-dbmgr -t sqlite -s /var/www/webapps/opentrep/trep/sqlite_travel.db -p ${INSTALL_BASEDIR}/share/opentrep/data/por/optd_por_public.csv
    create_user
    fill_from_por_file
    quit
```

# Searching
* Searching without any relational database support:
```bash
$ ./opentrep/opentrep-searcher -q "nce sfo"
```

* Searching with SQLite (note that it should be quicker than without a database):
```bash
$ ./opentrep/opentrep-searcher -t sqlite -q "nce sfo"
```

* Searching with MySQL/MariaDB:
```bash
$ ./opentrep/opentrep-searcher -t mysql -q "nce sfo"
```

* Searching with SQLite (with Xapian and SQLite DB in a `webapps` directory):
```bash
$ ./opentrep/opentrep-searcher -d /var/www/webapps/opentrep/trep/traveldb -t sqlite -s /var/www/webapps/opentrep/trep/sqlite_travel.db -q "nce sfo"
```

# Deployment stages
The idea is to have at least two pieces of infrastructure (SQL database,
Xapian index) in parallel:
* one is used by the production;
* the other one used as a staging platform in order to test
  and validate a new version.

Once the new version has been validated, the two pieces of infrastructure
can then be interverted, ie, the production becomes the new version,
and the older version ends up in staging.

It means that all programs have to choose which version they want to work on.
That version may even be toggled in live.

That method to deploy in production through a staging process
is even more needed by the fact that indexing a new POR data file
takes up to 30 minutes in the worst case. So, we cannot afford 30-45 minutes
of downtime everytime a new POR data file is released (potentially every day).

With that staging process, it is even possible to fully automate
the re-indexing after a new POR data file release: once the new release
has been cleared by
[QA (Quality Assurance)](http://en.wikipedia.org/wiki/Quality_assurance)
on staging, it becomes production.

The corresponding command-line option for the various programs
(`opentrep-dbmgr`, `opentrep-indexer`, `opentrep-searcher`) is `-m`.

* For instance, to build the Xapian index/database for deployment number 0:
```bash
$ ./opentrep/opentrep-indexer -m 0 -d /var/www/webapps/opentrep/trep/traveldb
$ chown -R apache.apache /var/www/webapps/opentrep/trep
$ ls -laFh /var/www/webapps/opentrep/trep/
-rw-r--r-- 1 apache apache  16M Oct 14 2018 sqlite_travel.db0
drwxr-xr-x 2 apache apache 4.0K Oct 14 2018 traveldb0/
```

* And to build the Xapian index/database for deployment number 1:
```bash
$ ./opentrep/opentrep-indexer -m 1 -d /var/www/webapps/opentrep/trep/traveldb 
$ chown -R apache.apache /var/www/webapps/opentrep/trep
$ ls -laFh /var/www/webapps/opentrep/trep/
-rw-r--r-- 1 apache apache  16M Oct 14 2018 sqlite_travel.db1
drwxr-xr-x 2 apache apache 4.0K Oct 14 2018 traveldb1/
```

# Index, or not, non-IATA POR
There is also a command-line option, namely `-n`, to state whether or not
the non-IATA-referenced POR should be included/parsed and indexed.

By default, and historically, only the POR, which are referenced by
[IATA](http://en.wikipedia.org/wiki/IATA_airport_code)
(ie, which have a specific IATA code), are indexed (and may be searched for)
in OpenTREP.

POR are also referenced by other international organizations, such as
[ICAO](http://en.wikipedia.org/wiki/ICAO_airport_code)
or [UN/LOCODE](http://en.wikipedia.org/wiki/UN/LOCODE),
and may not be referenced by IATA (in which case their IATA code
is left empty).

As of October 2018, there are around 110,000 POR in
[OpenTravelData (OPTD)](http://github.com/opentraveldata/opentraveldata),
the reference data source for OpenTREP:
*  Around 20,000 POR are referenced by IATA
*  Around 90,000 POR are not referenced by IATA, but referenced
   by other international organizations (eg, ICAO, UN/LOCODE)

* Indexing 20,000 POR takes already a few minutes on standard hardware.
* Indexing 110,000 POR would take 15 to 20 minutes.

Once indexed, all those POR become searchable. That flag is therefore
only used at indexing time (ie, by the `opentrep-dbmgr` and
`opentrep-indexer` programs).

# Installing a Python virtual environment
All the details are explained on a
[dedicated procedure](http://github.com/machine-learning-helpers/induction-python/tree/master/installation/virtual-env),
which works for the major Linux distributions and on MacOS.

The procedure first installs a specific version of Python (as of end of 2018,
3.7.1) thanks to Pyenv, then install `pipenv` thanks to the `pip` utility
provided with that specific Python version.

# Checking that the Python module works
* With the standard installation:
```bash
PYTHON_VERSION=$(python3 --version 2>&1 | cut -d' ' -f2,2 | cut -d'.' -f1,2)
PYTHONPATH=${INSTALL_BASEDIR}/opentrep-${TREP_VER}/lib${LIBSUFFIX}:${INSTALL_BASEDIR}/opentrep-${TREP_VER}/lib${LIBSUFFIX}/python${PYTHON_VERSION}/site-packages/pyopentrep \
 pipenv run python -c "import pyopentrep; \
 openTrepLibrary = pyopentrep.OpenTrepSearcher(); \
 initOK = openTrepLibrary.init ('/tmp/opentrep/xapian_traveldb', 'sqlite', '/tmp/opentrep/sqlite_travel.db', 0, 'pyopentrep.log'); \
 print (openTrepLibrary.search ('S', 'los las'))"
```

* With an _ad hoc_ installation:
```bash
PYTHON_VERSION=$(python3 --version 2>&1 | cut -d' ' -f2,2 | cut -d'.' -f1,2)
PYTHONPATH=${INSTALL_DIR}/lib${LIBSUFFIX}:${INSTALL_BASEDIR}/lib${LIBSUFFIX}/python${PYTHON_VERSION}/site-packages/pyopentrep \
 pipenv run python -c "import pyopentrep; \
 openTrepLibrary = pyopentrep.OpenTrepSearcher(); \
 initOK = openTrepLibrary.init ('/var/www/webapps/opentrep/trep/traveldb', 'mysql', 'db=trep_trep user=trep password=trep', 0, 'pyopentrep.log'); \
 print (openTrepLibrary.search ('S', 'los las'))"
```

# (Optional) Running the Django-based application server
```bash
$ export TREP_LIB=${INSTALL_BASEDIR}/opentrep-${TREP_VER}/lib${LIBSUFFIX}
$ export TREP_TRAVELDB=/tmp/opentrep/traveldb
$ export TREP_LOG=django_trep.log
$ cd gui/django/opentrep
$ # The first time, the database must be initialised:
$ #./manage.py syncdb localhost:8000
$ # Add the Python library directories to PYTHONPATH:
$ export PYTHONPATH=${INSTALL_BASEDIR}/opentrep-${TREP_VER}/lib${LIBSUFFIX}:${INSTALL_BASEDIR}/opentrep-${TREP_VER}/lib${LIBSUFFIX}/python${PYTHON_VERSION}/site-packages/pyopentrep
$ # Start the Django Web application server
$ ./manage.py runserver localhost:8000
$ # In another Shell, check that everything went fine with, for instance:
$ open http://localhost:8000/search?q=rio de janero reykyavik sna francicso
```

