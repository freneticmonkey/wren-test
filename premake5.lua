
workspace "wren-test"
   configurations { "Debug", "Release" }

   filter "configurations:Debug"
      symbols "On"
      defines { "_DEBUG" }

   filter "configurations:Release"
      optimize "On"
      defines { "NDEBUG" }
   
   filter { "system:windows" }
      defines {
         "TINY_WINDOWS"
      }

   filter { "system:macosx" }
      defines {
         "TINY_MACOS"
      }

   filter { "system:linux" }
      -- buildoptions { "-std=c99" }
      defines {
         "TINY_LINUX"
      }

solution "wren-test"
   filename "wren-test"
   location "projects"

   startproject "wren-test"

project "wren"
   kind "StaticLib"
   language "C"
   targetdir( "build" )

   sysincludedirs {
      "ext/wren/include",
      "ext/wren/optional",
      "ext/wren/vm"
   }

   files { 
      "ext/wren/**.h",
      "ext/wren/**.c"
   }

project "wren-test"
   kind "ConsoleApp"
   language "C"
   targetdir( "build" )

   filter {}
      links {
         "wren"
      }

      libdirs {
         "build"
      }

      sysincludedirs {
         "ext/wren/include",
         "ext/wren/vm",
      }

      files { 
         "*.h",
         "*.c"
      }
   
   filter "configurations:Debug"
      debugdir "."
      debugargs { "script.wren" }
   
   configuration { "linux", "gmake" }
      buildoptions { "-std=c99" }
      disablewarnings { 
         "-Wmacro-redefinition"
      }

   -- filter { "system:windows" }
   --    defines {
   --       "_CRT_SECURE_NO_WARNINGS",
   --       "TINY_WINDOWS"         
   --    }
   --    disablewarnings { 
   --       "4005"
   --    }

   -- filter { "system:macosx"}
   --    defines { 
   --       "TINY_MACOS"
   --    }
   --    links {
   --       "Cocoa.framework",
   --       "Metal.framework",
   --       "MetalKit.framework",
   --       "QuartzCore.framework",
   --       "OpenGL.framework",
   --       "IOKit.framework",
   --       "c++"
   --    }

   -- filter {
   --    "system:macosx",
   --    "files:tiny/src/main.c"
   -- }
   --    compileas "Objective-C"
      