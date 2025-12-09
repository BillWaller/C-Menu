# CMENU ROADMAP FOR DEVELOPERS

This document outlines the planned features and improvements for CMENU, aimed at developers who contribute to the project. The roadmap is subject to change based on community feedback and evolving project needs.

## Short-Term Goals (Next 3-6 Months)

- **Enhanced Plugin System**: Improve the plugin architecture to allow for easier
  integration of third-party plugins.
- **Documentation Update**: Revise and expand the developer documentation to include
  more examples and best practices.
- **Formatting, Validation, and Database Support for Fields:** Add support for
  various field types, including text, numeric, date, and dropdown fields. Implement validation rules and integrate with databases for dynamic data retrieval and storage.
- **Implement a Modern build system** such as CMake or Meson to replace the
  existing Makefile-based build process. This will improve cross-platform
  compatibility, simplify dependency management, and enhance the overall
  build process for developers.
- **Extend Mouse Support:** Currently, menu items can be selected using the mouse,
  but scrolling is not supported. Implement full mouse support, including scrolling
  and other interactive elements.
- **Performance Optimization**: Identify and optimize performance bottlenecks in
  the codebase.
- **Continue Code Refactoring**: Much has been done, but much remains. The
  objective is not only to make the codebase cleaner and more maintainable, but
  also to facilitate migration to other languages such as Rust, Go, Java, or C++
  in the future.
- **Unit Testing Framework**: Implement a more robust unit testing framework to
  ensure code quality.
- **Bug Fixes**: Address known bugs and issues reported by the community.

## Medium-Term Goals (6-12 Months)

- **Consolidate and modularize screen IO** so that the NCurses API calls are
  isolated to a single module. This will facilitate future support for
  alternative screen handling API libraries such as those used by Vim,
  NeoVim, Emacs (Elisp), and QT, among others.
- **Support for Threaded Operations**: Improve the handling of threaded operations
  to ensure stability and performance.
- **Implement Interprocess Communication (IPC)**: Provide serialization and
  deserialization mechanisms using MsgPack (Neovim) or Tokio (Rust) to
  facilitate communication between CMENU and other applications or services.
- **Add Support for Theming:** Introduce a theming system to allow users to customize
  the appearance of CMENU.
- **Refactor Configuration Management**: Redesign the configuration management
  system using JSON or YAML to improve readability and ease of use. This will also
  facilitate integration with other tools and languages.
- **Implement a Scripting Interface**: Provide a scripting interface (e.g.,
  Lua or Python) to allow users to automate tasks and extend functionality.
- **New UI Components**: Develop new user interface components to enhance the user
  experience.
- **API Expansion**: Expand the existing API to provide more functionality for
  developers.
- **CLDR-based Internationalization**: Add support for multiple languages to
  make CMENU accessible to a broader audience.
- **Continuous Integration**: Set up a continuous integration pipeline to automate
  testing and deployment.
- **Community Engagement**: Foster a stronger developer community through forums,
  webinars, and hackathons.

## Long-Term Goals (12+ Months)

- **Modular Architecture**: Transition to a more modular architecture to facilitate
  easier maintenance and scalability.
- **Advanced Analytics**: Integrate advanced analytics features to provide insights
  into user behavior and application performance.
- **Cross-Platform Support**: Expand support for additional platforms, languages,
  and operating systems.
- **AI Integration**: Explore the integration of AI and machine learning capabilities
  to enhance functionality.
- **Sustainability Initiatives**: Implement practices to ensure the long-term
  sustainability of the project, including funding and resource management.

## Conclusion

This roadmap serves as a guide for developers contributing to CMENU. We encourage community involvement and feedback to help shape the future of the project. Regular updates will be provided to keep everyone informed of progress and changes to the roadmap.
