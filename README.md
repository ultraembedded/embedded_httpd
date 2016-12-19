### Embedded HTTP Server

Github:   [http://github.com/ultraembedded/embedded_httpd](https://github.com/ultraembedded/embedded_httpd)

A simplified HTTP 1.0 webserver library suitable for low-end embedded devices.

This webserver can either serve dynamic, generated content or files, if available.

No dynamic memory allocation, configurable memory footprint, and all stdio/libc/network I/O via user defined macros.

##### Build Options

See Configuration.txt. You will likely need to change HTTP_OPT_USE_PATH and HTTP_OPT_FILE_PATH.

##### Compile

The compile a test version for Linux;

cd example/linux

make

./test

##### Test

Then on the same machine;

firefox localhost:8080

firefox localhost:8080/hello.htm
