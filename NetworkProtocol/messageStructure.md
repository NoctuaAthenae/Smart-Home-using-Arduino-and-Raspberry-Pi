# Network Protocol

This Network Protocol is used for the communication between the central hub and the endpoints.

## IDs

The hub and each endpoint has it's each unique identifier, where the hub has the ID 1. The ID 0 is used for broadcast messages. Also groups can be created and assigned an ID, broadcast to the group ID are received by every endpoint in the group. It is stored in one byte, limiting the number of possible endpoints/groups to 254. Each member of the network only pays attention to message of it's parents or children and ignores all other messages, that are received from another member in range.

## Messages

Each message consists of the following fields:
- 1 Byte: Version
- 1 Byte: Receiver
- 1 Byte: Last Device, that handled the message
- 1 Byte: Next Hop
- 6 Bit: Message Type (max 64 message types)
<a name="GF"></a>
- 1 Bit: Group flag (GF)
<a name="GAF"></a>
- 1 Bit: Group ascending flag (GAF)
- Variable: Message Type specific fields
- 4 Byte: Timestamp
- 1 Byte: Checksum
- Total of 10 Bytes for the standard meta data

### Commands (0)

Commands are used for exchanging data between the network members. It can consist of a single command or a command with parameters. Commands are stored in one byte, limiting the number of possible commands to 255,
while the parameters can have variable data lengths. It also sends a timestamp for identifying messages. The message is repeated if there is no acknowledge message after a timeout.\
Additional fields:
- 1 Byte: Package number
- 1 Byte: Total packages
- 1 Byte: Command (only for first package)
- Variabel: Parameters

The total size of meta data for a command package is 12 Bytes, which leaves 20 Bytes per Package as the maximum for a nRF24L01 is 32 Bytes. Since there is 1 Byte for package numbers, there can be a maximum of 256 packages, 
which means the parameters can have 5 120 - 1 (Command) = 5 119 bytes at max.

```
void send(byte destination, byte command, byte[] payload)
void sendToGroup(byte destination, byte command, byte[] payload)
```


### Acknowledge (1)

Confirms the hop, that the message has reached the next hop by responding with the timestamp. This is done hop by hop. The timestamp field here is the timestamp of the incoming message.\
Only sent by the protocol.

### Register (2)

Registers an endpoint to the network at startup of the endpoint. See Registration for information.

```
void connect(byte parent)
```

### Accept/Reject (3)

Accepts or rejects the endpoint, that is trying to register.\
Additional fields:
- 1 Byte: Accept (1)/Reject (0)
- 1 Byte: Given ID
Only sent by the protocol.

### Ping (4)

Pings a device.\
Additional fields:
- 1 Byte: Sender
- 1 Byte: Ping ID

```
void ping(byte destination)
```

### Ping Response (5)

Response to a ping.\
Additional fields:
- 1 Byte: Ping ID

Only sent by the protocol.

### Route Creation (6)

Collects all hops to the hub for the new endpoint and writes the new endpoint into the routing list of each hop.\
Additional fields:
- 1 Byte: ID of the new endpoint
- Variabel: Each Hop to the hub

Only sent by the protocol.

### Add To/Remove From Group (7)

Adds or removes an endpoint to/from a group for broadcasting to a group of specific devices.\
Additional fields:
- 1 Byte: Add (1)/Remove (0) 
- 1 Byte: ID of the group

Can only be done at the interface of the hub, so the message is only sent by the protocol.

## Registration

If the endpoint has been registered already, it sends it's ID to the hub or another endpoint, which will be the new endpoint's parent. The parent sends a Route Creation message to the hub. Each hop on the way adds the new endpoint to it's routing list with all previous hops flagged invalid and adds itself to the hop list of the route creation message. The hub pings the ID of the new endpoint to check if the endpoint tries to hijack an ID.
If the ping does not get a response, the registration is accepted and an accept message is sent. Each hop on the way validates the entry in the routing list.\
If the endpoint is registered the first time, it sends a 0 as ID. The parent sends a Route Creation message to the hub. Each hop on the way adds the new endpoint to it's routing list with all previous hops flagged invalid and with the 0 as ID and adds itself to the hop list of the route creation message. The hub then assigns the next free ID in an accept message if there are is a free ID. Each hop on the way validates the entry and saves the correct ID in the routing list.\
![Diagram of an registration process example](Registration.png)

## Routing List

Each Node has a list with all children and via what children they are reachable. It can have invalid entries flagged valid if an endpoint disconnects, since there are no disconnect messages.\
![Diagram of an routing list example](TreeExample.png)

## Groups

Endpoints can be parts of groups to benefit from group broadcasts. A group broadcast is sent with the [GF](#GF) and the [GAF](#GAF) flag set. While the GAF flag is set, all endpoints send the message to their parent. When the messages reaches the hub the GAF flag is reset and sent to all children. Each endpoint picks up the message and sends it to all own children, while the Next Hop field is set to the own ID.
