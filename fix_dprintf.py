#!/usr/bin/env python3

import re
import sys

def count_format_args(format_str):
    """Count the number of format specifiers in a printf format string"""
    # Find all format specifiers like %s, %p, %d, %ld, etc.
    # This is a simplified version - doesn't handle all edge cases
    format_specs = re.findall(r'%[-#+ 0]*\*?(?:\d+|\*)?(?:\.(?:\d+|\*))?[hlL]?[diouxXeEfFgGaAcspn%]', format_str)
    # Filter out %% (literal %)
    format_specs = [spec for spec in format_specs if spec != '%%']
    return len(format_specs)

def fix_dprintf_calls(content):
    """Replace dprintf calls with numbered versions"""
    
    def replace_dprintf(match):
        full_match = match.group(0)
        format_str = match.group(1)
        
        # Count format arguments
        arg_count = count_format_args(format_str)
        
        # Replace dprintf with dprintfN
        return full_match.replace('dprintf(', f'dprintf{arg_count}(')
    
    # Pattern to match dprintf calls with format string
    pattern = r'dprintf\(("(?:[^"\\]|\\.)*")'
    
    return re.sub(pattern, replace_dprintf, content)

def fix_fatal_calls(content):
    """Replace fatal calls with numbered versions"""
    
    def replace_fatal(match):
        full_match = match.group(0)
        format_str = match.group(1)
        
        # Count format arguments
        arg_count = count_format_args(format_str)
        
        # Replace fatal with fatalN
        return full_match.replace('fatal(', f'fatal{arg_count}(')
    
    # Pattern to match fatal calls with format string
    pattern = r'fatal\(("(?:[^"\\]|\\.)*")'
    
    return re.sub(pattern, replace_fatal, content)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 fix_dprintf.py <file>")
        sys.exit(1)
    
    filename = sys.argv[1]
    
    with open(filename, 'r') as f:
        content = f.read()
    
    # Fix dprintf calls
    content = fix_dprintf_calls(content)
    
    # Fix fatal calls
    content = fix_fatal_calls(content)
    
    with open(filename, 'w') as f:
        f.write(content)
    
    print(f"Fixed dprintf and fatal calls in {filename}")
