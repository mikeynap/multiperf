package socket

import (
	"fmt"
	"net"

	"golang.org/x/net/ipv4"
)

type MSocket struct {
	inter *net.Interface
	conn  *ipv4.PacketConn
}

func GetMulticastInterface() *net.Interface {
	ifaces, _ := net.Interfaces()
	for _, iface := range ifaces {
		f := iface.Flags
		if f&net.FlagUp == net.FlagUp && f&net.FlagMulticast == net.FlagMulticast && f&net.FlagLoopback == 0 {
			return &iface
		}
	}
	return nil
}

func NewMulticastListener(port int, inter *net.Interface) (*MSocket, error) {
	if inter == nil {
		inter = GetMulticastInterface()
	}

	c, err := net.ListenPacket("udp4", "0.0.0.0:"+string(port))
	if err != nil {
		return nil, err
	}
	p := ipv4.NewPacketConn(c)
	if err := p.SetControlMessage(ipv4.FlagDst, true); err != nil {
		fmt.Println(err)
		return nil, err
	}

	return &MSocket{inter, p}, nil
}

func NewMulticastSender(group net.IP, port int, inter *net.Interface) (*MSocket, error) {
	if inter == nil {
		inter = GetMulticastInterface()
	}

	c, err := net.ListenPacket("udp4", "0.0.0.0:"+string(port))
	if err != nil {
		return nil, err
	}
	p := ipv4.NewPacketConn(c)
	p.SetTOS(0x0)
	p.SetTTL(16)
	p.SetMulticastTTL(16)

	if _, err := p.WriteTo(data, nil, dst); err != nil {
		// error handling
	}

	return &MSocket{inter, p}, nil

}

func inc(ip net.IP) net.IP {
	for j := len(ip) - 1; j >= 0; j-- {
		ip[j]++
		if ip[j] > 0 {
			break
		}
	}
	return ip
}

func (ms *MSocket) JoinGroups(group net.IP, n int) {
	for i := 0; i < n; i++ {
		ms.JoinGroup(group)
		group = inc(group)
	}
}

func (ms *MSocket) JoinGroup(group net.IP) error {
	if err := ms.conn.JoinGroup(ms.inter, &net.UDPAddr{IP: group}); err != nil {
		return err
	}
	return nil
}
