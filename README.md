# KitchenRadio-BTSlave
Bluetooth to I²S slave for the KitchenRadio

## Commands
### AT+RESET
Restart the ESP

### AT+START
Start the A2DP sink.

### AT+END
Stops the sink (will disconnect a connected device)
The ESP will reset automatically following this command.

### AT+PLAY / AT+PAUSE / AT+STOP
Play / pause / stop the audio.