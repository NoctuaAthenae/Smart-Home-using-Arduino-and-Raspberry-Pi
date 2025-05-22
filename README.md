# Smart Home using Arduino and Raspberry-Pi

This project aims to provide scripts for a wireless (2.4 Ghz using NRF24L01 modules) network with tree topology of Arduinos fullfilling different roles in a Smart Home, while a Raspberry Pi acts as the central hub and a server for a web interface.

# Roadmap

- [X] Define Network Protocol
- [ ] Network Library
- [ ] Web Admin GUI
- [ ] Routing of the hub
- [ ] Web control panel GUI
- [ ] Android App control panel GUI

# Setup

## Endpoints (Arduinos)

The endpoints can be Control Panel, Displays, Sensors, etc. or can combine multiple of these. Control Panels and Sensors send data to the hub, which sends data to Displays, Lamps, LEDs, etc.

## Central Hub (Raspberry Pi)

The hub coordinates routing between the endpoints and provides a server for a web interface and an API. It also saves states of endpoints.
