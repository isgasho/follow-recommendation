#!/bin/bash
filename='/var/lib/distsn/instance-speed/instance-speed.json'
if [ -f $filename ]
then
	echo 'Access-Control-Allow-Origin: *'
	echo 'Content-type: application/json'
	echo ''
	cat $filename
else
	echo 'Status: 404 Not found'
fi

