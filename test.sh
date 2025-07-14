#!/bin/bash

PROJECT_ROOT="$(pwd)"

make clean && make || { echo "Build failed"; exit 1; }

alacritty --working-directory "$PROJECT_ROOT" -e ./build/server &

sleep 1

cat > "$PROJECT_ROOT/login_client.exp" <<'EOF'
#!/usr/bin/expect -f
set username [lindex $argv 0]
set password [lindex $argv 1]
set timeout 30
spawn ./build/client
# Wait for the animated header and progress bar
sleep 2
expect {
    -re "Enter your choice:" { sleep 1; send "2\r" }
}
# Wait for the username prompt
sleep 1
expect {
    -re "USERNAME" { exp_continue }
    -re "Enter username:" { sleep 1; send "$username\r" }
    -re "Enter desired username:" { sleep 1; send "$username\r" }
}
# Wait for the password prompt
sleep 1
expect {
    -re "PASSWORD" { exp_continue }
    -re "Enter password:" { sleep 1; send "$password\r" }
    -re "Enter secure password:" { sleep 1; send "$password\r" }
}
interact
EOF

chmod +x "$PROJECT_ROOT/login_client.exp"

alacritty --working-directory "$PROJECT_ROOT" -e "$PROJECT_ROOT/login_client.exp" test test &

alacritty --working-directory "$PROJECT_ROOT" -e "$PROJECT_ROOT/login_client.exp" vision test &

(sleep 10; rm -f "$PROJECT_ROOT/login_client.exp") &