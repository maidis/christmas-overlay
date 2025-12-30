# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles/ChristmasOverlay_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/ChristmasOverlay_autogen.dir/ParseCache.txt"
  "ChristmasOverlay_autogen"
  )
endif()
