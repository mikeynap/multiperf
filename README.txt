Sender:
    - Sends Traffic with specified bitrate/pkt size to receivers
    - Sends over m mcast addresses, and p ports
    - Sends Sender Info in the data stream packets.

Receiver:
    - Gets Traffic over m mcast addresses and p ports
    - Compiles Jitter Measurements (histogram?)
    - Sends back to sender the results. (Over Mcast?)

Stats:
    - Histogram of Jitter
    - loss
    - bit rate
    - Aggr Bitrate.
    - (bytes, time, recv/lost)


Flow:
- Sender Multicasts Out to the N receivers "Hey, starting tests! Here's the info"
    - Includes type of test, parameters etc.

Test case: 1 address, 1 port, increment 1, 10 mbps, 5 seconds, 1316 pktsize:
    - does 1 send to 1 address/port for 5 seconds

Test case: 50 address, 5 port, increment 5, 10 mbps, 5 seconds, 1316 pktsize:
    - 1. send 1 addr, 5 port
    - 2. send 5 addr, 5 port
    - 3. send 10 addr, 5 port ...
    - 10. send 50 addr, 5 port

    -- Listens on ctrl mcast port for results.

Receiver:
    - Opens (to 1 address, 5 port) 5 listeners, each runs for 5 seconds
        - Merge Histogram/compute
        - Store results as test 1/10.
    - Starts 25 listeners (5 addr, 5 ports)...
    ...
    - Prints results.
    - Sends results back on ctrl address.
