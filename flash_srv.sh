#!/bin/sh

IMAGE_PATHNAME=$1
IMAGE_NAME=`basename $1`

echo "Image name: " ${IMAGE_NAME}
ssh rock@oyrocksrv "./pwr_off.sh && rm -f /home/rock/${IMAGE_NAME}" 

scp $IMAGE_PATHNAME rock@oyrocksrv:/home/rock/${IMAGE_NAME} 

ssh rock@oyrocksrv "em100 --stop --set MX25L51245G -d /home/rock/${IMAGE_NAME} -v --start && ./pwr_on.sh"
