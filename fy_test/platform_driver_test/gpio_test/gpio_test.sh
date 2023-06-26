#!/bin/sh
Num=`expr $1 \* 8 + $2`;
Meter="/sys/class/gpio/gpio0/";
Target="/sys/class/gpio/gpio$Num/";

echo "GPIO0_0 is multimeter!"
if [ ! -d "$Meter" ]; then
	echo 0 > "/sys/class/gpio/export";
fi
echo in > "/sys/class/gpio/gpio0/direction";

echo "Test GPIO$1_$2($Num)"
if [ ! -d "$Target" ]; then
	echo "$Num" > "/sys/class/gpio/export";
fi
echo out > "/sys/class/gpio/gpio$Num/direction";
echo 1 > "/sys/class/gpio/gpio$Num/value";
Vvalue=$(cat /sys/class/gpio/gpio0/value);
if [ $Vvalue -eq 1 ]; then
	echo "output high level OK"
else
	echo "output high level ERROR"
fi
echo 0 > "/sys/class/gpio/gpio$Num/value";
Vvalue=$(cat /sys/class/gpio/gpio0/value)
if [ $Vvalue -eq 0 ]; then
	echo "output low level  OK"
else
	echo "output low level ERROR"
fi
echo in > "/sys/class/gpio/gpio$Num/direction";
echo out > "/sys/class/gpio/gpio0/direction";
echo 1 > "/sys/class/gpio/gpio0/value";
Vvalue=$(cat /sys/class/gpio/gpio$Num/value)
if [ $Vvalue -eq 1 ]; then
	echo "input high level  OK"
else
	echo "input high level ERROR"
fi
echo 0 > "/sys/class/gpio/gpio0/value";
Vvalue=$(cat /sys/class/gpio/gpio$Num/value)
if [ $Vvalue -eq 0 ]; then
	echo "input low level   OK"
else
	echo "input low level ERROR"
fi
