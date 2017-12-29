FROM ubuntu:yakkety

# Add dependencies
RUN apt-get update && \
    apt-get -y install make gcc-arm-none-eabi

RUN apt-get -y install python3-pip
RUN pip3 install --upgrade pip
RUN pip3 install pros-cli

RUN apt-get -y install clang-format

ENV LC_ALL C.UTF-8
ENV LANG C.UTF-8

RUN mkdir -p /build/project
WORKDIR /build/project

CMD /bin/bash
