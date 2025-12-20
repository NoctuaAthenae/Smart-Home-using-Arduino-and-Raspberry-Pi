# Network Protocol

This Network Protocol is used for the communication between the central hub and the endpoints.

## IDs

The hub and each endpoint has it's each unique identifier. There are the following special IDs:
- 0: Hub
- 255: Discover
Also groups can be created and assigned an ID, broadcast to the group ID are received by every endpoint in the group. It is stored in one byte, limiting the number of possible endpoints to 254. Groups have an own address room, where group and endpoint IDs are distinguished by the Group flag. Each member of the network only pays attention to message of it's parents or children and ignores all other messages, that are accidentally received from another member in range. A message has an transmission identifier, to uniquely identify a message between two hops. If the messages is forwarded further, it gets a new ID. Since the transmission ID has only one byte the number of messages, that can be sent without being acknowledged is limited to 256.

## Messages

Each message consists of the following fields:
- [0] 1 Byte: Version
- [1] 1 Byte: Receiver
- [2] 6 Bit: Message Type (max 64 message types)
<a name="GF"></a>
- 1 Bit: Group flag (GF)
<a name="GAF"></a>
- 1 Bit: Group ascending flag (GAF)
- Variable: Message Type specific fields
- Total of 3 Bytes for the standard meta data

### Commands (0)

Commands are used for exchanging data between the network members. It can consist of a single command or a command with parameters. Commands are stored in one byte, limiting the number of possible commands to 255, while the parameters can have variable data lengths. Each command message package contains its origin and message ID, which are used to identify the message and reconstruct it.\
Additional fields:
- [3] 1 Byte: Package number
- [4] 1 Byte: Origin
- [5] 2 Byte: Message ID
- [7] 1 Byte: Command (only for first package)
- [8] 1 Byte: Total packages (only for first package)
- Variabel: Parameters

The total size of meta data for a command package is 7 Bytes, which leaves 25 bytes per Package as the maximum for a nRF24L01 is 32 bytes. Since there is 1 byte for package numbers, there can be a maximum of 256 packages,
which means the parameters can have 6 400 - 2 (Command and total packages) = 6 398 bytes at max.

```
void send(byte destination, byte command, byte[] payload)
void sendToGroup(byte destination, byte command, byte[] payload)
```

### Registration (1)

Additional fields:
- [3] 1 Byte: Registration Message Type 
- [4] 1 Byte: ID
- [5] 4 Byte: Temporary ID


Discover (0)

A new device sends a discover message to find the best parent over the discover channel (RF24: Sends with discover ID as sender ID). Sends own ID in the ID field if it does have one.\
Additional fields:
- [9] 1 Byte: Hierarchy Level of the discovered device (255 indicates, that this is a discovery request).

Register (1)

Registers an endpoint to the network at startup of the endpoint. See Registration for information. Sends own ID in the ID field if it does have one.\

Route Creation (2)

Tells the parent of a device d, that there is a new device, that can be reached via d. Sends the ID of the new device in the ID field if it does have one.\

Accept/Reject (3)

Accepts or rejects the endpoint, that is trying to register. Sends the ID of the new device in the receiver field and the ID field (this makes the code easier) if it does have one.\
Additional fields:
- [9] 1 Byte: Accept (1)/Reject (0)


### Ping (2)

Pings a device.\
Additional fields:
- [3] 1 Byte: Sender
- [4] 1 Byte: Ping ID
- [5] 1 Byte: Is Response
- [6] 4 Byte: Timestamp

```
void ping(byte destination)
```

Represents both ping request and response. The response is marked with the IsResponse field

### Add To/Remove From Group (3)

Adds or removes an endpoint to/from a group for broadcasting to a group of specific devices.\
Additional fields:
- [3] 1 Byte: Add (1)/Remove (0) 
- [4] 1 Byte: ID of the group

Can only be done at the interface of the hub, so the message is only sent by the protocol.

### Error (4)

Signals an error, that occurred during another message.\
Additional fields:
- [3] 1 Byte: Error code
- [4] Variable: First 28 byte of original message

This error message is reserved for use by the protocol. For application errors use the command message. All errors are logged at the hop.

### Disconnected/Reconnect (5)

If the hub pings a device, which does not answer, it sends a disconnect message down its routing path.\
If a device finds a better parent, it sends a reconnect message to its new parent.\

- [3] 1 Byte: Reconnect/Disconnect

Only sent by the protocol.

## Registration

A new endpoint chooses its parent itself. Since the nRF listening is limited to six devices and it has to listen to its parent, the number of children for each device is limited to five. A new endpoint sends a discover message to all possible IDs. Each device, that receives this message, responds with its ID and distance to the root if it has a slot available. The new endpoint chooses the device with the lowest distance and performs a connection quality check by sending 100 pings and measuring the RTT and the response rate. If the quality is less than a certain threshold, the device with the next highest distance is selected and tested. This is done until a device with a good connection is found.

If the endpoint has been registered already or is assigned a static ID, it sends it's ID to the hub or another endpoint, which will be the new endpoint's parent. The parent sends a Route Creation message to the hub. Each hop on the way adds the new endpoint to it's routing list with all previous hops flagged invalid and adds itself to the hop list of the route creation message. The hub pings the ID of the new endpoint to check if the endpoint tries to hijack an ID.
If the ping does not get a response, the registration is accepted and an accept message is sent. Each hop on the way validates the entry in the routing list.\
If the endpoint is registered the first time, it sends a 0 as ID and sets its temporaray ID to the current timestamp. This allows multiple registrations at the same time with a low collision probability. The parent sends a Route Creation message to the hub. Each hop on the way adds the new endpoint to it's routing list with all previous hops flagged invalid and with the temporaray ID and adds itself to the hop list of the route creation message. The hub then assigns the next free ID in an accept message if there is a free ID. Each hop on the way validates the entry and saves the correct ID in the routing list.\
<img src="Registration.png" alt="Diagram of an registration process example" width="500"/>

## Routing List

Each Node has a list with all children and via which children they are reachable. It can have invalid entries flagged valid if an endpoint disconnects, since there are no disconnect messages.\
<img src="TreeExample.png" alt="Diagram of an routing list example" width="150"/>

## Groups

Endpoints can be parts of groups to benefit from group broadcasts. A group broadcast is sent with the [GF](#GF) and the [GAF](#GAF) flag set. While the GAF flag is set, all endpoints send the message to their parent. When the messages reaches the hub the GAF flag is reset and sent to all children. Each endpoint picks up the message and sends it to all own children.

## Disconnects

The hub pings regularly all devices. It pings one device every pingTime / numberDevices (milli)seconds, so each device is pinged every pingTime (milli)seconds. If a device does not answer, the hub sends a disconnect message down its routing path.

## Reconnect

Every now and then each device sends a discover message to all IDs. It benchmarks the connection to all devices, that answer, and picks the best connection. The device sends a reconnect message to its parent. Each device that gets the reconnect message, adds the path to its routing table. If the reconnected device was already in the routing table, it sends a disconnect message down the routing path, otherwise it sends the reconnect message to its parent.
