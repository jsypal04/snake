# Protocol
---

This document defines the protocol between the client and the server. It is built on top of UDP.

# Packet Format
---

All packets must be in the following format:

Authentication token

Player coordinate data (json format)

Apple collision detected (Y/N)

Other player collision detected (Y/N)

# Basics
---

The basic server flow is as follows:
1. Listen packets
2. Process packet
3. Broadcast game state
4. Update connection ttls

# Packet Processing
---

The server processes the packet in the following steps:
1. Check for a valid authentication token (this ensures that the packet comes from a real client)
2. If the IP address is not in the connections table, add it
3. Update the game state with the player's coordinate data
4. Reset that connection's ttl to the connection timeout time.

# Connections
---

This protocol implements a very lightweight connection scheme. Whenever a packet arrives from an IP address not in the connections
table, the server adds it to the connections table. When the connection is added, a time to live (ttl) is set for it (right now the
initial value is 33). Every time the server processes a packet, it decrements all ttls by one. When the server receives a packet from
an IP, it resets the ttl to the original timeout value. Whenever a connection's ttl reaches 0, it is disconnected (i.e., removed from
the connections table). This allows the server to connect and disconnect clients dynamically without the client needing to request to
connect or tell the server that it is disconnecting.

# Responses
---

The server returns one of the following response codes:
1. 200 - Indicates success
2. 401 - Authentication failed
3. 402 - Issue decoding player coordinate data

