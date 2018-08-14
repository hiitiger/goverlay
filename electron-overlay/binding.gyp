{
    "targets": [
        {
            "target_name": "electron_overlay",
            "cflags!": ["-fno-exceptions"],
            "cflags_cc!": ["-fno-exceptions"],
            "include_dirs": [
                "<!@(node -p \"require('node-addon-api').include\")"
            ],
            'defines': ['NAPI_DISABLE_CPP_EXCEPTIONS', 'UNICODE'],
            "sources": ["./src/main.cc"],
        }
    ]
}
