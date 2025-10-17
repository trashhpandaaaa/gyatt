# AUR Package Submission Guide

## Files for AUR

This directory contains the necessary files for publishing Gyatt on the Arch User Repository:

- **PKGBUILD** - Build script for Arch Linux
- **.SRCINFO** - Package metadata

## How to Submit to AUR

### 1. First Time Setup

```bash
# Install base-devel if you haven't
sudo pacman -S base-devel

# Clone the AUR repository
git clone ssh://aur@aur.archlinux.org/gyatt-git.git aur-gyatt
cd aur-gyatt
```

### 2. Add Package Files

```bash
# Copy PKGBUILD and .SRCINFO
cp ../PKGBUILD .
cp ../.SRCINFO .

# Add and commit
git add PKGBUILD .SRCINFO
git commit -m "Initial commit: gyatt-git v0.1.0"
```

### 3. Push to AUR

```bash
# Push to AUR
git push origin master
```

### 4. Update .SRCINFO (when updating)

Whenever you update PKGBUILD, regenerate .SRCINFO:

```bash
makepkg --printsrcinfo > .SRCINFO
```

## Testing the Package Locally

Before submitting to AUR, test it:

```bash
# Build and install locally
cd /path/to/gyatt
makepkg -si

# Test installation
gyatt --help
gyatt init
gyatt status

# Uninstall
sudo pacman -R gyatt-git
```

## Updating the Package

When you release a new version:

1. Update pkgver in PKGBUILD if needed (git packages auto-update)
2. Bump pkgrel
3. Regenerate .SRCINFO: `makepkg --printsrcinfo > .SRCINFO`
4. Commit and push to AUR

## Package Info

- **Package name**: gyatt-git
- **Repository**: https://github.com/trashhpandaaaa/gyatt
- **AUR page**: https://aur.archlinux.org/packages/gyatt-git (after submission)

## Installation (for users)

Once submitted to AUR, users can install with:

```bash
# Using yay
yay -S gyatt-git

# Using paru
paru -S gyatt-git

# Manual
git clone https://aur.archlinux.org/gyatt-git.git
cd gyatt-git
makepkg -si
```

## Notes

- This is a `-git` package, meaning it builds from the latest git commit
- Version format: `r{commits}.{short_hash}` (e.g., r10.c66ce1c)
- Users will get automatic updates when running AUR helper updates
- Package will be built from source on user's machine

## Maintainer Info

Update the email in PKGBUILD before submitting!
