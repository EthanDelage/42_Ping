FROM gcc:9.5.0-bullseye

RUN apt update && apt upgrade -y

RUN apt-get install -y \
    make \
    build-essential \
    wget

RUN wget https://ftpmirror.gnu.org/inetutils/inetutils-2.0.tar.gz && \
    tar -xvf inetutils-2.0.tar.gz && \
    cd inetutils-2.0 && \
    ./configure && \
    make && \
    make install

CMD ["/bin/bash"]