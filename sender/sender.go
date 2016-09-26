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
	s.JoinGroups(ip, inc)
	return &Sender{s, seconds, pktSize}, nil
}

func (s *Sender) Send(ip net.IP, port int) error {
	data := []byte("HEY!!!")
	dst := &net.UDPAddr{IP: ip, Port: port}
	for {
		if _, err := s.Socket.WriteTo(data, nil, dst); err != nil {
			fmt.Println(err)
			return err
		}
		fmt.Printf("Wrote to %v\n", dst)
		time.Sleep(1 * time.Second)
	}
}
