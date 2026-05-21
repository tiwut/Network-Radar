# 📡 Local Network Radar

An ultra-lightweight, self-hosted Docker utility that sweeps your local network and displays every connected device in a beautiful web dashboard. 

Written in optimized C++ for maximum speed and containerized in a zero-vulnerability Alpine image.

## ✨ Features
- **ARP Scanning:** Bypasses basic firewalls by pinging hardware at the MAC layer.
- **Auto Vendor Resolution:** Identifies devices as Apple, Samsung, Intel, Nintendo, etc., based on their hardware addresses.
- **Smart UI:** Custom CSS radar animations and dynamic icon matching.
- **Zero Overhead:** C++ backend uses virtually 0% CPU while idling.

## 🐳 How to Run
Because this tool needs to send ARP packets on your physical network, it requires specific Docker privileges.

1. Clone the repository.
2. Run Docker Compose:
```bash
docker compose up -d --build
```
3. Open your browser to `http://<your-server-ip>:8080`

**Note:** If you want to change the port, you will need to edit the `main.cpp` file where it says `svr.listen("0.0.0.0", 8080);`, as this container relies on `network_mode: "host"`.
