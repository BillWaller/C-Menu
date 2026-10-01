#!/usr/bin/env python3
import sys
import json
import re
import urllib.request

OLLAMA_API_URL = "http://localhost:11434/api/generate"
MODEL_NAME = "mistral"  # Change to your preferred local model (e.g., qwen2.5-coder)

def ask_ollama(text, target_lang):
    if not text.strip():
        return ""
        
    prompt = (
        f"You are a professional software localization tool translating text for a C terminal utility.\n"
        f"Translate the following string into fluent {target_lang}.\n"
        f"CRITICAL RULES:\n"
        f"1. Preserve all printf specifiers exactly as they are (e.g., %s, %d, %x).\n"
        f"2. Keep layout spaces, escape codes (like \\n or \\t) identical.\n"
        f"3. Output ONLY the raw translated string. Do not include quotes, markdown, or explanations.\n\n"
        f"String to translate: \"{text}\"\n"
        f"Translation:"
    )
    
    data = {
        "model": MODEL_NAME,
        "prompt": prompt,
        "stream": False,
        "options": {"temperature": 0.1}
    }
    
    try:
        req = urllib.request.Request(
            OLLAMA_API_URL, 
            data=json.dumps(data).encode('utf-8'), 
            headers={'Content-Type': 'application/json'}
        )
        with urllib.request.urlopen(req) as response:
            res_data = json.loads(response.read().decode('utf-8'))
            return res_data['response'].strip().strip('"')
    except Exception as e:
        print(f"Error calling Ollama: {e}", file=sys.stderr)
        return ""

def split_to_po_format(text, indent=0):
    """Splits a long translated string into multi-line PO syntax if it contains real newlines."""
    # Split by actual literal '\n' characters embedded in the translation string
    parts = text.split("\\n")
    if len(parts) == 1:
        return f'"{text}"\n'
    
    out = '""\n'
    for i, part in enumerate(parts):
        if i < len(parts) - 1:
            out += f'"{part}\\n"\n'
        elif part: # Append the trailing segment if it isn't empty
            out += f'"{part}"\n'
    return out

def parse_and_translate_po(filepath, target_lang):
    print(f"Translating {filepath} into {target_lang} using Ollama...")
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    # Regex block to match entire structural PO translation blocks, capturing multi-lines cleanly
    entry_pattern = re.compile(
        r'(msgid\s+(?:""\s*\n)?(?:".*"\s*\n)*)\s*(msgstr\s+(?:""\s*\n)?(?:".*"\s*\n)*)'
    )

    def replace_entry(match):
        msgid_block = match.group(1)
        msgstr_block = match.group(2)

        # Extract content inside quotes
        msgid_strings = re.findall(r'"(.*)"', msgid_block)
        msgstr_strings = re.findall(r'"(.*)"', msgstr_block)

        full_msgid = "".join(msgid_strings)
        full_msgstr = "".join(msgstr_strings)

        # Only process if the ID exists and the translation string is currently completely empty
        if full_msgid and not full_msgstr:
            translated = ask_ollama(full_msgid, target_lang)
            print(f"  '{full_msgid}'\n   -> '{translated}'")
            
            # Format the output matching PO line regulations
            formatted_msgstr = "msgstr " + split_to_po_format(translated)
            return f"{msgid_block}{formatted_msgstr}"
        
        return match.group(0)

    updated_content = entry_pattern.sub(replace_entry, content)

    with open(filepath, 'w', encoding='utf-8') as f:
        f.write(updated_content)

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: ./translate_po.py <path_to_po_file> <target_language>")
        sys.exit(1)
    parse_and_translate_po(sys.argv[1], sys.argv[2])

