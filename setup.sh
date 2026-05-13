#!/bin/bash
echo "Setting up Pi-ChessBox dependencies..."

# Install ssobjects
echo "Installing ssobjects..."
cd ssobjects && sudo ./install.sh && cd ..

# Check if SDL2 is already installed
if pkg-config --exists sdl2; then
    echo "SDL2 already installed, skipping"
else
    echo "SDL2 not found - please install SDL2 2.0.12, SDL2_image 2.0.5, SDL2_ttf 2.0.15"
    echo "Download from: https://libsdl.org/download-2.0.php"
    exit 1
fi

# Build GUI
echo "Building GUI..."
mkdir -p build && cd build && cmake .. && make -j4 && cd ..

# Build controller
echo "Building controller..."
g++ -o cbcontroller_new controller.cpp common/thc.cpp -lwiringPi -I./common -I/usr/local/include/ssobjects -L/usr/local/lib -lssobjects -lpthread

echo "Done! Run ./start_chessbox.sh to start"
