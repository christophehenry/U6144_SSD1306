#!/bin/bash

set -e

if [ "$EUID" -ne 0 ]
    then echo "Please run as root"
    exit 1
fi

mv display /usr/bin/uc_display
cat << EOF > /etc/systemd/system/uc-display.service
[Unit]
Description=UCTronics screen display service
After=local-fs.target

[Service]
ExecStart=/usr/bin/uc_display
Restart=always

[Install]
WantedBy=multi-user.target
EOF

systemctl daemon-reload
systemctl enable uc-display
systemctl start uc-display
