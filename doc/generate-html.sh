#!/bin/bash

# Script to generate HTML documentation from .html.in template files
# This script processes the HTML templates using AWK scripts to create
# numbered sections and table of contents

echo "Generating HTML documentation..."

# List of HTML files to generate (without .html.in extension)
HTML_FILES="pepsi coke prototypes"

for file in $HTML_FILES; do
    if [ -f "${file}.html.in" ]; then
        echo "Processing ${file}.html.in..."
        
        # Step 1: Add section numbers
        awk -f number.awk "${file}.html.in" > "${file}.html.num"
        
        # Step 2: Extract top part (before table of contents)
        awk -f top.awk "${file}.html.num" > "${file}.html.top"
        
        # Step 3: Generate table of contents
        cat "${file}.html.num" | fgrep -v h1 | awk -f toc.awk > "${file}.html.toc"
        
        # Step 4: Extract bottom part (after table of contents)
        awk -f bot.awk "${file}.html.num" > "${file}.html.bot"
        
        # Step 5: Combine all parts
        cat "${file}.html.top" "${file}.html.toc" "${file}.html.bot" > "${file}.html"
        
        echo "Generated ${file}.html"
    else
        echo "Warning: ${file}.html.in not found, skipping..."
    fi
done

# Clean up intermediate files
echo "Cleaning up intermediate files..."
rm -f *.num *.top *.toc *.bot

echo "HTML generation complete!"
echo ""
echo "Generated files:"
for file in $HTML_FILES; do
    if [ -f "${file}.html" ]; then
        echo "  - ${file}.html"
    fi
done

echo ""
echo "Open index.html in a web browser to view the documentation."
