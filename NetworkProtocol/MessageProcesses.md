# Send and Receive

Here the internal send and receive methods are presented. These are the same for all message types. The distinction happens in the `processMessage()` method.

## Sending

All messages are added to a send queue. If the message has not been acknowledged after a timeout, resend it.

### Send

```
send(byte destination, byte messageTypeAndGroupFlags, byte[] payload)
  create message object
  set version, last hop and receiver
  set message type and group flags
  set payload
  set timestamp
  if group flag set:
    if this is hub:
      reset groupAscending flag
    if groupAscending set:
      set next hop to parent
    else:
      set next hop to self
  else:
    if destination in routingList:
      set next hop according to routing list
    else:
      set next hop to parent
  set checksum
  add to send queue
  send
```
Payload is set according to the message type.

## Receiving

All received messages are added to a receive queue, so if a acknowledgement gets lost and an already processed message is resent, it is ignored, but acknowledged.

### Receive

```
receive(Message message)
  if message.lastHop != parent and message.lastHop not in children:
    return
  if group flag set:
    if groupAscending not set:
      if this is in message.destination group:
        processMessage(message)
    send(message)
  else if message.nextHop == this:
    if isAck:
      remove message from send queue
      return
    sendAck(message.lastHop)
    if in receive queue:
      return
    add to receive queue
    if message.destination == this:
      processMessage(message)
    else:
      send(message)
```

# Message Types

This section describes the internal processes for the different message types. Message types, that belong to the same feature are described together in the same subsection.

## Register

The registration registers the device at the parent, which creates the route from the hub to the new device.

### Registration at new device

The `register` method starts the registration at the parent with the given ID.

```
register(byte parent)
  id = tryGetIdFromMemory()
  if id == null:
    id = 0
  this.id = id
  send(parent, getTypeByte(Register, NoGroup), [])
```

When a response is received, the registration is finished with the following method.

```
finishRegistration(Message message):
  if message.isReject:
    return
  if this.id == 0:
    this.id = message.givenId
  this.registered = true
```
