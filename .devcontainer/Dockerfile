FROM ubuntu:22.04

ARG IDF_VER=v5.2
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update -y && apt-get install -y \
    git git-lfs cmake ccache python3 python3-pip python3-venv flex bison gperf udev wget xz-utils file nodejs npm ninja-build \
    # Just since Windows don't like to play nice. This is why we can't have nice things
    linux-tools-generic \
    # We can remove these, They're just kind of nice to have
    vim less usbutils net-tools iproute2 clang-format

RUN git clone --recursive -b ${IDF_VER} https://github.com/espressif/esp-idf.git /opt/esp-idf &&  git -C /opt/esp-idf lfs pull && \
    git -C /opt/esp-idf submodule foreach --recursive "git lfs install && git lfs pull"

ENV IDF_PATH=/opt/esp-idf

RUN /opt/esp-idf/install.sh esp32s3 esp32c3

RUN printf '. /opt/esp-idf/export.sh\n' > /etc/profile.d/esp-idf.sh

CMD ["/bin/bash", "-c"]