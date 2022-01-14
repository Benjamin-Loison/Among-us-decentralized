#!/bin/bash

sysctl net.inet.ip.forwarding=1

while read -r ip_dest ip_router; do
	route -n add -net "$ip_dest" "$ip_router"
done<routing

