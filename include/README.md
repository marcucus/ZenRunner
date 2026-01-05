# Include Directory

This directory contains public header files and shared type definitions used across multiple modules.

## Purpose

- **Public APIs**: Headers exposing interfaces to other modules
- **Shared Types**: Common data structures, enums, and constants
- **Forward Declarations**: Minimize compilation dependencies

## Organization

Headers should be organized by module or functionality:
- `core/`: Public interfaces from the core module
- `platform/`: Platform abstraction layer interfaces
- `types/`: Common type definitions and enums

## Guidelines

- Keep headers minimal and focused
- Use forward declarations where possible
- Document all public APIs with doxygen-style comments
- Avoid implementation details in headers
- Use include guards or `#pragma once`
