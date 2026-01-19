# Release Guide

This guide explains how to create releases and compile StarServices.exe automatically using GitHub Actions.

## Quick Start: Create a Release

### Method 1: Push a Git Tag (Recommended)

```bash
# Clone the repository
git clone https://github.com/xpe-hub/StarServices.git
cd StarServices

# Make your changes to StarServices.c
# ...

# Commit your changes
git add StarServices.c
git commit -m "Update version or fix bug"

# Create a version tag (use semantic versioning: vMAJOR.MINOR.PATCH)
git tag v1.0.0

# Push the tag to GitHub (this triggers the build!)
git push origin v1.0.0
```

### Method 2: GitHub Actions Manual Trigger

1. Go to https://github.com/xpe-hub/StarServices/actions
2. Click on "Build and Release StarServices"
3. Click "Run workflow"
4. The compiled .exe will be available as an Artifact (not a Release)

## Version Format

Use semantic versioning:
- `v1.0.0` - First stable release
- `v1.0.1` - Bug fix
- `v1.1.0` - New features
- `v2.0.0` - Major version with breaking changes

## What Happens When You Push a Tag

1. **GitHub Actions starts** automatically
2. **Windows runner** spins up
3. **MinGW-w64** is installed
4. **StarServices.c** is compiled with:
   ```bash
   gcc -o StarServices.exe StarServices.c -ladvapi32 -lkernel32 -static -s -O2
   ```
5. **Verification** runs:
   - Checks file exists
   - Validates PE header (MZ)
   - Confirms 64-bit executable
6. **Release is created** on GitHub with StarServices.exe attached
7. **Attestation** is generated for security verification

## Download the Compiled Executable

1. Go to https://github.com/xpe-hub/StarServices/releases
2. Click on the latest release (or specific version)
3. Download "StarServices.exe" from Assets

## Run StarServices

```cmd
# Navigate to where you downloaded the exe
cd Downloads

# Run as Administrator (required!)
StarServices.exe

# Or right-click > Run as administrator
```

## Automated Features

### Build Verification
- File existence check
- PE header validation (MZ magic bytes)
- Architecture detection (x64/x86)

### Release Generation
- Automatic release notes from commits
- Build information included
- Secure download links

### Artifact Attestation
- Cryptographic provenance
- Verifiable build source
- Tamper-evidence

## Troubleshooting

### Build Fails

**Error: "gcc not found"**
- Wait for MSYS2 setup to complete
- Check GitHub Actions logs

**Error: "StarServices.c not found"**
- Ensure file is in repository root
- Check git add && git commit

**Error: "Invalid PE header"**
- File corruption during upload
- Check antivirus isn't interfering

### Release Not Created

- Ensure you're pushing a tag (`git tag v*`)
- Check Actions tab for build status
- Verify permissions: Settings > Actions > General > Workflow permissions

### exe Doesn't Run on Windows

**"DLL not found" error:**
- Make sure `-static` flag is used (included in workflow)
- Download fresh release from GitHub

**"Access denied":**
- Run as Administrator
- Check User Account Control (UAC) settings

## CI/CD Pipeline Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    GitHub Actions                            │
├─────────────────────────────────────────────────────────────┤
│  Trigger: Push tag v*                                       │
│  Runner: windows-latest                                     │
├─────────────────────────────────────────────────────────────┤
│  Step 1: Checkout                                           │
│  Step 2: Setup MinGW-w64 (MSYS2)                            │
│  Step 3: Compile with GCC (-static flag)                    │
│  Step 4: Verify PE header & file size                       │
│  Step 5: Generate attestation                               │
│  Step 6: Upload artifact                                    │
│  Step 7: Create GitHub Release                              │
│  Step 8: Verify release assets                              │
└─────────────────────────────────────────────────────────────┘
```

## Security

- All builds are reproducible from source
- Attestation provides build provenance
- Release assets are cryptographically signed by GitHub
- Audit trail available in Actions tab

## Support

- Issues: https://github.com/xpe-hub/StarServices/issues
- Actions: https://github.com/xpe-hub/StarServices/actions

## Copyright

Copyright (c) 2025-2030 xpe.nettt
