# arduino-serialwebserver
A webserver that runs on an Arduino microcontroller (tested on Arduino Uno)

# Purpose
Experimentation, of course! This also has potential to be useful for IoT applications and remote configuration, however.
If you want to use practical use out of this, it would be best to run it on a device with more than 2k RAM.

# Dependencies
This project relies on the [Arduino-MemoryFree](https://github.com/mpflaga/Arduino-MemoryFree) library's `F` function, which is used in place of `String` whereever possible.
You need to install the library to use this project.

# Why are strings in this project so messy?
Arduino's normal String implementation allocates RAM for literal Strings that you have in your code, and don't just use flash memory. This is a problem.
For this reason, this project makes heavy use of Arduino-MemoryFree's `F` function, which uses flash memory instead. Without it, this project would use double the memory, at least.

# Usage
Since most Arduino boards only have serial communication to use for talking to a computer, we need to use a serial to TCP bridge to expose the webserver to browsers.
[serial2tcp](https://github.com/termermc/serial2tcp) is an implementation I wrote for this purpose. If you want to use another implementation that you know of, then that should work too.

Before uploading to your project, there are a few sections you may want to change.

At the very top, you can find the following line:

```c
#define HTTP_MAX_HEADERS 6
#define BAUD 115200
```

`HTTP_MAX_HEADERS` defines how many headers will be read from HTTP requests (the rest will be skipped). If this is too high, your board may run out of memory and you won't be able to serve pages.

`BAUD` is pretty self-explanatory; it's the baud rate for serial communication.

The `loop` function is where you're free to add your own code. By default, it waits for an HTTP request to come in, parses it, and then serves a page based on the path requested.
To write your own logic, take a look at the `HttpRequest` struct and the `sendHttpResponse` and `sendHttpRedirect` functions.
You can also use the `getHeader` function to get header values by name from requests, but be aware that if your `HTTP_MAX_HEADERS` value is too low, it may not have been able to read all the headers you wanted.

First, compile and upload this project to your Arduino. Next, connect your Arduino to your computer using a USB cable. From here, you want to use your serial to TCP bridge to create a TCP server for it.
Assuming your TCP port is 8080, you can now visit [http://localhost:8080/](http://localhost:8080/) in your browser!

# Troubleshooting
Some problems that I faced.

## Why is the server returning an empty page?
It ran out of memory. If the server ran out of memory while composing a page, then it will just return an empty page. Try lowering `HTTP_MAX_HEADERS`.

## Why is it reading request data wrong?
This could be a variety of things, but most likely it's too much data being sent to the server at once. Try raising your baud rate so that it won't get so congested.

## Why can't it see a paricular request header?
Your `HTTP_MAX_HEADERS` value is probably too low and is cutting off the header you want.

# Further experimentation
You could easily create a web application that controls lights on your Arduino, or returns sensor information.

You could also use this as an example of IoT devices.
