#pragma once

// These get injected by PlatformIO build flags or the extra_script (below)
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

// Helpers (const char* so no heap)
namespace VersionInfo {
  inline const char* firmware()   { return FW_VERSION; }
  inline const char* git()        { return GIT_HASH; }
  inline const char* buildTime()  { return BUILD_TIME; }
  inline const char* fs()         { return FS_VERSION; }
}
