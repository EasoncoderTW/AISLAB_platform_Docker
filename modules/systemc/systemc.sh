#!/bin/bash

VER=${1:-2.3.3}
TOPDIR=/workspace/system/systemc
SRCDIR=${TOPDIR}/systemc-${VER}
BLDDIR=${TOPDIR}/systemc-${VER}/build
LIBDIR=${TOPDIR}/systemc-${VER}/lib-linux64

[ -d ${LIBDIR} ] && echo "SystemC ${VER} library exsists (${LIBDIR})" || {
    mkdir -p ${TOPDIR}
    wget https://github.com/accellera-official/systemc/archive/refs/tags/${VER}.tar.gz -O - | tar zxC ${TOPDIR}
    #wget https://www.accellera.org/images/downloads/standards/systemc/systemc-${VER}.tgz -O - | tar zxC ${TOPDIR}
    mkdir -p ${BLDDIR}
    cd ${BLDDIR}
    CC=gcc CXX=g++ ${SRCDIR}/configure --prefix=${SRCDIR}
    make -j $(nproc) all install
}

echo "Please enter \"module load systemc/${VER}\" to load it"
