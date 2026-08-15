View reads and formats text from lines terminated by line feed characters.
It parses ANSI SGR sequences, applies attributes and colors, and highlights matched search terms. Line and word length are tracked and lines longer than the page width are wrapped appropriately. With NCurses, View constructs arrays of complex characters (cchar_t) with attributes and colors. These arrays are written to a pad, which can be displayed as wrapped or horizontally scrolled lines. This is all working very well. 

The 16-byte Notcurses nccell contains a 32 bit gcluster, which can be extended through an EGC (Extended Grapheme Cluster) pool, an architectural component of the Notcurses plane. This robustly handles Unicode characters and their attributes, but it presents some challenges when adapting existing code that was designed for NCurses.

Because my current implementation relies heavily on the ability to manipulate arrays of complex character cells, I will create a new data structure that can hold the necessary information for each cell, including the character, its attributes, and any color information. It wouldn't be a good idea to manipulate the nccells directly, so I will use the Notcurses API to create and manage these cells.

I could experiment with different approaches, but given the complexity of my existing implementation and the need to maintain performance, I want to ensure that I am taking the most efficient path forward. Any suggestions or insights from those who have experience with Notcurses would be greatly appreciated.
