package receiver

import (
	"fmt"
	"net"

	"github.com/mikeynap/multiperf/socket"
	"github.com/mikeynap/multiperf/stats"
)

type Listener struct {
	Socket  socket.MSocket
	Seconds int
	PktSize int

	Stats *stats.Stats
}

func NewListener(ip net.IP, port int, inc int, seconds int, pktSize int) (*Listener, error) {
	inter, err := socket.GetMulticastInterface()
	if err != nil {
		return nil, err
	}
	s := socket.NewMulticastListener(port, inter)
	s.JoinGroup(ip, inc)
	return &Listener{s, seconds, pktSize, stats}
}

func (l *Listener) Listen() {
	b := make([]byte, 1500)
	for {
		n, cm, src, err := l.Socket.ReadFrom(b)
		fmt.Printf("%v,%v,%v,%v,%v", string(b), n, cm, src, err)
		if err != nil {
			// error handling
		}
	}
}
