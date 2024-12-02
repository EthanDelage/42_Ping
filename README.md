# ft_ping

This is a project for the 42 school which implements a simple ping utility in C.

## Usage

```bash
Usage: ft_ping [-dv?] [-c NUMBER] [-i NUMBER] [-s NUMBER] [-W N]
            [--count=NUMBER] [--debug] [--interval=NUMBER] [--size=NUMBER]
            [--ttl=N] [--verbose] [--linger=N] [--help] [--usage] HOST
```

## Options

| Short opt | Long opt            | Description                                     |
|:---------:|---------------------|-------------------------------------------------|
| `-c`      | `--count=NUMBER`    | stop_ping after sending NUMBER packets          |
| `-d`      | `--debug`           | set the SO_DEBUG option                         |
| `-i`      | `--interval=NUMBER` | wait NUMBER seconds between sending each packet |
| `-s`      | `--size=NUMBER`     | send NUMBER data octets                         |
|           | `--ttl=N`           | specify N as time-to-live                       |
| `-v`      | `--verbose`         | verbose output                                  |
| `-W`      | `--linger=N`        | number of seconds to wait for response          |
| `-?`      | `--help`            | Give this help list                             |
|           | `--usage`           | Give a short usage message                      |

## Description
The ft_ping command sends ICMP ECHO_REQUEST packets to network hosts, emulating the behavior of the standard ping utility.
It measures the round-trip time and packet loss rate between the source and destination hosts.

## Examples

Send 5 ICMP packets to example.com:

```bash
./ft_ping -c 5 example.com
```

Send 3 ICMP packets with increased verbosity to google.com:

```bash
./ft_ping -v -c 3 google.com
```

## Build Instructions
To build the ft_ping executable, use make:

```bash
make
```

This will compile the source code and produce the ft_ping executable.

## Requirements
This project requires a Unix-like system capable of sending and receiving ICMP packets.

Root privileges might be required for some functionalities (e.g., raw socket creation).

## Author
This project was developed by **edelage**.

## Disclaimer
Use this software responsibly and only on networks and hosts that you have permission to test.

ICMP packets and network probing tools can be subject to security policies and legal regulations.
