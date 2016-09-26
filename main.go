package main

import (
	"flag"
	"fmt"
	"net"

	"github.com/mikeynap/multiperf/receiver"
	"github.com/mikeynap/multiperf/sender"
	"github.com/mikeynap/multiperf/socket"
)

func main() {
	mode := flag.String("mode", "sender", "sender or listener")
	flag.Parse()
	if *mode == "sender" {
		s, e := sender.NewSender(net.ParseIP("231.0.0.1"), 9999, 2, 5, 1316)
		s2, e2 := sender.NewSender(net.ParseIP("231.0.0.2"), 9999, 2, 5, 1316)
		if e != nil {
			fmt.Println(e)
			return
		}
		if e2 != nil {
			fmt.Println(e2)
			return
		}
		addr := net.ParseIP("231.0.0.1")
		for i := 0; i < 99; i++ {
			go s.Send(addr, 9999)
			addr = socket.Inc(addr)
		}
		s2.Send(addr, 9999)
	} else {
		r, e := receiver.NewListener(net.ParseIP("231.0.0.1"), 9999, 100, 5, 1316)
		if e != nil {
			fmt.Println(e)
			return
		}
		r.Listen()

	}
}
