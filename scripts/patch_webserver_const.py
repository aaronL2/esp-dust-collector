# scripts/patch_webserver_const.py
Import("env")
import os, re

TARGET_LIB_PREFIX = "espasyncwebserver"   # case-insensitive startswith
HEADER_REL = os.path.join("src", "ESPAsyncWebServer.h")

# Replace both forms with explicit parentheses:
#   _server->status()  ->  (const_cast<AsyncServer*>(_server))->status()
#   _server.status()   ->  ((const_cast<AsyncWebServer *>(this))->_server).status()
PATTERNS = [
    (re.compile(r"_server\s*->\s*status\s*\(\s*\)"),
     r"(const_cast<AsyncServer*>(_server))->status()"),
    (re.compile(r"_server\s*\.\s*status\s*\(\s*\)"),
     r"((const_cast<AsyncWebServer *>(this))->_server).status()"),
]

def patch_file(path: str) -> bool:
    try:
        with open(path, "r", encoding="utf-8") as f:
            txt = f.read()
    except FileNotFoundError:
        return False

    changed = 0
    new_txt = txt

    for pat, repl in PATTERNS:
        new_txt, n = pat.subn(repl, new_txt)
        changed += n

    if changed == 0:
        return False

    # one-time backup
    try:
        with open(path + ".bak", "x", encoding="utf-8") as b:
            b.write(txt)
    except FileExistsError:
        pass

    with open(path, "w", encoding="utf-8") as f:
        f.write(new_txt)
    print(f"[patch_webserver_const] Patched {changed} occurrence(s): {path}")
    return True

def run():
    project_dir = env["PROJECT_DIR"]
    libdeps_root = os.path.join(project_dir, ".pio", "libdeps")

    changed_any = False
    if os.path.isdir(libdeps_root):
        for envdir in os.listdir(libdeps_root):          # base, station, etc.
            envpath = os.path.join(libdeps_root, envdir)
            if not os.path.isdir(envpath):
                continue
            for name in os.listdir(envpath):
                if name.lower().startswith(TARGET_LIB_PREFIX):
                    header = os.path.join(envpath, name, HEADER_REL)
                    if os.path.isfile(header):
                        changed_any |= patch_file(header)

    if not changed_any:
        print("[patch_webserver_const] Nothing to patch (already patched, not installed yet, or pattern not found).")

run()
