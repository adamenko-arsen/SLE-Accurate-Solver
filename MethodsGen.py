#!/usr/bin/env python

import os
import re

# Function to extract method definitions from the tags file
def parse_tags(tags_file):
    methods = {}
    with open(tags_file, 'r') as file:
        for line in file:
            parts = line.split('\t')
            if len(parts) > 3:
                name = parts[0]
                info = parts[3:]
                class_match = re.search(r'class:(\w+)', ' '.join(info))
                if class_match:
                    class_name = class_match.group(1)
                    if class_name not in methods:
                        methods[class_name] = set()
                    methods[class_name].add(name)
    return methods

# Function to find method calls in source files
def find_method_calls(source_directory):
    method_calls = set()
    method_call_pattern = re.compile(r'(\w+)::(\w+)\s*\(')
    for root, _, files in os.walk(source_directory):
        for file in files:
            if file.endswith('.cpp') or file.endswith('.h'):
                with open(os.path.join(root, file), 'r') as f:
                    content = f.read()
                    for match in method_call_pattern.finditer(content):
                        class_name, method_name = match.groups()
                        method_calls.add((class_name, method_name))
    return method_calls

# Main logic
def main(source_directory, tags_file):
    methods = parse_tags(tags_file)
    method_calls = find_method_calls(source_directory)

    used_methods = []
    for class_name, method_name in method_calls:
        if class_name in methods and method_name in methods[class_name]:
            used_methods.append((class_name, method_name))

    for class_name, method_name in used_methods:
        print(f'Used Method - Class: {class_name}, Method: {method_name}')

# Usage
source_directory = 'Sources'
tags_file = 'tags'
main(source_directory, tags_file)
