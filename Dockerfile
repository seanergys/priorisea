FROM rockylinux/rockylinux:9.8-minimal

ENV TZ=Etc/UTC
ENV PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig:$PKG_CONFIG_PATH

#
# Base packages
#
RUN microdnf install -y \
    dnf && \
    microdnf clean all
RUN dnf install -y \
    wget \
    openssh-clients \
    git \
    cmake \
    gcc \
    gcc-c++ \
    make \
    jq \
    'dnf-command(config-manager)' \
    epel-release

RUN dnf config-manager --set-enabled crb || true

#
# Build flux-security
#
WORKDIR /opt
RUN dnf install -y \
    autoconf \
    automake \
    libtool \
    pkgconf-pkg-config \
    libsodium-devel \
    jansson-devel \
    libuuid-devel \
    munge-devel \
    pam-devel \
    python3-devel \
    fmt-devel
RUN wget https://github.com/flux-framework/flux-security/releases/download/v0.15.0/flux-security-0.15.0.tar.gz && \
    tar -xzf flux-security-0.15.0.tar.gz

WORKDIR /opt/flux-security-0.15.0

RUN ./configure --prefix=/usr/local && \
    make -j$(nproc) && \
    make install && \
    ldconfig

#
# Build flux-core
#
WORKDIR /opt
RUN dnf install -y \
    zeromq-devel \
    python3-devel \
    python3-cffi \
    python3-ply \
    python3-yaml \
    lua-devel \
    lua-posix \
    hwloc-devel \
    lz4-devel \
    sqlite-devel \
    ncurses-devel \
    libarchive-devel
RUN wget https://github.com/flux-framework/flux-core/releases/download/v0.85.0/flux-core-0.85.0.tar.gz && \
    tar -xzf flux-core-0.85.0.tar.gz

WORKDIR /opt/flux-core-0.85.0

RUN ./configure --prefix=/usr/local --with-flux-security && \
    make -j$(nproc) && \
    make install && \
    ldconfig


WORKDIR /app

COPY . .

#
# Update submodules
#
ARG CI_JOB_TOKEN
RUN if [ -n "$CI_JOB_TOKEN" ]; then \
        git config --global url."https://gitlab-ci-token:${CI_JOB_TOKEN}@gitlab.seanergys.fz-juelich.de/".insteadOf "git@gitlab.seanergys.fz-juelich.de:"; \
    fi

RUN git submodule update --init --recursive

#
# Build seanerbus
#
RUN dnf install -y \
    capnproto \
    capnproto-devel
WORKDIR /app/external/seanerbus-cpp
RUN mkdir build
WORKDIR /app/external/seanerbus-cpp/build
RUN cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DBUILD_SHARED_LIBS=ON && \
    make -j$(nproc) && \
    make install
RUN rm /usr/local/lib64/pkgconfig/fmt.pc && \
    rm -r /usr/local/include/fmt && \
    ldconfig

#
# Build plugin
#
WORKDIR /app
RUN ./autogen.sh && \
    ./configure --prefix=/usr/local && \
    make -j$(nproc) && \
    make install && \
    ldconfig
