{
  "targets": [
    {
      "target_name": "darling",
      "sources": [
        "src/darling_node.cc",
        "../core/src/darling.c",
        "../core/src/platform/win32/window_win32.c"
      ],
      "include_dirs": [
        "../core/include",
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "cflags_cc": [ "-std=c++20" ],
      "defines": [
        "NAPI_CPP_EXCEPTIONS",
        "DARLING_BUILD",
        "NAPI_DISABLE_CPP_EXCEPTIONS"   
      ],
      "conditions": [
        ["OS=='win'", {
          "msvs_settings": {
            "VCCLCompilerTool": {
              "ExceptionHandling": 1,
              "AdditionalOptions": [
                "/std:c++20",
                "/utf-8"
              ],
              "WarningLevel": 3,
              "DisableSpecificWarnings": [ "4530" ]
            }
          },
          "libraries": [ "-luser32", "-lgdi32" ]
        }]
      ]
    }
  ]
}