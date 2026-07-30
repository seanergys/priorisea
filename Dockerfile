FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Etc/UTC

RUN apt update && apt install -y \
    wget \
    openssh-client \
    git \
    cmake \
    libfmt-dev

#
# Build flux-security
#
WORKDIR /opt

RUN wget https://github.com/flux-framework/flux-security/releases/download/v0.15.0/flux-security-0.15.0.tar.gz && \
    tar -xzf flux-security-0.15.0.tar.gz

WORKDIR /opt/flux-security-0.15.0

RUN apt install -y \
    autoconf \
    automake \
    libtool \
    make \
    pkg-config \
    libsodium-dev \
    libjansson-dev \
    uuid-dev \
    libmunge-dev \
    libpam0g-dev \
    python3-dev
RUN ./configure --prefix=/usr/local && \
    make -j$(nproc) && \
    make install
RUN ldconfig

#
# Build flux-core
#
WORKDIR /opt

RUN wget https://github.com/flux-framework/flux-core/releases/download/v0.85.0/flux-core-0.85.0.tar.gz && \
    tar -xzf flux-core-0.85.0.tar.gz

WORKDIR /opt/flux-core-0.85.0

RUN yes | bash scripts/install-deps-deb.sh

RUN ./configure --prefix=/usr/local --with-flux-security && \
    make -j$(nproc) && \
    make install
RUN ldconfig

##
## Build seanerbus
##
#COPY seanerbus /opt/seanerbus
#
#WORKDIR /opt/seanerbus/bindings/cpp
#
#RUN git submodule update --init --recursive
#RUN mkdir build
#
#WORKDIR /opt/seanerbus/bindings/cpp/build
#
#RUN cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local && \
#    cmake --build . && \
#    cmake --install .
#
## Remove seanerbus' fmt lib
#RUN rm /usr/local/lib/libfmt*
#RUN rm -rf /usr/local/include/fmt
#RUN rm -r /usr/local/lib/pkgconfig/fmt*
#RUN ldconfig

#
# Build plugin
#
WORKDIR /app

COPY . .

RUN git submodule update --init --recursive

RUN ./autogen.sh && \
    ./configure --prefix=/usr/local && \
    make -j$(nproc) && \
    make install && \
    strip /usr/local/bin/priorisea
RUN ldconfig
