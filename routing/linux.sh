#!/bin/bash

sysctl net.ipv4.ip_forward=1

while read -r ip_dest ip_router; do
	ip route add "$ip_dest" via "$ip_router"
done<routing

