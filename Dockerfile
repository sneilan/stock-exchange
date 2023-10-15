FROM alpine:3.14

RUN apk add g++ git cmake make

# Testing framework
RUN git clone https://github.com/catchorg/Catch2.git && \
    cd Catch2 && \
    cmake -Bbuild -H. -DBUILD_TESTING=OFF && \
    cmake --build build/ --target install

# Logging library
RUN git clone https://github.com/gabime/spdlog.git && \
    cd spdlog && mkdir build && cd build && \
    cmake .. && make -j && make install

# Compile
WORKDIR /app
COPY . /app
RUN cmake .
RUN make

# Run
CMD ["./main"]
