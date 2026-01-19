# StarServices

StarServices - Tournament Anti-Cheat Service Manager for Windows

## Description

StarServices is a professional tool designed to manage and activate Windows services required for competitive gaming anti-cheat verification. It provides a comprehensive console interface for managing critical system services.

## Features

- **Console Interface**: Professional colored console output
- **Service Management**: Activate and verify 7 critical Windows services
- **Admin Verification**: Ensures proper administrator privileges
- **Detailed Status**: Real-time service status reporting
- **Error Handling**: Comprehensive error detection and reporting
- **Visual Feedback**: Color-coded status indicators (Green/Red/Yellow)

## Services Managed

1. PcaSvc - Program Compatibility Assistant
2. PlugPlay - Plug and Play
3. DPS - Diagnostic Policy Service
4. DiagTrack - Connected User Experiences and Telemetry
5. SysMain - Superfetch (Performance Optimization)
6. EventLog - Windows Event Log
7. Sysmon - System Monitor (Sysinternals)

## Requirements

- Windows 10/11 (x64/x86)
- Administrator privileges
- GCC compiler (MinGW-w64) for compilation

## Compilation

Using GCC (MinGW-w64):

```bash
gcc -o StarServices.exe StarServices.c -ladvapi32 -lkernel32 -static
```

Or for smaller binary:

```bash
gcc -o StarServices.exe StarServices.c -ladvapi32 -lkernel32
```

## Usage

1. Download or compile StarServices.exe
2. Right-click on the executable
3. Select "Run as administrator"
4. The tool will automatically verify and activate all required services
5. Review the service status report
6. Press any key to exit

## License

Copyright (c) 2025-2030 xpe.nettt

## Author

xpe.nettt
