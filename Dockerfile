FROM rockylinux/rockylinux:9.8-minimal

ENV TZ=Etc/UTC
ENV PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig

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
    python3-devel \
    epel-release

RUN dnf config-manager --set-enabled crb || true

#
# devcontainer
#
RUN dnf install -y \
    clang-tools-extra \
    doxygen \
    bash-completion
RUN pip install sphinx breathe sphinx_rtd_theme

ARG CI_JOB_TOKEN
RUN if [ -n "$CI_JOB_TOKEN" ]; then \
        git config --global url."https://gitlab-ci-token:${CI_JOB_TOKEN}@gitlab.seanergys.fz-juelich.de/".insteadOf "git@gitlab.seanergys.fz-juelich.de:"; \
    fi

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

#
# Rust install
#
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y --profile minimal --default-toolchain 1.95
ENV PATH="/root/.cargo/bin:$PATH"

#
# Build seanerbus server
#
WORKDIR /opt
RUN dnf install -y \
    capnproto \
    capnproto-devel
RUN git clone --recurse-submodules --revision 06aed34cc69e3de66e0e6587d786d58423a1dc30 \
    git@gitlab.seanergys.fz-juelich.de:software/demobus.git
WORKDIR /opt/demobus
RUN cargo install --path . --root /usr/local

#
# Build Seanerbus naming service
#
WORKDIR /opt
RUN git clone --recurse-submodules --revision 44893ce72181079f81b26f6140ca8d1c808dff15 \
    git@gitlab.seanergys.fz-juelich.de:software/seanerbus-naming-service.git
WORKDIR /opt/seanerbus-naming-service
RUN cargo install --path . --root /usr/local

#
# Build seanerbus cpp
#
WORKDIR /opt
RUN git clone --recurse-submodules --revision 3075f14aa9b75ac8020d8a4c0dd09f7dae3c251d \
    git@gitlab.seanergys.fz-juelich.de:software/seanerbus-cpp.git
WORKDIR /opt/seanerbus-cpp
RUN mkdir build
WORKDIR /opt/seanerbus-cpp/build
RUN cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DBUILD_SHARED_LIBS=ON && \
    make -j$(nproc) && \
    make install
RUN rm /usr/local/lib64/pkgconfig/fmt.pc && \
    rm -r /usr/local/include/fmt && \
    echo "/usr/local/lib64" > /etc/ld.so.conf.d/local.conf && \
    ldconfig

#
# Build priorisea
#
WORKDIR /app
COPY . .
RUN git submodule update --init --recursive
RUN ./autogen.sh && \
    ./configure --prefix=/usr/local && \
    make -j$(nproc) && \
    make install && \
    ldconfig
