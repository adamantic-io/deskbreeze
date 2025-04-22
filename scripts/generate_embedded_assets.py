
import re
from pathlib import Path

index_path = Path(__file__).parent / "../qt-app/www/index.html"
asset_dir = Path(__file__).parent / "../qt-app/www/assets"
embedded_path = Path(__file__).parent / "../qt-app/embedded_assets.cpp"

# Extract the JS filename from the index.html
index_html = index_path.read_text()
match = re.search(r'src="/assets/([^"]+)"', index_html)
if not match:
    raise RuntimeError("No JS bundle found in index.html")

js_filename = match.group(1)
print(f"Detected JS filename: {js_filename}")

# Write embedded_assets.cpp
embedded_path.write_text(f'''
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#include "incbin.h"

INCBIN(index_html, "../qt-app/www/index.html");
INCBIN(main_js, "../qt-app/www/assets/{js_filename}");
''')
