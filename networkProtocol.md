# Network Protocol

This Network Protocol is used for the communication between the central hub and the endpoints.

## IDs

The hub and each endpoint has it's each unique identifier, where the hub has the ID 0. It is stored in one byte, limiting the number of possible endpoints to 255.

## Messages

Each message consists of the following fields:
- 1 Byte: Receiver
- 1 Byte: Sender
- 1 Byte: Message Type
- Variable: Message Type specific
- 1 Byte: Checksum
- Total of 4 Bytes for the standard meta data

### Commands (0)

Commands are used for exchanging data between the network members. It can consist of a single command or a command with parameters. Commands are stored in one byte, limiting the number of possible commands to 255,
while the parameters can have variable data lengths. It also sends a timestamp for identifying messages. The message is repeated if there is no acknowledge message after a timeout.\
Additional fields:
- 1 Byte: Package number
- 1 Byte: Total packages
- 1 Byte: Command (only for first package)
- Variabel: Parameters
- 4 Byte: Timestamp

The total size of meta data for a command package is 10 Bytes, which leaves 22 Bytes per Package as the maximum for a nRF24L01 is 32 Bytes. Since there is 1 Byte for package numbers, there can be a maximum of 256 packages, 
which means the parameters can have 5 632 - 1 (Command) = 5 631 bytes at max.

### Acknowledge (1)

Confirms the sender, that the message has been received by the receiver by responding with the timestamp.\
Additional fields:
- 4 Byte: Timestamp of the incoming message

### Register (2)

Registers an endpoint to the network at startup of the endpoint. If the endpoint has been registered already, it sends it's ID. The hub pings the ID to check if the endpoint tries to hijack an ID.
If the ping does not get a response, the registration is accepted and an accept message is sent. If the endpoint is registered the first time, it sends a 0 as ID. The hub then assigns the next free ID in an accept message
if there are no more than 255 endpoints registered yet. The message is repeated if there is no accept or reject message after a timeout.

### Accept/Reject (3/4)

Accepts or rejects the endpoint, that is trying to register.

## Commands
