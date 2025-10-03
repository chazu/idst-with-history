# IDST Documentation

This directory contains the documentation for the IDST (Id with Smalltalk) programming language system.

## Generated HTML Files

The following HTML documentation files have been generated:

- **[index.html](index.html)** - Main documentation index with links to all documents
- **[pepsi.html](pepsi.html)** - Complete guide to the Pepsi programming language
- **[coke.html](coke.html)** - Coke programming guide (functional abstract syntax processor)
- **[prototypes.html](prototypes.html)** - Technical memo on Lieberman-style prototypes

## Source Files

The HTML files are generated from template files using AWK scripts:

### Template Files (.html.in)
- `pepsi.html.in` - Pepsi language documentation template
- `coke.html.in` - Coke programming guide template  
- `prototypes.html.in` - Prototypes technical memo template

### AWK Processing Scripts
- `number.awk` - Adds section numbering to headings (h2, h3, h4)
- `top.awk` - Extracts content before the table of contents marker
- `toc.awk` - Generates table of contents from headings
- `bot.awk` - Extracts content after the table of contents marker

## Generating HTML

### Using the Script (Recommended)
```bash
./generate-html.sh
```

### Using Make
```bash
make pepsi.html coke.html prototypes.html
```

### Manual Process
For each document (e.g., pepsi):
```bash
awk -f number.awk pepsi.html.in > pepsi.html.num
awk -f top.awk pepsi.html.num > pepsi.html.top
cat pepsi.html.num | fgrep -v h1 | awk -f toc.awk > pepsi.html.toc
awk -f bot.awk pepsi.html.num > pepsi.html.bot
cat pepsi.html.top pepsi.html.toc pepsi.html.bot > pepsi.html
```

## How It Works

1. **Numbering**: The `number.awk` script adds hierarchical numbering to section headings
2. **Top Section**: `top.awk` extracts everything before the `<!--toc-->` marker
3. **Table of Contents**: `toc.awk` generates a formatted TOC from all headings
4. **Bottom Section**: `bot.awk` extracts everything after the `<!--toc-->` marker  
5. **Assembly**: All parts are concatenated to create the final HTML file

## Viewing the Documentation

1. Open `index.html` in a web browser for the main documentation index
2. Or open any individual HTML file directly:
   - `pepsi.html` - Main language documentation
   - `coke.html` - Coke/Jolt programming guide
   - `prototypes.html` - Technical implementation details

## File Structure

```
doc/
├── README.md              # This file
├── index.html             # Documentation index page
├── generate-html.sh       # HTML generation script
├── Makefile              # Original build configuration
├── *.html.in             # HTML template files
├── *.awk                 # AWK processing scripts
└── *.html                # Generated HTML documentation
```

## Notes

- The table of contents is automatically generated from h2, h3, and h4 headings
- Section numbers are automatically assigned in hierarchical order
- The `<!--toc-->` marker in template files indicates where the TOC should be inserted
- Intermediate files (*.num, *.top, *.toc, *.bot) are automatically cleaned up after generation
