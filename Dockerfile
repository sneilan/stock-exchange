# Use the official Debian-based image as the base image
FROM debian:bullseye

# Update package repository and install necessary tools
RUN apt-get update && \
    apt-get install -y gcc g++ cmake build-essential catch2

# Install spdlog using CMake and Git
RUN apt-get install -y git && \
    git clone https://github.com/gabime/spdlog.git && \
    mkdir spdlog/build && cd spdlog/build && \
    cmake .. && make install

# Create a directory for your project inside the container
WORKDIR /app

# Copy everything from the local directory into the container's /app directory
COPY . /app

# Compile the C++ program
RUN cmake .

# Run the compiled C++ program
CMD ["./main"]

