
FROM gcc:14

WORKDIR /app

# DEPENDENCIES 
RUN apt-get update && apt-get install -y \
    libboost-system-dev \
    libboost-thread-dev \
    cmake \
    g++ \
# SDL 
    build-essential git make \
    pkg-config cmake ninja-build gnome-desktop-testing libasound2-dev libpulse-dev \
    libaudio-dev libfribidi-dev libjack-dev libsndio-dev libx11-dev libxext-dev \
    libxrandr-dev libxcursor-dev libxfixes-dev libxi-dev libxss-dev libxtst-dev \
    libxkbcommon-dev libdrm-dev libgbm-dev libgl1-mesa-dev libgles2-mesa-dev \
    libegl1-mesa-dev libdbus-1-dev libibus-1.0-dev libudev-dev libthai-dev \
    && rm -rf /var/lib/apt/lists/*

COPY . .

# Build (adjust if using Make instead of CMake)
RUN cmake -B build -S . && cmake --build build
