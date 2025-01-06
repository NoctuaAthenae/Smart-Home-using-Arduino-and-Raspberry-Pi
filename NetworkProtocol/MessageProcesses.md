Here the internal send and receive methods are presented.

# Sending

Message sending works similar for most message Types. All messages are added to a send queue. If the message has not been acknowledged after a timeout, resend it.

## Send

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

# Receiving

All received messages are added to a receive queue, so if a acknowledgement gets lost and an already processed message is resent, it is ignored, but acknowledged.

## Receive

```
receive(byte[] message)
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
