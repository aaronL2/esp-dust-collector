# scripts/version_stamp.py
from datetime import datetime
from SCons.Script import Import
Import("env")

stamp = datetime.now().strftime("%Y%m%d.%H%M%S")  # YYYYMMDD.HHMMSS
version_str = f'0.{stamp}'

# Inject as a quoted C string macro
env.Append(CPPDEFINES=[('FW_VERSION', f'\"{version_str}\"')])
print(f"[version_stamp] FW_VERSION={version_str}")
