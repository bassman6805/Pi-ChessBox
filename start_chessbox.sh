#!/bin/bash
export DISPLAY=:0
cd ~/chessbox

# Kill any existing instances
killall cbcontroller_new controller_gui stockfish8 2>/dev/null
sleep 1

# Start controller first
./cbcontroller_new &
CONTROLLER_PID=$!

# Wait for controller to start
echo "Waiting for controller to start..."
sleep 3
echo "Controller ready!"

# Start GUI
./build/controller_gui ./engine/stockfish8 -h 127.0.0.1 -p 9999

# When GUI exits, kill everything
kill $CONTROLLER_PID 2>/dev/null
killall stockfish8 2>/dev/null
echo "Chessbox stopped."
