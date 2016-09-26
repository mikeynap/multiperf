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
		adders := make([]net.IP, 100, 100)
		for i := 0; i < 100; i++ {
			fmt.Println(fmt.Sprintf("231.0.0.%d", i+1))
			fmt.Println(net.ParseIP(fmt.Sprintf("231.0.0.%d", i+1)))
			adders[i] = net.ParseIP(fmt.Sprintf("231.0.0.%d", i+1))
		}
		for i := 0; i < 99; i++ {
			if s, e := sender.NewSender(adders[i], 9999, 1, 5, 1316); e == nil {
				fmt.Printf("%d: %v\n", i, adders[i])
				go s.Send(adders[i], 9999)
				fmt.Printf("%d: %v\n", i, adders[i])
				fmt.Printf("%d: %v\n", i, adders[i])
			} else {
				fmt.Println("Uh errorz")
				fmt.Println(e)
			}
		}
		s, _ := sender.NewSender(adders[99], 9999, 1, 5, 1316)
		s.Send(adders[99], 9999)
	} else {
		r, e := receiver.NewListener(net.ParseIP("231.0.0.1"), 9999, 100, 5, 1316)
		if e != nil {
			fmt.Println(e)
			return
		}
		r.Listen()

	}
}
