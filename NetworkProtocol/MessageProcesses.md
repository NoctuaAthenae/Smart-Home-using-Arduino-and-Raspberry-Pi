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

### Processing the message

The message is handled dependend on the message type. Command can be split into different packages to transmit larger data, so it has to be put back together at the receiver.

```
processMessage(Message message)
  byte messageType = message.getType()
  switch messageType:
    case command:
      CommandMessage commandMessage;
      if commandMessageBuilder.newCommandMessage(message, &commandMessage):
        processCommand(commandMessage)
    ...
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
  this.parent = parent
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

### Registration at parent

The parent initiates the route creation for the new device.

```
registrationMessage(Message message):
  this.routingTable.create(message.lastHop, message.lastHop, invalid)
  send(this.parent, getTypeByte(RouteCreation, NoGroup, [message.lastHop])
```

### Each hop on the way

Each hop adds the new device to its routing table.

```
registrationUpwards(Message message):
  this.routingTable.create(message.newDevice, message.lastHop, invalid)
  send(this.parent, getTypeByte(RouteCreation, NoGroup), message.payload.append(this.id))

registrationDownwards(Message message):
  send(message.givenId, getTypeByte(AcceptReject, NoGroup), [message.isAccept, message.givenId])
  if message.isReject:
    this.routingTable.remove(message.givenId)
    return
  if this.routingTable.hasZero():
    this.routingTable.replace(message.givenId)
  this.routingTable.validate(message.givenId)
```

### Hub

The hub checks if there is already a device with the requested ID if one is given. If not and there are still IDs available accept the request.

```
registrationRequest(Message message):
  if message.newDevice == 0:
    id = tryGetNewId()
    if id != null:
      message.newDevice = id
      accept(message)
    else:
      reject(message)
    return
  send(message.newDevice, getTypeByte(Ping, NoGroup), [this.id, timestamp])
  timer.timeout = accept(message)
  timer.start()

pingReceived(Message message):
  timer.delete()
  reject(message)

accept(Message message):
  this.routingTable.create(message.newDevice, message.lastHop, valid)
  send(message.givenId, getTypeByte(AcceptReject, NoGroup), [true, message.givenId])
reject(Message message):
  send(message.givenId, getTypeByte(AcceptReject, NoGroup), [false, message.givenId])
```
