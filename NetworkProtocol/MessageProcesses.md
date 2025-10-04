# Send and Receive

Here the internal send and receive methods are presented. These are the same for all message types. The distinction happens in the `processMessage()` method.

## Sending

All messages are added to a send queue. If the message has not been acknowledged after a timeout, resend it.

### Send

```
send(byte destination, byte messageTypeAndGroupFlags, byte[] payload)
  create message object
  set version
  set message type and group flags
  set payload
  ID = nextID
  nextID = (nextID + 1) mod 256
  if group flag set:
    if this is hub:
      reset groupAscending flag
    if groupAscending set:
      set nextHop to parent
    else:
      set nextHop to all children
  else:
    if destination in routingList:
      set nextHop according to routing list
    else:
      set nextHop to parent
  set checksum
  send to nextHop
```
Payload is set according to the message type.

## Receiving

All received messages are added to a receive queue, so if a acknowledgement gets lost and an already processed message is resent, it is ignored, but acknowledged.

### Receive

```
receive(Message message)
  if group flag set:
    if groupAscending not set:
      if this is in message.destination group:
        processMessage(message)
    send(message)
  else:
    if message.destination == this.id or message.actionAlsoAtNotDestination:
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

Setting up a new device uses the `setup` method.

```
setup(byte id = 0)
  payload = [id]
  if id == 0:
    tempId = time()
    payload.add(tempId)
  for i in 0,…,253:
    send(i, getTypeByte(Discover, NoGroup), [id]) // send and listen on discovery channel 254
  wait x seconds
  pick parent in answered with highest level
  register(parent, id, tempId)
```

The `register` method starts the registration at the parent with the given ID.

```
register(byte parent, byte id, uint32_t tempId)
  if id == 0:
    tempId = tempId
  else:
    this.id = id
  this.parent = parent
  send(parent, getTypeByte(Register, NoGroup), [id, tempId])
```

When a response is received, the registration is finished with the following method.

```
finishRegistration(Message message):
  if message.isReject:
    return
  this.id = message.givenId
  this.registered = true
```

### Registration at parent

The parent initiates the route creation for the new device.

```
registrationMessage(Message message):
  id = message.id
  if id == 0:
    id = message.tempId
  this.tempRouteTable.create(id, discoverChannel, invalid)
  send(this.parent, getTypeByte(RouteCreation, NoGroup), [this.Id])
```

### Each hop on the way

Each hop adds the new device to its routing table.

```
registrationUpwards(Message message):
  id = message.id
  if id == 0:
    id = message.tempId
  this.tempRouteTable.create(id, message.sender, invalid)
  send(this.parent, getTypeByte(RouteCreation, NoGroup), [this.Id])

registrationDownwards(Message message):
  id = message.id
  if id == 0:
    id = message.tempId
  send(message)
  if message.isReject:
    this.tempRouteTable.remove(id)
    return
  this.routingTable.create(message.givenId, tempRouteTable.get(id), valid)
  this.tempRouteTable.remove(id)
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

## Disconnect

The hub periodically pings each device. The disconnect method is executed by all device on the routing path to the device.

```
pingEachDeviceEvery
pingId
numberDevices = this.routingTable.length
sendPingAfter = pingEachDeviceEvery / numberDevices
if time() - lastPing > sendPingAfter:
  send(pingId, getTypeByte(Ping, NoGroup), [this.id, timestamp])
  timer.timeout = disconnect(pingId)
  timer.start()
  lastPing = time()
  pingId = (pingId + 1) mod numberDevices

disconnect(byte id):
  send(id, getTypeByte(Disconnect, NoGroup), [])
  this.routingTable.remove(id)
```
