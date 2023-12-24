FROM alpine:3.14

RUN apk update
RUN apk add g++ git cmake make openssl sqlite-dev openssl-dev \
  spdlog-dev wget sqlite-dev python3 \
  openssl openssl-dev
# RUN wget https://download.libsodium.org/libsodium/releases/libsodium-1.0.19-stable.tar.gz && \
#   tar -xzf libsodium-1.0.19-stable.tar.gz && \
#   cd libsodium-stable && \
#   ./configure && \
#   make && make check && make install

# Testing framework
RUN git clone https://github.com/catchorg/Catch2.git && \
  cd Catch2 && \
  cmake -Bbuild -H. -DBUILD_TESTING=OFF && \
  cmake --build build/ --target install

RUN apk add py3-pip python3-dev libffi-dev
RUN pip install argon2-cffi

# Logging library
# RUN git clone https://github.com/gabime/spdlog.git && \
#   cd spdlog && mkdir build && cd build && \
#   cmake .. && make -j && make install

WORKDIR /
COPY /scripts/deploy/run.sh /run.sh
RUN chmod +x /run.sh

CMD ["/run.sh"]
