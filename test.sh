#!/bin/bash

# Get the absolute path to the project root
PROJECT_ROOT="$(pwd)"

# Clean and build
make clean && make || { echo "Build failed"; exit 1; }

# Start server in a new Alacritty window
alacritty --working-directory "$PROJECT_ROOT" -e ./build/server &

# Wait a moment for the server to start
sleep 1

# Create expect script for client login
cat > "$PROJECT_ROOT/login_client.exp" <<'EOF'
#!/usr/bin/expect -f
set username [lindex $argv 0]
set password [lindex $argv 1]
set timeout 10
spawn ./build/client
expect "Enter your choice:"
send "2\r"
expect "Enter username:"
send "$username\r"
expect "Enter password:"
send "$password\r"
interact
EOF

chmod +x "$PROJECT_ROOT/login_client.exp"

# Start first client and auto-login as test|test
alacritty --working-directory "$PROJECT_ROOT" -e "$PROJECT_ROOT/login_client.exp" test test &

# Start second client and auto-login as regex|test
alacritty --working-directory "$PROJECT_ROOT" -e "$PROJECT_ROOT/login_client.exp" regex test &

# Clean up expect script after a short delay (optional)
(sleep 10; rm -f "$PROJECT_ROOT/login_client.exp") &