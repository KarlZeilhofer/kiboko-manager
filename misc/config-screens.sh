#/bin/bash

# Lenovo Zeitnehmung
if [[ $HOSTNAME == ZEITNEHMUNG* ]]
then
	xrandr --output LVDS1 --auto --primary
	xrandr --output HDMI1 --auto --right-of LVDS1
	sleep 5

# Nexoc Fritz	
elif [[ $HOSTNAME == nexoc ]]
then
	xrandr --output LVDS-0 --auto --primary
	xrandr --output HDMI-0 --auto --right-of LVDS-0
	sleep 5
fi
