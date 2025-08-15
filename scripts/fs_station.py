# pyright: reportUndefinedVariable=false
Import("env")
import os, shutil, stat
from pathlib import Path

def _on_rm_error(func, path, excinfo):
    # Make read-only files writable, then retry
    os.chmod(path, stat.S_IWRITE)
    func(path)

proj = Path(env.subst("$PROJECT_DIR"))
src = proj / "station" / "data"
dst = proj / "data"

print(f"[fs_station] preparing FS from: {src}")
if dst.exists():
    shutil.rmtree(dst, onerror=_on_rm_error)
if src.exists():
    shutil.copytree(src, dst)
else:
    os.makedirs(dst, exist_ok=True)
    print("[fs_base] WARNING: base/data doesn't exist; created empty data/")
