# oscour: C++17 OSC library

This library is an updated fork of [OscPack](http://www.rossbencina.com/code/oscpack).

It uses ASIO for networking in order to allow OSC over TCP, serial port, websockets... 
It also provides new high-level wrappers to simplify application code.


Desired hl api:

```C++
sender.send("/baz", 123, 4.56, "booh");
receiver.on("/foo/bar", [] (float f, int b, array<2> col) {
   ...
});
```
