#!/bin/sh
echo "this is user shell"
cd /mnt/usr
export LD_LIBRARY_PATH=/mnt/usr
if [ -e update/serial_ctrl_bin ]
then
	rm ./serial_ctrl_bin
	mv update/serial_ctrl_bin /mnt/usr
	chmod +x ./serial_ctrl_bin
fi
if [ -e update/httpd_bin ]
then
	rm ./httpd_bin
	mv update/httpd_bin /mnt/usr
	chmod +x ./httpd_bin
fi

if [ -e update/zlog.conf ]
then
	rm ./zlog.conf
	mv update/zlog.conf /mnt/usr
	chmod 777 ./zlog.conf
fi

if [ -e update/ss.db ]
then
	rm ./ss.db
	mv update/ss.db /mnt/usr
	chmod 777 ./ss.db
fi


if [ -e update/index.html ]
then
	rm ./htdocs/index.html
	mv update/index.html /mnt/usr/htdocs
	chmod 777 ./htdocs/index.html
fi

if [ -e update/scene.html ]
then
	rm ./htdocs/scene.html
	mv update/scene.html /mnt/usr/htdocs
	chmod 777 ./htdocs/scene.html
fi
./httpd_bin 1>/dev/null &
./serial_ctrl_bin 1>/dev/null &
exit 0
