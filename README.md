# oscour

Desired hl api:

```C++
sender.send("/baz", 123, 4.56, "booh");
receiver.on("/foo/bar", [] (float f, int b, array<2> col) {
   ...
});
```
