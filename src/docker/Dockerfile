
FROM ubuntu:latest

# try to suppress certain warnings during apt-get calls
ARG DEBIAN_FRONTEND=noninteractive
RUN echo 'debconf debconf/frontend select Noninteractive' | debconf-set-selections

# install of apt-utils suppresses bogus warnings later
RUN apt-get update \
  && apt-get install -y apt-utils 2>&1 | grep -v "debconf: delaying package configuration, since apt-utils is not installed" \
  && apt-get install -y \
    bison \
    build-essential \
    flex \
    git \
    perl \
  && rm -rf /var/lib/apt/lists/*

COPY . /zmdp

RUN cd /zmdp/src && make
