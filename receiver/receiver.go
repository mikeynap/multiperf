package receiver

import (
	"fmt"
	"net"

	"github.com/mikeynap/multiperf/socket"
	"github.com/mikeynap/multiperf/stats"
)

type Listener struct {
	Socket  *socket.MSocket
	Seconds int
	PktSize int

	Stats *stats.Stats
}

func NewListener(ip net.IP, port int, inc int, seconds int, pktSize int) (*Listener, error) {
	inter, err := socket.GetMulticastInterface()
	if err != nil {
		return nil, err
	}
	s, err := socket.NewMulticastListener(port, inter)
	if err != nil {
		return nil, err
	}
	s.JoinGroups(ip, inc)
	return &Listener{s, seconds, pktSize, stats.NewStats()}, nil
}

func (l *Listener) Listen() {
	b := make([]byte, 1500)
	for {
		n, cm, src, err := l.Socket.ReadFrom(b)
		if err != nil {
			fmt.Printf("Err Reading: %v", err)
		}
		fmt.Printf("%v,%v,%v,%v,%v\n", string(b), n, cm, src, err)

	}
}
