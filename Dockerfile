FROM debian:bullseye

# Necessary tools and logging
RUN apt-get update && \
    apt-get install -y gcc g++ cmake build-essential libspdlog-dev git

# Testing framework
RUN git clone https://github.com/catchorg/Catch2.git && \
    cd Catch2 && \
    cmake -Bbuild -H. -DBUILD_TESTING=OFF && \
    cmake --build build/ --target install

# Compile
WORKDIR /app
COPY . /app
RUN cmake .
RUN make

# Run
CMD ["./main"]
