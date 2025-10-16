# Gyatt

A version control system built from scratch in C. Like Git, but with built-in private server support.

## Why?

Git is great, but what if you want to collaborate with your team without relying on GitHub or GitLab? Gyatt lets you host your own repositories on any computer and work together over your local network.

## Features

- All the basics: init, add, commit, status, log, branch, checkout
- Built-in server mode - turn any computer into a Git-like server
- Push/pull to your private server AND to GitHub
- Fast and lightweight

## Building

You need GCC and Make:

```bash
make
./bin/gyatt help
```

## Usage

Just like Git, but simpler:

```bash
gyatt init
gyatt add .
gyatt commit -m "first commit"
gyatt server start          # start your own server
gyatt push myserver         # push to your server
gyatt push github           # also push to GitHub
```

## Status

Work in progress. Building it step by step.
