# View Field Development Guide

### 2025-11-30T13:26:32-06:00

- Note: This document is a work in progress and may contain incomplete
  sections or ideas that are not fully fleshed out. It is intended to provide
  guidance for developers working on the View field of the application.

- Implementing horizontal scrolling using pads was far easier than I expected.
  See the code in view.c for details.

## Line Truncation

View currently truncates output beyond the number of window columns. This is not
ideal as many applications require the ability to view long text lines. Two
features are needed to address this limitation:

### Horizontal Scrolling

There are two ways to implement horizontal scrolling. One is to add a horizontal
offset to the view structure and adjust the rendering functions to take this
offset into account. The other is to implement a separate horizontal scrollbar
that allows the user to scroll left and right. The first approach is simpler to
implement, while the second approach provides a better user experience.

1. View currently has a horizontal offset variable that is not used. This variable
   can be used to track the horizontal scroll position. This introduces several
   challenges, not the least of which is rendering highlighted search results
   that may be partially off-screen.

2. Using NCurses pad functionality could be another approach. Pads allow for a larger
   virtual window that can be scrolled both vertically and horizontally. This
   would require significant changes to the rendering logic but would provide a
   robust solution for handling large text areas. The problem with this approach
   is that NCurses pads have their own limitations and may not integrate well
   with the existing view architecture. The author is not familiar enough with
   NCurses pads to evaluate this option fully at this time.

### Horizontal Wrapping

Another approach to handling long lines is to implement horizontal wrapping. This
involves breaking long lines into multiple shorter lines that fit within the
window width. This can be done by modifying the rendering functions to check the
length of each line and inserting line breaks as needed. This approach has the advantage of keeping all text visible without the need for scrolling. However, it can make it more difficult to read and navigate through the text, especially if there are many long lines.

### Implementation Considerations

When implementing either horizontal scrolling or wrapping, several factors need to
be considered:

- Performance: Both approaches may impact performance, especially with large
  files. Careful optimization may be needed to ensure smooth scrolling and
  rendering.
- User Experience: The chosen approach should provide a good user experience,
  allowing users to easily navigate and read long lines of text.
- Compatibility: The implementation should be compatible with existing features
  such as search highlighting and line numbering.
- Configuration: It seems intuitive that users must be allowed to choose between horizontal scrolling and wrapping, not only through configuration options, but dynamically and on the fly.
- Testing: Thorough testing is needed to ensure that the implementation works
  correctly across different terminal types and configurations.

## Future Enhancements

Beyond addressing line truncation, several future enhancements could further improve
View's usability.

### Syntax Highlighting

Implementing syntax highlighting would greatly enhance the readability of code
files. This could be achieved by integrating a syntax highlighting library or
developing a custom solution that recognizes common programming languages and
applies appropriate color schemes.

### Plugin System

A plugin system would allow users to extend View's functionality by adding
custom features. This could include support for additional file formats, custom
commands, or integration with other tools.

### Improved Search Functionality

View already supports regular expressions and highlights all match instances on a page,
but there is room for improvement. For example, it would be helpful in some instances to display an occurrence counter with the total number of matches.
