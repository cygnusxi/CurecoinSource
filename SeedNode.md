# 🌐 CureCoin Seed Node Setup Guide

Running a seed node is a valuable contribution to the CureCoin ecosystem. Seed nodes provide the initial points of contact for new clients joining the network, ensuring fast synchronization and a healthy peer-to-peer structure.

---

## 📋 Prerequisites



- ✅ **Fully synced CureCoin client** (wallet/daemon)
- ✅ **Port forwarding capability** on your router/firewall
- ✅ **Public address** - ONE of the following:
  - Static IP address
  - Domain with subdomain capability
  - Dynamic DNS (DDNS) service

---

## ⚙️ Quick Setup

### 1️⃣ Configure Connection Limits

Seed nodes handle high traffic. Edit your `curecoin.conf` file and add:

```conf
maxconnections=1000
```

> **Note:** Expect your node to maintain around 500 active connections.

### 2️⃣ Open Network Port

Configure your firewall/router to forward **TCP port 9911** to your CureCoin client machine.

### 3️⃣ Choose Your Node Address

You must provide a stable address for your node. Choose one option:

#### **Option A: Domain / DDNS**

Use your own domain or Dynamic DNS service:

- **Example:** `seed-nyc.yoursite.com` or `seednode1234.ddns.net`
- **Naming:** Include "seed" and a unique identifier
- ⚠️ Offensive or inappropriate names will be rejected

#### **Option B: Static IP**

If you have a static IP but no domain, the CureCoin team can assign you a subdomain:

- **Format:** `seednodeX.curecoin.net` (where X is the next sequential number)
- **Custom names** are available upon request
- Check the [CureCoin GitHub](https://github.com/cygnusxi/CurecoinSource) for the next available number

---

## 📝 Registration

Once your node is configured and accessible:

1. **Verify** your IP or domain is publicly reachable on port 9911
2. **Contact** the CureCoin team or submit a pull request on GitHub with your node details
3. **Wait** for merge - new clients will then automatically discover your node

---

## 🔧 Advanced Configuration

### 🔒 Security Best Practices

**⚠️ CRITICAL: Empty Wallet Policy**

Never store coins on your seed node. Keep a zero balance. The node's only purpose is to relay block and peer data.

#### **Deployment Options** (Best Practices)

Choose one of these isolation methods:

- 🖥️ **Virtual Machine (VM)** - Run the node in a VM with network isolation
- ☁️ **Dedicated VPS** - Use a cloud VPS (DigitalOcean, Linode, AWS, etc.)
- 🏠 **Dedicated Machine** - Use a separate physical machine at home

#### **Network Isolation**

- ❌ Do not expose domains/IPs hosting other sensitive services
- ❌ Do not run on machines with unsecured ports, home automation, or file servers
- ✅ Configure firewalls to allow subnet access to the node
- ✅ Block the node from initiating connections back to your private network

#### **System-Level Security**

- Create a **dedicated OS user** specifically for running CureCoin
- Use **minimal permissions** for the CureCoin process
- Keep your system and CureCoin client **updated**
- Monitor logs for suspicious connection patterns

#### **Additional Security Measures**

```conf
# Add to curecoin.conf for enhanced security
listen=1
discover=1
# Limit connections if needed
maxconnections=1000
# Disable wallet functionality (if supported)
disablewallet=1
```

### 📊 Enabling RPC for Network Stats

⚠️ **Warning:** This opens an additional port and requires strict security controls.

If you wish to contribute to network statistics (block explorers, status pages), enable JSON-RPC:

Edit your `curecoin.conf`:

```conf
server=1
rpcuser=<StrongUsername>
rpcpassword=<StrongAlphanumericPassword>
rpcallowip=<Parser_IP_Address>
rpcallowip=127.0.0.1
```

> 🔐 **Security Note:** Strictly limit `rpcallowip` to `127.0.0.1` (localhost) and the specific IP of the stats server. **Never** set this to `0.0.0.0/0` or allow wildcard access. Contact the dev team for the current parser IP address.

### 🐳 Docker Deployment

For users comfortable with containerization, a community-created Docker build is available on the [CureCoin GitHub](https://github.com/cygnusxi/CurecoinSource). Docker provides excellent isolation similar to a VM.

**Benefits:**
- Easy deployment and updates
- Built-in isolation from host system
- Simplified port management
- Consistent environment across platforms

---

## 🆘 Support

Need help? Reach out to the CureCoin community:

- **GitHub:** [CurecoinSource Repository](https://github.com/cygnusxi/CurecoinSource)
- **Issues:** Submit questions or problems via GitHub Issues

---

**Thank you for contributing to the CureCoin network! 💚**