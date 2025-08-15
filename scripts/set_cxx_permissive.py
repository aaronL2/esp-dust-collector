# scripts/set_cxx_permissive.py
Import("env")

# Make sure -fpermissive is NOT on any global flags first
for var in ("CFLAGS", "CCFLAGS", "CXXFLAGS"):
    try:
        env[var].remove("-fpermissive")
    except (ValueError, KeyError):
        pass

# Add -fpermissive ONLY for C++ compilation
env.Append(CXXFLAGS=["-fpermissive"])
