#!/bin/sh
echo "this is user shell"
cd /mnt/usr
./httpd_bin 1>/dev/null &
./serial_ctrl_bin 1>/dev/null &
exit 0
