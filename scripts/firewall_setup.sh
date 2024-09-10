#!/bin/bash

/sbin/iptables -N MQTT-RESTRICT
/sbin/iptables -F MQTT-RESTRICT
/sbin/iptables -A MQTT-RESTRICT --src 216.3.13.0/24 -j ACCEPT
/sbin/iptables -A MQTT-RESTRICT --src 216.84.179.0/24 -j ACCEPT
/sbin/iptables -A MQTT-RESTRICT --src 207.212.62.0/24 -j ACCEPT
/sbin/iptables -A MQTT-RESTRICT --src 66.209.78.0/24 -j ACCEPT
/sbin/iptables -A MQTT-RESTRICT --src 217.163.63.0/24 -j ACCEPT
/sbin/iptables -A MQTT-RESTRICT --src 205.201.50.0/24 -j ACCEPT
/sbin/iptables -A MQTT-RESTRICT --src 205.201.51.0/24 -j ACCEPT
/sbin/iptables -A MQTT-RESTRICT -j DROP
/sbin/iptables -I INPUT -m tcp -p tcp --dport 8883 -j MQTT-RESTRICT
