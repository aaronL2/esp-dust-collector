#pragma once

// These are injected by scripts/version_stamp.py (with safe fallbacks)
#ifndef FW_VERSION
  #define FW_VERSION "0.0.0"
#endif
#ifndef GIT_HASH
  #define GIT_HASH "nogit"
#endif
#ifndef BUILD_TIME
  #define BUILD_TIME "unknown"
#endif
#ifndef FS_VERSION
  #define FS_VERSION "unknown"
#endif

namespace Version {
  inline const char* firmware()  { return FW_VERSION; }
  inline const char* git()       { return GIT_HASH; }
  inline const char* buildTime() { return BUILD_TIME; }
  inline const char* fs()        { return FS_VERSION; }
}
