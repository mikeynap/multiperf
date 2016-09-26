package main

import (
	"flag"
	"fmt"
	"net"

	"github.com/mikeynap/multiperf/receiver"
	"github.com/mikeynap/multiperf/sender"
)

func main() {
	mode := flag.String("mode", "sender", "sender or listener")
	flag.Parse()
	if *mode == "sender" {
		s, e := sender.NewSender(net.ParseIP("231.0.0.1"), 9999, 2, 5, 1316)
		if e != nil {
			fmt.Println(e)
			return
		}
		s.Send(net.ParseIP("231.0.0.1"), 9999)
	} else {
		r, e := receiver.NewListener(net.ParseIP("231.0.0.1"), 9999, 2, 5, 1316)
		if e != nil {
			fmt.Println(e)
			return
		}
		r.Listen()

	}
}
