# Gyatt Development Roadmap

30 steps to make Gyatt the coolest version control system.

---

## ✅ **Completed: Foundation (Steps 1-10)**

- [x] **Step 1-3**: Project setup, SHA-1 hashing, repository initialization
- [x] **Step 4-6**: Object storage (blobs/trees/commits), staging area, commit functionality
- [x] **Step 7-8**: Status command, branch management
- [x] **Step 9-10**: TCP server, basic network push/pull

**Status**: Core VCS working! Can init, add, commit, branch, and run a server.

---

## 🚧 **Phase 1: IPFS Integration (Steps 11-15)**

### **Step 11: IPFS Setup & Research**
- [ ] Install IPFS daemon
- [ ] Test basic IPFS commands
- [ ] Research IPFS HTTP API
- [ ] Document IPFS integration strategy

### **Step 12: IPFS C Client Library**
- [ ] Create `src/ipfs/` directory
- [ ] Write HTTP client for IPFS API
- [ ] Implement connection to local IPFS daemon
- [ ] Add error handling

### **Step 13: Upload Objects to IPFS**
- [ ] Implement `ipfs_add_object()` function
- [ ] Upload blobs to IPFS
- [ ] Store CID (Content ID) mappings
- [ ] Test with sample files

### **Step 14: Download from IPFS**
- [ ] Implement `ipfs_get_object()` function
- [ ] Verify downloaded content
- [ ] Handle missing objects
- [ ] Add retry logic

### **Step 15: IPFS Commands**
- [ ] `gyatt ipfs init` - Initialize IPFS support
- [ ] `gyatt ipfs push` - Upload repo to IPFS
- [ ] `gyatt ipfs pull` - Clone from IPFS
- [ ] Test end-to-end workflow

---

## 🌐 **Phase 2: Web UI (Steps 16-20)**

### **Step 16: Static Web Interface**
- [ ] Create `web/` directory
- [ ] Build basic HTML/CSS/JS structure
- [ ] Design modern, clean UI
- [ ] Test locally

### **Step 17: IPFS.js Integration**
- [ ] Add IPFS.js library
- [ ] Connect to IPFS from browser
- [ ] Fetch repository manifest
- [ ] Display repo info

### **Step 18: Repository Browser**
- [ ] List branches
- [ ] Show commit history
- [ ] Display file tree
- [ ] Add navigation

### **Step 19: Code Viewer**
- [ ] Fetch files from IPFS
- [ ] Add syntax highlighting
- [ ] Show line numbers
- [ ] Support images/binaries

### **Step 20: Diff Viewer**
- [ ] Show commit diffs
- [ ] Side-by-side comparison
- [ ] Highlight changes
- [ ] Navigate between files

---

## 🎨 **Phase 3: Advanced Web Features (Steps 21-25)**

### **Step 21: Host UI on IPFS**
- [ ] Build static web bundle
- [ ] Upload UI to IPFS
- [ ] Get permanent UI link
- [ ] Test access via ipfs.io

### **Step 22: Deep Linking**
- [ ] Support `#/repo/QmXyz` URLs
- [ ] Direct links to commits/files
- [ ] Browser back/forward
- [ ] Shareable URLs

### **Step 23: Search & Filter**
- [ ] Search commits by message
- [ ] Search files by name
- [ ] Search code content
- [ ] Filter by author/date

### **Step 24: Repository Stats**
- [ ] Commit count
- [ ] Contributors list
- [ ] Language breakdown
- [ ] Activity graphs

### **Step 25: Clone Button**
- [ ] Show clone command
- [ ] Copy to clipboard
- [ ] QR code generation
- [ ] Instructions for newbies

---

## 🔗 **Phase 4: Git Compatibility (Steps 26-30)**

### **Step 26: Read Git Repos**
- [ ] Implement Git pack format parser
- [ ] Read Git objects
- [ ] Convert Git → Gyatt objects
- [ ] Test with real Git repos

### **Step 27: Write Git Format**
- [ ] Implement Git pack writer
- [ ] Convert Gyatt → Git objects
- [ ] Support Git smart HTTP
- [ ] Handle authentication

### **Step 28: Push to GitHub**
- [ ] `gyatt push github.com/user/repo`
- [ ] Support HTTPS transport
- [ ] Handle credentials
- [ ] Test with real GitHub repos

### **Step 29: Clone from Git**
- [ ] `gyatt clone https://github.com/user/repo`
- [ ] Fetch Git objects
- [ ] Convert to Gyatt format
- [ ] Optional: Also push to IPFS

### **Step 30: Hybrid Push**
- [ ] `gyatt push --all` command
- [ ] Push to IPFS + Git remotes simultaneously
- [ ] Show progress for each
- [ ] Handle partial failures
- [ ] **Release v1.0** 🎉

---

## 🚀 **Phase 5: Advanced Features (Future)**

Ideas for after v1.0:

- [ ] Real-time collaboration (WebSocket server)
- [ ] Built-in code review system
- [ ] Encrypted repositories
- [ ] Multi-threaded operations
- [ ] Smart conflict resolution
- [ ] Plugin system
- [ ] Mobile apps (iOS/Android)
- [ ] VS Code extension
- [ ] CI/CD integration

---

## 📊 **Progress Tracking**

```
Phase 1: Foundation        [██████████] 10/10 (100%) ✅
Phase 2: IPFS Core         [░░░░░░░░░░]  0/5  (0%)
Phase 3: Web UI            [░░░░░░░░░░]  0/5  (0%)
Phase 4: Advanced Web      [░░░░░░░░░░]  0/5  (0%)
Phase 5: Git Compatibility [░░░░░░░░░░]  0/5  (0%)

Overall: [██░░░░░░░░░░░░░░░░░░] 10/30 (33%)
```

---

## 🎯 **Milestones**

- **M1: Foundation** ✅ - Basic VCS working
- **M2: IPFS Integration** - After Step 15
- **M3: Web UI** - After Step 20
- **M4: Production Web** - After Step 25
- **M5: Git Compatible** - After Step 30
- **M6: v1.0 Release** - Full feature set

---

## ⏱️ **Estimated Timeline**

- Steps 11-15 (IPFS): ~2-3 weeks
- Steps 16-20 (Web UI): ~2 weeks
- Steps 21-25 (Advanced Web): ~1-2 weeks
- Steps 26-30 (Git): ~2-3 weeks

**Total**: ~8-10 weeks for v1.0

---

## 💡 **Current Focus**

**Next Up**: Step 11 - IPFS Setup & Research

Stay tuned! We're building something awesome here. 🚀
