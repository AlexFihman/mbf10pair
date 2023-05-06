FROM ubuntu:22.04 as build

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        libboost-all-dev \
        software-properties-common \
        autoconf \
        automake \
        libtool \
        pkg-config \
        ca-certificates \
        libssl-dev \
        wget \
        git \
        curl \
        language-pack-en \
        locales \
        locales-all \
        vim \
        gdb \
        valgrind && \
    apt-get clean

COPY ./mbf9rng ./mbf9rng

RUN cd mbf9rng;./make.sh



FROM ubuntu:22.04

VOLUME /golem/input /golem/output

WORKDIR /golem/work

COPY --from=build /mbf9rng /golem/work/
