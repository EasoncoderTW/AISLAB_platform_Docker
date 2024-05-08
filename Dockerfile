FROM ubuntu:20.04

LABEL maintainer "Eason Yeh <e24096695@gs.ncku.edu.tw>"

ENV TZ=Asia/Taipei
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Location in the container file system where the host gem5 working
# directory is expected to be mounted.
ARG mountdir=/workspace
ARG UID=1000
ARG GID=1000
ARG NAME=user

## Install dependency
ENV LINUX_PACKAGES bc \
    bison \
    crossbuild-essential-arm64 \
    flex \
    kmod \
    libssl-dev \
    make \
    libc6-dev \
    libncurses5-dev

ENV QEMU_PACKAGES build-essential \
    ccache \
    chrpath \
    clang \
    cpio \
    diffstat \
    gawk \
    gettext \
    git \
    git-core \
    glusterfs-common \
    libaio-dev \
    libattr1-dev \
    libbrlapi-dev \
    libbz2-dev \
    libcacard-dev \
    libcap-ng-dev \
    libcurl4-gnutls-dev \
    libdrm-dev \
    libepoxy-dev \
    libfdt-dev \
    libgbm-dev \
    libibverbs-dev \
    libiscsi-dev \
    libjemalloc-dev \
    libjpeg-turbo8-dev \
    liblzo2-dev \
    libncursesw5-dev \
    libnfs-dev \
    libnss3-dev \
    libnuma-dev \
    libpixman-1-dev \
    librados-dev \
    librbd-dev \
    librdmacm-dev \
    libsasl2-dev \
    libseccomp-dev \
    libsnappy-dev \
    libspice-protocol-dev \
    libspice-server-dev \
    libssh-dev \
    libssl-dev \
    libusb-1.0-0-dev \
    libusbredirhost-dev \
    libvdeplug-dev \
    libvte-2.91-dev \
    libzstd-dev \
    locales \
    make \
    ninja-build \
    python3-yaml \
    python3-sphinx \
    python3-sphinx-rtd-theme \
    rsync \
    samba \
    sparse \
    texinfo \
    unzip \
    xfslibs-dev

ENV TOOL_PACKAGES curl \
    gdb-multiarch \
    htop \
    procps \
    sudo \
    telnet \
    vim \
    wget \
    sudo \
    emacs

ENV ARMNN_PACKAGES scons \
    autoconf \
    libtool \
    xxd \
    cmake \
    colordiff

RUN chmod 1777 /tmp    

## Install dependency
RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get -y install apt-utils
RUN DEBIAN_FRONTEND=noninteractive apt-get -y install $LINUX_PACKAGES
RUN DEBIAN_FRONTEND=noninteractive apt-get -y install $QEMU_PACKAGES
RUN DEBIAN_FRONTEND=noninteractive apt-get -y install $TOOL_PACKAGES
RUN DEBIAN_FRONTEND=noninteractive apt-get -y install $ARMNN_PACKAGES
RUN dpkg -l $LINUX_PACKAGES $QEMU_PACKAGES $TOOL_PACKAGES $ARMNN_PACKAGES | sort > /packages.txt

## Install Environment Modules tool for managing/switching software environment
RUN DEBIAN_FRONTEND=noninteractive apt install -y autoconf tcl-dev tk-dev
RUN cd /usr/local/src \
    && wget -O - https://github.com/cea-hpc/modules/releases/download/v5.0.1/modules-5.0.1.tar.gz 2> /dev/null | tar zxv \
    && cd modules-5.0.1 \
    && ./configure \
    && make -j $(nproc) \
    && make install

## Add kitware's repository for cmake to apt sources list
RUN DEBIAN_FRONTEND=noninteractive apt install -y software-properties-common lsb-release \
    && apt clean all \
    && wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null \
    | gpg --dearmor - | tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null \
    && apt-add-repository "deb https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main"

## Install packages for IREE
RUN DEBIAN_FRONTEND=noninteractive apt install -y cmake ninja-build clang lld libncurses5
RUN DEBIAN_FRONTEND=noninteractive apt install -y python3-pip
ENV PATH=$PATH:/home/$NAME/.local/bin

RUN groupadd -g $GID -o $NAME

RUN useradd -u $UID -m -g $NAME -G plugdev $NAME && \
    echo "$NAME ALL = NOPASSWD: ALL" > /etc/sudoers.d/user && \
    chmod 0440 /etc/sudoers.d/user

RUN chown -R $NAME:$NAME /home/$NAME

WORKDIR $mountdir
# Watermark $mountdir in docker image, so we can detect if a host directory is
# actualy mounted on top when we run the container.
RUN touch .in-docker-container


ADD startup.sh /
RUN set -x \
    && sed -e s@#MOUNTDIR#@mountdir=$mountdir@ /startup.sh \
    > /usr/local/bin/startup \
    && chmod 755 /usr/local/bin/startup \
    && rm /startup.sh

USER $NAME

# Initialize the Environment Modules tool
RUN echo "source /usr/local/Modules/init/bash" >> ~/.bashrc
RUN echo "module use $mountdir/docker/modules" >> ~/.bashrc

# Run the front-end script
RUN echo eval /usr/local/bin/startup >> ~/.bashrc
RUN echo "export PS1=\"\[\e[0;31m\]\u@\[\e[m\e[0;34m\]\h\[\e[m \e[0;32m\] \w[\!]\$\[\e[m\]  \"" >> ~/.bashrc



RUN echo "export PATH=\"$mountdir/qemu/build/riscv64-softmmu:\$PATH\"" >> ~/.bashrc \
    && echo "export PATH=\"$mountdir/qemu/build/riscv32-softmmu:\$PATH\"" >> ~/.bashrc \
    && echo "export PATH=\"$mountdir/riscv/bin:\$PATH\"" >> ~/.bashrc 

CMD ["/bin/bash"]
WORKDIR /workspace
VOLUME ["/workspace"]
