# Gyatt Documentation Website

This directory contains the official documentation website for Gyatt.

## Structure

- `index.html` - Main documentation page
- `style.css` - Minimalistic CSS styling
- `script.js` - Interactive JavaScript features

## Features

- Clean, minimalistic design
- Responsive layout for all devices
- Smooth scrolling navigation
- Copy-to-clipboard for code examples
- Animated section reveals
- Mobile-friendly interface

## Viewing Locally

Simply open `index.html` in your web browser:

```bash
# Using your default browser
open index.html  # macOS
xdg-open index.html  # Linux
start index.html  # Windows
```

Or use a local web server:

```bash
# Python 3
python -m http.server 8000

# Node.js (with http-server)
npx http-server

# Then visit: http://localhost:8000
```

## Sections

1. **Home** - Introduction and quick start
2. **Features** - Key features of Gyatt
3. **Installation** - Installation instructions for various platforms
4. **Documentation** - Complete command reference
5. **IPFS Integration** - IPFS setup and usage guide
6. **Technical Details** - Architecture and implementation details

## Deployment

This site can be deployed to:
- GitHub Pages
- IPFS (via `gyatt ipfs publish`)
- Any static hosting service (Netlify, Vercel, etc.)

## Contributing

To update the documentation:
1. Edit the HTML/CSS/JS files
2. Test locally in your browser
3. Submit a pull request

## License

Part of the Gyatt project.
