#!/bin/sh

mkdir -p ~/.config/systemd/user
cp smbd.service ~/.config/systemd/user
systemctl --user daemon-reload

echo "Run 'systemctl --user start smbd.service' to start the service"
echo "Run 'systemctl --user start smbd.service' to stop the service"
