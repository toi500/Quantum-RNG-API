{
  "targets": [{
    "target_name": "quantum_rng",
    "sources": [ 
      "src/binding.cc",
      "src/quantum_rng/quantum_rng.c"
    ],
    "include_dirs": [
      "<!@(node -p \"require('node-addon-api').include\")",
      "src/quantum_rng",
      "src/common",
      "src"
    ],
    "defines": [ 
      "NAPI_VERSION=8"
    ],
    "cflags!": [ "-fno-exceptions" ],
    "cflags_cc!": [ "-fno-exceptions" ],
    "cflags": [
      "-O3",
      "-fPIC",
      "-march=native",
      "-Wall",
      "-Wextra"
    ],
    "conditions": [
      ['OS=="linux"', {
        "cflags": [
          "-std=c++17"
        ],
        "cflags_cc": [
          "-std=c++17"
        ]
      }]
    ]
  }]
}
