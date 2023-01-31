# File Server

Below are the following server types supported.

A compile definiton 'SERVERT' is used to compile specific server

| Supported Server  | SERVERT value |
| ------------- | ------------- |
| Echo Server ability to handle only single client at a time  | 1  |
| File Server ability to handle only single client at a time  | 2  |
| Multi Threaded Echo Server ability to handle upto NTHREADS clients   | 3  |
| Echo Server ability to handle multiple clients using select() syscall  | 4  |
| Echo Server ability to handle multiple clients using poll() syscall  | 5  |
| Echo Server ability to handle multiple clients using epoll() syscall  | 6  |

A compile definition 'NTHREADS' is used to specify the no.of threads for the threadpool only when you choose the 'SERVERT' as 3

### To compile:

1. Echo Server ability to handle only single client at a time
    + `./run.sh -o "configure" -s 1`

2. File Server ability to handle only single client at a time
    + `./run.sh -o "configure" -s 2`

3. Multi Threaded Echo Server ability to handle upto NTHREADS clients
    + `./run.sh -o "configure" -s 1 -t 2`
    + -t flag is used to specify the no.of threads to initialize the threadpool

4. Echo Server ability to handle multiple clients using select() syscall
    + `./run.sh -o "configure" -s 4`

5. Echo Server ability to handle multiple clients using poll() syscall
    + `./run.sh -o "configure" -s 5`

6. Echo Server ability to handle multiple clients using epoll() syscall
    + `./run.sh -o "configure" -s 6`

### To build:

`$ ./run.sh -o "build" `

### To run:

1. To run the server
    + `$ ./run.sh -o "run_server" `

2. To run the server
    + `$ ./run.sh -o "run_client" `


#### A lot of changes will happen in the project in the future