# http_server
http_server is a simple, multi-threaded HTTP server written in C. It can respond to requests from standard HTTP/1.1 clients and serve static content from the filesystem.

## Features
* Multi-threaded and capable of handling multiple connections in parallel
* Can respond to HTTP GET and HEAD methods from clients
* An index page can be provided when a directory is visited if index.html is present
* A **directory listing** can be provided when a directory is visited if index.html is absent
* Can **percent encode** non-unreserved characters in file names for links to entries in directory listing
* Can list directories with **arbitrary** number of files since each directory entry is visited and sent to the client on the fly
* Can reject unsupported or invalid requests from clients
* Can send an appropriate HTTP error status code when an error is encountered
* Can **prevent path traversal attacks**
* Can **decode percent encoded URI** from client requests
* Support for **HTTP range requests**
* Support for **name-based virtual host**
* Support for **encryption with TLS**

## Building http_server
To build http_server, you need to have “make” utility, “gcc” compiler as well as OpenSSL header files and libraries installed on your system.
The following steps can be used to build http_server:
```
cd src
make
```
The steps above should produce an executable named “http_server” in the same directory

## Running http_server
To show the usage of the server:
```
./http_server
```
By default, the server serves contents from its current working directory.
To run the server on port 9000:
```
./http_server -p 9000
```
You can change the working directory by specifying “-d” option.
To run the server on port 9000 and serve contents from directory “/var/www/html”:
```
./http_server -p 9000 -d /var/www/html
```
The server supports name based virtual hosts by inspecting the “Host” field in HTTP/1.1 request headers.
A virtual host configuration file is a text file where each line starts with a hostname followed by one or more space characters followed by a directory name, which is then terminated by a newline character.
To create a virtual host configuration file named “vh.conf” which defines two hosts, “a.com” and “b.com” which map to “/var/www/a” and “/var/www/b” respectively, type the following commands:
```
echo “a.com /var/www/a” > vh.conf
echo “b.com /var/www/b” >> vh.conf
```
If a hostname in the request is absent from the configuration file, the directory specified in “-d” option will be used, or the current working directory of the server if “-d” unspecified. To run the server on port 9000 with the virtual host configuration file above:
```
./http_server -p 9000 -v vh.conf
```
The server has support for TLS from OpenSSL. To run the server on port 9000 with TLS:
```
./http_server -t 9000 -c cert.pem -k key.pem
```
where cert.pem is your TLS certificate and key is your private key in PEM format.
If you do not have a key or certificate, you can generate them with the following commands, which will produce a private key and a self-signed certificate:
```
openssl genrsa -out key.pem 2048
openssl req -new -x509 -key key.pem -out cert.pem -days 1095
```
The server can work with plain text HTTP and TLS connections at the same time. To run the server with plain text HTTP on port 9000 and TLS on port 9001:
```
./http_server -p 9000 -t 9001 -c cert.pem -k key.pem
```

## Code structure
* connection_handler.c contains code for threads handling plain text HTTP and TLS connections
* dir_listing.c generates and sends directory listings to clients
* file_io.c sends the content of a file to clients
* http_server.c contains the entry point which parses the command line agruments and initialises necessary resources for the server
* process_request.c parses HTTP requests from clients
* process_uri.c canonicalises, decodes the percent encoding and performs sanity checks on URIs received from clients
* response_header.c sends HTTP Status-Line and Response Header Fields to clients
* vhost_table.c provides hostname to directory lookup for virtual hosts

## Words on memory usage
All resources allocated for each new network connection will be freed upon disconnection.
That is, the memory usage of the server after serving one connection remains the same as that after serving 1000 connections.
If you run the server with valgrind and kill the server while it is still running, you might see some "still reachable" bytes in leak summary.
These are caused by one-time allocations of some data structures required by the server as well as the OpenSSL library.
The server is supposed to run indefinitely and the OS will perform the cleanup after it being killed.

## Acknowledgements
* [uthash](https://troydhanson.github.io/uthash/) for hash table to store hostname and directory pairs for virtual hosts
* [OpenSSL](https://www.openssl.org) for TLS support
