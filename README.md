# Custom VPN Using TUN/TAP and OpenSSL

This project implements a simple VPN using TUN/TAP interfaces, OpenSSL encryption, and TCP sockets. It securely forwards packets between a client and a server.

## How to Run the VPN

### 1. Setup TUN Interfaces

On both the client and server, set up the TUN interface:

```bash
sudo ip tuntap add dev tun0 mode tun
sudo ip link set tun0 up
```

Assign IP addresses:

- **Server:**
  ```bash
  sudo ip addr add 10.0.0.1/24 dev tun0
  ```
- **Client:**
  ```bash
  sudo ip addr add 10.0.0.2/24 dev tun0
  ```

### 2. Compile the Code

Use `gcc` to compile both the server and client:

```bash
gcc vpn_server.c -o vpn_server -lssl -lcrypto
gcc vpn_client.c -o vpn_client -lssl -lcrypto
```

### 3. Run the VPN

- **On the Server:**
  ```bash
  sudo ./vpn_server
  ```

- **On the Client:**
  ```bash
  sudo ./vpn_client
  ```

### 4. Verify Connectivity

#### Check TUN interfaces
Run the following command on both client and server to ensure `tun0` is active:

```bash
ip addr show tun0
```

#### Ping between Client and Server

- **From Client to Server:**
  ```bash
  ping 10.0.0.1
  ```

- **From Server to Client:**
  ```bash
  ping 10.0.0.2
  ```

If the pings are successful, your VPN is working.
