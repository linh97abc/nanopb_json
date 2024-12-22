# Nanopb JSON Project

## Overview

This project provides functionality to encode and decode Protocol Buffers messages to and from JSON format using Nanopb. It includes functions to handle JSON strings and convert them into Protocol Buffers structures, as well as the reverse process.

## Features

- Encode Protocol Buffers structure to JSON strings.
- Decode JSON strings into Protocol Buffers structure.
- Support for various data types and structures defined in Protocol Buffers.

## Getting Started

### Prerequisites

- C/C++ compiler
- CMake (optional, for building the project)
- Protocol Buffers compiler (protoc)


### Building the Project


### Usage

#### Encoding a Protocol Buffers Message to JSON

```c
#include <pb/json.h>
#include "your_protobuf_message.pb.h"

void encode_example() {
    YourMessage msg = YourMessage_init_zero;
    // Populate your message fields
    char json_output[1024];
    int json_len = pbjson_encode(json_output, sizeof(json_output), YourMessage_fields, &msg);
    if (json_len > 0) {
        printf("Encoded JSON: %s\n", json_output);
    } else {
        printf("Encoding failed with error code: %d\n", result);
    }
}
```
#### Decoding a JSON String to a Protocol Buffers Message

```c
#include <pb/json.h>
#include "your_protobuf_message.pb.h"

void decode_example() {
    const char *json_input = "{\"field1\": \"value1\", \"field2\": 123}";
    YourMessage msg = YourMessage_init_zero;
    int result = pbjson_decode(json_input, YourMessage_fields, &msg);
    if (result == 0) {
        // Use the decoded message
    } else {
        printf("Decoding failed with error code: %d\n", result);
    }
}
```

### Acknowledgements

- Nanopb - Protocol Buffers for Embedded Systems
- Protocol Buffers - Google's data interchange format