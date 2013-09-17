javahidapi
==========

A fork of the javahidapi library made by Codeminders

[Google code project here](https://code.google.com/p/javahidapi/)

I've just rearranged the native lib layout a bit to make it work with leiningen and Clojure

```bash
native/
├── linux
│   ├── arm
│   │   └── libhidapi-jni-32.so
│   ├── x86
│   │   └── libhidapi-jni-32.so
│   └── x86_64
│       └── libhidapi-jni-64.so
├── macosx
│   ├── x86
│   │   └── libhidapi-jni-32.jnilib
│   └── x86_64
│       └── libhidapi-jni-64.jnilib
└── windows
    ├── x86
    │   └── hidapi-jni-32.dll
    └── x86_64
        └── hidapi-jni-64.dll
```
