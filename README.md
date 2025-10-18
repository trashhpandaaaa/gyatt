# Gyatt

A version control system built from scratch in C. Like Git, but with some cool extras.

## Why though?

Git is great, but I wanted to build something that:
- Has a **built-in server** (no GitHub needed for private projects)
- Supports **IPFS** for free decentralized hosting
- Works with Git repos too (also coming soon)
- Is actually understandable

Plus, building your own VCS is just fun

## What works right now

All the basics: `init`, `add`, `commit`, `status`, `log`, `branch`, `checkout`  
**Built-in server** - `gyatt server 9999` and you have your own Git-like server  
Network push/pull (basic version working)  

## What's coming
 
Git compatibility - push to GitHub, pull from any Git repo  
Web UI - browse repos in your browser  
Real-time collaboration features  

## Building

```bash
make
./bin/gyatt help
```

You need GCC, Make, and zlib.

## Quick start

```bash
# Basic workflow
gyatt init
gyatt add .
gyatt commit -m "first commit"
gyatt status

# Branches
gyatt branch feature
gyatt checkout feature

# Run your own server
gyatt server 9999

# Push to it (from another machine)
gyatt push 192.168.1.100:9999
```

## The plan

I'm building this in 30 steps:
1. Core VCS features (init, add, commit, etc.)
2. Built-in TCP server
3. Git protocol support
4. Web UI on IPFS
5. Advanced features (encryption, collaboration, etc.)

Check out the full roadmap in the code.

## Why "Gyatt"?

Why not? 

## Contributing

Found a bug? Have an idea? PRs welcome!

---

Built with C and curiosity. Star if you think decentralized version control is cool 

