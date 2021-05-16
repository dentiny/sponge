FROM ubuntu:18.04

# Install Ubuntu packages.
# Please add packages in alphabetical order.
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get -y update && \
    apt-get -y install \
      apt-utils \
      bash-completion \
      build-essential \
      clang-8 \
      clang-format-8 \
      clang-tidy-8 \
      cmake \
      doxygen \
      git \
      g++-7 \
      pkg-config \
      valgrind \
      vim \
      wget \
      zlib1g-dev

WORKDIR /hao
CMD bash

# Run container on Windows:
# docker run --name hao --mount type=bind,source=C:\Users\19108\Desktop\Hao,destination=/hao -it hao bash
# Run container on MacOS"
# docker run --name hao --mount type=bind,source=/Users/haojiang/Desktop/CS144,destination=/hao -it hao bash