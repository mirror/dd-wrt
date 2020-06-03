#!/bin/sh

mkdir -p ~/.config/systemd/user
cp usmbd.service ~/.config/systemd/user
systemctl --user daemon-reload

echo "Run 'systemctl --user start usmbd.service' to start the service"
echo "Run 'systemctl --user start usmbd.service' to stop the service"
