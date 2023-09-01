# Use the official Debian-based image as the base image
FROM debian:bullseye

# Update package repository and install necessary tools
RUN apt-get update && \
    apt-get install -y gcc g++ cmake build-essential libspdlog-dev git

RUN git clone https://github.com/catchorg/Catch2.git && \
    cd Catch2 && \
    cmake -Bbuild -H. -DBUILD_TESTING=OFF && \
    cmake --build build/ --target install

# Create a directory for your project inside the container
WORKDIR /app

# Copy everything from the local directory into the container's /app directory
COPY . /app

# Compile the C++ program
RUN cmake .
RUN make

# Run the compiled C++ program
CMD ["./main"]

