#!/bin/bash

PROJECT_ROOT="$(pwd)"

make clean && make || { echo "Build failed"; exit 1; }

alacritty --working-directory "$PROJECT_ROOT" -e ./build/server &

sleep 1

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

alacritty --working-directory "$PROJECT_ROOT" -e "$PROJECT_ROOT/login_client.exp" test test &

alacritty --working-directory "$PROJECT_ROOT" -e "$PROJECT_ROOT/login_client.exp" random test &

(sleep 10; rm -f "$PROJECT_ROOT/login_client.exp") &