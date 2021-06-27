![](https://github.com/darkbox/InverterRemote/blob/main/InverterRemoteAndroid/app/src/main/res/mipmap-xhdpi/ic_launcher_round.png?raw=true)
# InverterRemote
Control a chinese inverter On/Off/Reset switch with arduino nano, ethernet shield and Android app.

The "worker" controls the inverter with a relay contected to the pin `A0`.
Default "worker" IP `192.168.1.140` with MAC `90:A2:DA:0D:78:EE`.

The "sentinel" is optional and checks the status of the "worker". It also works as a physical remote to reset the "worker".
Default "sentinel" IP `192.168.1.141` with MAC `90:A2:DA:0D:78:EF`.

Example http request:
```http
http://192.168.1.140?s=1
```
Parameter `s` can be `0` (off) or `1` (on)

Note: You must be on the same network in order to work.
