# perf-abr
Tool to profile multiple multicast streams. 

# Usage

```
1)  Default: Test 1 address (231.1.1.1), over one port (13000)
    trasmitting at 10mbps for 10 seconds:
	
	Receiver: perfabr
	Sender: perfabr -S
   
   
2)  Test 10 simulatenous multicast addresses (231.2.2.1 -> 231.2.2.10),
    over 5 ports each (231.2.2.1:13000-4 -> 231.2.2.10:13049)
    with each of the 50 connections transmitting at 2mbps, for 8 seconds:

	Receiver: perfabr -R -a 10 -s 5 -m 231.2.2.1 
	Sender: perfabr -S -a 10 -s 5 -m 231.2.2.1 -b 2.0 -t 8 

3)  Multitest: Do nine tests up to 80 addresses,
    incrementing by 10 each "round" (1 addr, 10 addr... 80 addr)
	Over 5 streams, starting at port 13000 (default)
	each transmitting at 10mbps for 10 seconds each (default). 

	Receiver: perfabr -R -a 80 -s 5 
	Sender: perfabr -S -a 80 -s 5

This will run 9 tests varying over the number of addresses in increments of 10
and give a CSV report at the end.

```
