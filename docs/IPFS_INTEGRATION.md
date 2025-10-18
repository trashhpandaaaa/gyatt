# IPFS Integration for Gyatt

How Gyatt will use IPFS for decentralized code hosting.

---

## What is IPFS?

**IPFS (InterPlanetary File System)** is a peer-to-peer network for storing and sharing files in a distributed way.

- **Content-addressed**: Files are identified by their hash (CID), not location
- **Decentralized**: No central server, files stored across peer network
- **Permanent**: Content stays available as long as someone pins it
- **Free**: No hosting costs for basic usage

## Why IPFS for Gyatt?

Git objects (blobs, trees, commits) are already content-addressed! They're perfect for IPFS:

1. **Immutable**: Once created, objects never change (perfect for IPFS)
2. **Hash-based**: We already use SHA-1 for identification
3. **Small files**: Most objects are small, ideal for P2P distribution
4. **Deduplication**: Same content = same hash = stored once

## Architecture

```
┌─────────────────────────────────────────────────┐
│  Gyatt Repository (.gyatt/)                     │
│  ├── objects/          ← Traditional storage    │
│  └── ipfs-cache/       ← Downloaded from IPFS   │
└────────────┬────────────────────────────────────┘
             │
    ┌────────▼─────────┐
    │  IPFS Daemon     │
    │  (localhost:5001)│
    │  HTTP API        │
    └────────┬─────────┘
             │
    ┌────────▼─────────────────────┐
    │  IPFS Network (P2P)          │
    │  - Gyatt objects stored here │
    │  - Anyone can access         │
    │  - Free, permanent hosting   │
    └──────────────────────────────┘
```

## Implementation Plan

### Phase 1: Basic Upload/Download

1. **Connect to local IPFS daemon**
   - HTTP API at `http://127.0.0.1:5001`
   - Check daemon is running
   - Ping endpoint: `GET /api/v0/version`

2. **Upload object to IPFS**
   ```
   POST /api/v0/add
   Body: file contents
   Response: {"Name":"...","Hash":"QmXyz...","Size":"..."}
   ```

3. **Download object from IPFS**
   ```
   POST /api/v0/cat?arg=QmXyz...
   Response: file contents
   ```

4. **Pin important objects**
   ```
   POST /api/v0/pin/add?arg=QmXyz...
   ```

### Phase 2: Repository Manifest

Create a manifest file that describes the repository:

```json
{
  "version": "1.0",
  "name": "my-repo",
  "head": "QmCommitHash123...",
  "branches": {
    "main": "QmCommitHash123...",
    "develop": "QmCommitHash456..."
  },
  "objects": {
    "abc123...": "QmBlobHash...",
    "def456...": "QmTreeHash...",
    "ghi789...": "QmCommitHash..."
  }
}
```

Upload manifest to IPFS → Get repository CID → Share it!

### Phase 3: Commands

```bash
# Initialize IPFS support
gyatt ipfs init

# Push repository to IPFS
gyatt ipfs push
# Output: Repository uploaded: ipfs://QmRepo123abc...

# Pull repository from IPFS
gyatt ipfs pull ipfs://QmRepo123abc...

# Pin repository (keep it available forever)
gyatt ipfs pin

# Publish repository (get shareable link)
gyatt ipfs publish
# Output: https://ipfs.io/ipfs/QmRepo123abc
```

## Data Flow

### Pushing to IPFS

1. Read all objects from `.gyatt/objects/`
2. For each object:
   - Upload to IPFS via HTTP API
   - Get CID back
   - Store mapping: `SHA1 → CID`
3. Create repository manifest
4. Upload manifest to IPFS
5. Display repository CID to user

### Pulling from IPFS

1. Download repository manifest by CID
2. Parse manifest to get object list
3. For each object CID:
   - Download from IPFS
   - Verify SHA-1 hash
   - Store in `.gyatt/ipfs-cache/`
4. Reconstruct repository structure

## HTTP API Examples

### Upload a blob

```bash
curl -X POST -F file=@myfile.txt \
  http://127.0.0.1:5001/api/v0/add

# Response:
# {"Name":"myfile.txt","Hash":"QmXyz...","Size":"1234"}
```

### Download a file

```bash
curl -X POST \
  "http://127.0.0.1:5001/api/v0/cat?arg=QmXyz..."

# Response: file contents
```

### Pin a file

```bash
curl -X POST \
  "http://127.0.0.1:5001/api/v0/pin/add?arg=QmXyz..."

# Response: {"Pins":["QmXyz..."]}
```

### Check if file exists

```bash
curl -X POST \
  "http://127.0.0.1:5001/api/v0/refs/local"

# Response: list of pinned CIDs
```

## C Implementation Notes

### HTTP Client

We'll use libcurl for HTTP requests (it's widely available):

```c
#include <curl/curl.h>

// Upload to IPFS
char* ipfs_add(const char *data, size_t size) {
    CURL *curl = curl_easy_init();
    // POST to /api/v0/add
    // Parse JSON response for CID
    return cid;
}

// Download from IPFS
void* ipfs_cat(const char *cid, size_t *size) {
    CURL *curl = curl_easy_init();
    // POST to /api/v0/cat?arg=CID
    return data;
}
```

### CID Format

IPFS uses CIDv0 (base58) and CIDv1 (multibase):
- CIDv0: `Qm...` (46 characters, base58)
- CIDv1: `bafybei...` (newer format)

We'll support both but prefer CIDv1.

### Storage

Store CID mappings in `.gyatt/ipfs-refs`:

```
# Format: SHA1 CID
abc123... QmXyz...
def456... QmAbc...
```

## Cost & Performance

### Free Tier
- IPFS daemon: FREE (run locally)
- Storage: FREE (as long as you pin)
- Bandwidth: FREE (P2P network)

### Pinning Services (optional)
If you want guaranteed availability without running a node:
- Pinata: $0.015/GB/month
- Web3.Storage: FREE (limited)
- Filebase: $0.0059/GB/month

### Performance
- Upload: ~1-5MB/s (depends on daemon)
- Download: ~2-10MB/s (depends on peers)
- Latency: 100-500ms (P2P lookup)

## Security

- **Encryption**: IPFS is public by default
  - Encrypt sensitive repos before upload
  - Use symmetric encryption (AES-256)
  - Share key separately

- **Integrity**: CIDs are cryptographic hashes
  - Content can't be tampered with
  - If hash matches, content is authentic

## Next Steps

1. ✅ Research IPFS (this document)
2. Install IPFS daemon locally
3. Test basic add/cat/pin operations
4. Implement HTTP client in C
5. Create `gyatt ipfs` commands

---

**Resources:**
- IPFS Docs: https://docs.ipfs.tech/
- HTTP API: https://docs.ipfs.tech/reference/kubo/rpc/
- libcurl: https://curl.se/libcurl/
