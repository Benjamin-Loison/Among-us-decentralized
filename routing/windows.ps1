set interface 1 forwarding="enabled"

Get-Content routing | foreach-object {
	$ipdest = ($_ -split ' ')[0]
	$iprouter = ($_ -split ' ')[1]
	route add $ipdest mask 255.255.255.255 $iprouter
}

