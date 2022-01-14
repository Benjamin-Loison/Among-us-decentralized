#!/bin/bash

TMUX_SESSION=TP2

#            +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
#            | Definition des noms d'espace |
#            +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
function add_namespaces () {
	while [[ $# > 0 ]]; do
		sudo ip netns add $1
		shift
	done
}
add_namespaces sw0ns sw1ns r2 c1 c2



#            +~~~~~~~~~~~~~~~~~~~~+
#            | Ajout des switches |
#            +~~~~~~~~~~~~~~~~~~~~+
sudo ip netns exec sw0ns ip link add sw type bridge
sudo ip netns exec sw1ns ip link add sw type bridge



#            +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
#            | Ajout des câbles attachés à SW0 |
#            +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
sudo ip link add eth0 type veth peer r1_sw netns sw0ns
sudo ip netns exec sw0ns ip link set r1_sw master sw

sudo ip netns exec c1 ip link add eth0 type veth peer c1_sw netns sw0ns
sudo ip netns exec sw0ns ip link set c1_sw master sw

sudo ip netns exec r2 ip link add eth0 type veth peer r2_sw netns sw0ns
sudo ip netns exec sw0ns ip link set r2_sw master sw


#            +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
#            | Ajout des câbles attachés à SW1 |
#            +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
sudo ip netns exec r2 ip link add eth1 type veth peer r2_sw netns sw1ns
sudo ip netns exec sw1ns ip link set r2_sw master sw

sudo ip netns exec c2 ip link add eth0 type veth peer c2_sw netns sw1ns
sudo ip netns exec sw1ns ip link set c2_sw master sw



#                      +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
#                      | Visualisation dans le tmux TP2 |
#                      +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
function add_window_netns () {
	# NETNS: $2
	# WINDOW_ID: $1
	tmux split-pane -h -t $TMUX_SESSION:$1.0
	tmux split-pane -v -t $TMUX_SESSION:$1.0
	tmux split-pane -v -t $TMUX_SESSION:$1.2
	tmux split-pane -v -t $TMUX_SESSION:$1.3

	if [[ $2 ]]; then
		tmux send-keys -t $TMUX_SESSION:$1.0 "sudo ip netns exec $2 bash" C-m
		tmux send-keys -t $TMUX_SESSION:$1.1 "sudo ip netns exec $2 bash" C-m
		tmux send-keys -t $TMUX_SESSION:$1.2 "sudo ip netns exec $2 bash" C-m
		tmux send-keys -t $TMUX_SESSION:$1.3 "sudo ip netns exec $2 bash" C-m
		tmux send-keys -t $TMUX_SESSION:$1.4 "sudo ip netns exec $2 bash" C-m
	fi
	tmux send-keys -t $TMUX_SESSION:$1.0 "export COLORFGBG=\"7;0\"" C-m
	tmux send-keys -t $TMUX_SESSION:$1.1 "sudo -i" C-m
	tmux send-keys -t $TMUX_SESSION:$1.1 "export COLORFGBG=\"7;0\"" C-m
	tmux send-keys -t $TMUX_SESSION:$1.2 "export COLORFGBG=\"7;0\"" C-m
	tmux send-keys -t $TMUX_SESSION:$1.3 "export COLORFGBG=\"7;0\"" C-m
	tmux send-keys -t $TMUX_SESSION:$1.4 "export COLORFGBG=\"7;0\"" C-m

	tmux send-keys -t $TMUX_SESSION:$1.0 "watch sudo nft list ruleset" C-m
	tmux send-keys -t $TMUX_SESSION:$1.2 "watch -c ip -c a" C-m
	tmux send-keys -t $TMUX_SESSION:$1.3 "watch -c ip -c r" C-m
	tmux send-keys -t $TMUX_SESSION:$1.4 "watch -c ip -c n" C-m

	tmux send-keys -t $TMUX_SESSION:$1.1 "clear" C-m
}

tmux rename-window -t TP2:0 "R1"
add_window_netns 0

tmux new-window -t TP2; tmux rename-window -t TP2:1 "c1"
add_window_netns 1 c1

tmux new-window -t TP2; tmux rename-window -t TP2:2 "r2"
add_window_netns 2 r2

tmux new-window -t TP2; tmux rename-window -t TP2:3 "c2"
add_window_netns 3 c2

#tmux new-window -t TP2; tmux rename-window -t TP2:4 "sw0ns"
#add_window_netns 4 sw0ns

#tmux new-window -t TP2; tmux rename-window -t TP2:5 "sw1ns"
#add_window_netns 5 sw1ns

#            +~~~~~~~~~~~~~~~~~~~~~~~~+
#            | Ajout des addresses ip |
#            +~~~~~~~~~~~~~~~~~~~~~~~~+
sudo ip address add 192.168.1.1/24 dev eth0
sudo ip netns exec c1 ip address add 192.168.1.2/24 dev eth0
sudo ip netns exec r2 ip address add 192.168.1.3/24 dev eth0
sudo ip netns exec r2 ip address add 192.168.2.1/24 dev eth1
sudo ip netns exec c2 ip address add 192.168.2.2/24 dev eth0



#            +~~~~~~~~~~~~~~~~~~~~~~~~~+
#            | Allumage des interfaces |
#            +~~~~~~~~~~~~~~~~~~~~~~~~~+
sudo ip link set up dev eth0

sudo ip netns exec sw0ns ip link set up dev r1_sw
sudo ip netns exec sw0ns ip link set up dev c1_sw
sudo ip netns exec sw0ns ip link set up dev r2_sw
sudo ip netns exec sw1ns ip link set up dev r2_sw
sudo ip netns exec sw1ns ip link set up dev c2_sw

sudo ip netns exec c1 ip link set up dev eth0
sudo ip netns exec r2 ip link set up dev eth0
sudo ip netns exec r2 ip link set up dev eth1
sudo ip netns exec c2 ip link set up dev eth0

sudo ip netns exec sw0ns ip link set up dev sw
sudo ip netns exec sw1ns ip link set up dev sw



#            +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
#            | Gestion des règles NFTables |
#            +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
function basic_filter () {
	sudo ip netns exec $1 nft flush ruleset
	sudo ip netns exec $1 nft add table inet filter
	sudo ip netns exec $1 nft add chain inet filter input '{ type filter hook input priority filter; policy drop; }'
	sudo ip netns exec $1 nft add rule inet filter input 'iifname "lo" accept'
	sudo ip netns exec $1 nft add rule inet filter input 'ct state {established, related} accept'
	sudo ip netns exec $1 nft add rule inet filter input 'ip protocol icmp counter accept'
}
function set_masquerade () {
	sudo ip netns exec $1 nft add table ip nat
	sudo ip netns exec $1 nft add chain ip nat postrouting '{ type nat hook postrouting priority srcnat; }'
	sudo ip netns exec $1 nft add rule ip nat postrouting "oifname $2 masquerade"
}

#basic_filter c1
#basic_filter r2
#basic_filter c2
set_masquerade r2 eth0

sudo sysctl net.ipv4.ip_forward=1
sudo ip netns exec r2 sysctl net.ipv4.ip_forward=1
sudo ip netns exec c1 ip route add default via 192.168.1.1
sudo ip netns exec r2 ip route add default via 192.168.1.1
sudo ip netns exec c2 ip route add default via 192.168.2.1

sudo ip route add 192.168.2.0/24 via 192.168.1.3



#                      +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
#                      | Test de connectivité + ping c1 → c2 |
#                      +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
#tmux send-keys -t $TMUX_SESSION:0.1 "mtr google.com" C-m
#tmux send-keys -t $TMUX_SESSION:1.1 "mtr 192.168.2.2" C-m
#tmux send-keys -t $TMUX_SESSION:2.1 "mtr google.com" C-m
#tmux send-keys -t $TMUX_SESSION:3.1 "mtr google.com" C-m



#                      +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
#                      | Lancement d'Among Us (c1, R2, c2) |
#                      +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
tmux send-keys -t $TMUX_SESSION:1.1 "su dsac" C-m
sleep 1; tmux send-keys -t $TMUX_SESSION:1.1 "cd SoftwareEngineering/Among-us-decentralized/" C-m
sleep 1; tmux send-keys -t $TMUX_SESSION:1.1 "./AmongUsDecentralized" C-m

tmux send-keys -t $TMUX_SESSION:2.1 "su dsac" C-m
sleep 1; tmux send-keys -t $TMUX_SESSION:2.1 "cd SoftwareEngineering/Among-us-decentralized/" C-m
sleep 1; tmux send-keys -t $TMUX_SESSION:2.1 "./AmongUsDecentralized" C-m

tmux send-keys -t $TMUX_SESSION:3.1 "su dsac" C-m
sleep 1; tmux send-keys -t $TMUX_SESSION:3.1 "cd SoftwareEngineering/Among-us-decentralized/" C-m
sleep 1; tmux send-keys -t $TMUX_SESSION:3.1 "./AmongUsDecentralized" C-m
