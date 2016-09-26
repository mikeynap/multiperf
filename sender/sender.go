package sender

import (
	"fmt"
	"net"
	"time"

	"github.com/mikeynap/multiperf/socket"
)

type Sender struct {
	Socket  *socket.MSocket
	Seconds int
	PktSize int
}

func NewSender(ip net.IP, port int, inc int, seconds int, pktSize int) (*Sender, error) {
	inter, err := socket.GetMulticastInterface()
	if err != nil {
		return nil, err
	}
	s, err := socket.NewMulticastSender(ip, port, inter)
	if err != nil {
		return nil, err
	}
	s.JoinGroup(ip)
	return &Sender{s, seconds, pktSize}, nil
}

func (s *Sender) Send(ip net.IP, port int) error {
	data := []byte("HEY!!!")
	dst := &net.UDPAddr{IP: ip, Port: port}
	for {
		if rt, err := s.Socket.WriteTo(data, nil, dst); err != nil {
			fmt.Println(err)
			continue
		} else {
			fmt.Println(rt)
		}
		fmt.Printf("Wrote to %v %v\n", dst, ip)
		time.Sleep(1 * time.Second)
	}
}
