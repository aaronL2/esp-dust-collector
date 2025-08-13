# scripts/version_stamp.py
from datetime import datetime
from SCons.Script import Import
import subprocess

Import("env")

stamp = datetime.now().strftime("%Y%m%d.%H%M%S")  # YYYYMMDD.HHMMSS
fw = f'0.{stamp}'

git_hash = "nogit"
try:
    git_hash = subprocess.check_output(
        ["git", "rev-parse", "--short", "HEAD"], stderr=subprocess.DEVNULL
    ).decode().strip()
except Exception:
    pass

env.Append(CPPDEFINES=[
    ('FW_VERSION', f'\\"{fw}\\"'),
    ('GIT_HASH',   f'\\"{git_hash}\\"'),
    ('BUILD_TIME', f'\\"{stamp}\\"'),
    # Optionally define FS_VERSION here too if you version your /data image
])
print(f"[version_stamp] FW_VERSION={fw} GIT_HASH={git_hash} BUILD_TIME={stamp}")
