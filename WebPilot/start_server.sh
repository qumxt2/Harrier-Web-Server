#!/bin/bash

uwsgi --master --vacuum --socket 127.0.0.1:9001 --uid graco --gid graco --processes 4 --threads 2 --enable-threads --chdir=/home/graco/pump/WebPilot --module=GracoPump.wsgi:application
