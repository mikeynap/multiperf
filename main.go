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
		addr := net.ParseIP("231.0.0.1")
		for i := 0; i < 99; i++ {
			if s, e := sender.NewSender(addr, 9999, 1, 5, 1316); e == nil {
				go s.Send(addr, 9999)
				addr = socket.Inc(addr)
			} else {
				fmt.Println(e)
			}
		}
		s, _ := sender.NewSender(addr, 9999, 1, 5, 1316)
		s.Send(addr, 9999)
	} else {
		r, e := receiver.NewListener(net.ParseIP("231.0.0.1"), 9999, 100, 5, 1316)
		if e != nil {
			fmt.Println(e)
			return
		}
		r.Listen()

	}
}
