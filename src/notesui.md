# Uniform Abstraction Layer User Interface (UALUI)

The Uniform Abstraction Layer User Interface (UALUI) is a user interface framework designed to provide a consistent and unified experience across different platforms and devices. It abstracts the underlying complexities of various operating systems and hardware, allowing developers to create applications that can run seamlessly on multiple environments without needing to rewrite code for each specific platform.

Admittedly, this ambitious project is still in its early stages, and there are many challenges to overcome. However, the potential benefits of UALUI are significant, as it can greatly reduce development time and costs while improving user experience.

My first step is to create a Uniform Abstraction Layer for NCurses, which my program currently uses, and NotCurses, which I am adding. I believe I have a reasonable understanding of NCurses and NotCurses. Although, I am not fluent with the latter, I am confident that I can assimilate it quickly. The goal is to create a unified interface that allows developers to write code that works with both libraries without having to worry about the underlying differences. Once I get the NotCurses implementation working, I would like to create another abstraction for a GUI library such as GTK or QT.

With your help, I have managed to get past some of the initial hurdles in understanding the intricacies of both NCurses and NotCurses. The next steps involve designing a set of common functions and data structures that can serve as the foundation for the UALUI. This will include defining how to handle input, output, window management, and other essential features in a way that is agnostic to the underlying library.

I have started by creating functional units of code using NotCurses, which will provide capabilities similar to NCurses. My confidence begins to erode however, as I consider the precise mechanics of combining NCurses and NotCurses into a single abstraction layer.

As a specific example, I will begin with initialization. I often use pipes for input and output, so I open a stream from a tty device (tty_fp), and use screen = newterm(nullptr, tty_fp, tty_fp) to initialize NCurses. I will need to call delscreen(screen) to clean up when I am done. Currently, in my program, screen is a global variable, but I would like to avoid using global variables in the UALUI. Instead, I plan to encapsulate the screen management within a context structure that can be passed around as needed. I have created a structure, SIO (screen I/O), which I use to manage the screen state and related resources. I plan to use this structure will hold pointers to the NCurses and NotCurses contexts, as well as any other relevant information needed for screen management. So, perhaps I can emit SIO as a product of ui_init() and use it to manage the screen state throughout the application. This way, I can avoid global variables and maintain a cleaner design.

Should I use SIO, adding a pointer to an NCurses and NotCurses context, or
should I create separate context structures for each library and manage them independently? I am leaning towards using a single SIO structure, as it would simplify the interface and make it easier to manage the screen state. However, I am open to suggestions and would appreciate any insights you may have on this matter.

The next question is, should I attempt to make the underlying libraries (NCurses and NotCurses) completely opaque to the user? Should I implement a set of wrapper functions so that the user interacts only with the UALUI interface, or should I allow users to access the underlying libraries directly if they choose to do so? Making the libraries opaque would provide a cleaner and more consistent interface, but it may limit flexibility for advanced users who want to leverage specific features of NCurses or NotCurses. On the other hand, allowing direct access could lead to potential conflicts and inconsistencies in how the libraries are used.

I could provide two separate libraries for my program, in which identical
functions on the application side would call either the NCurses or NotCurses implementation, depending on which library is being used. This approach would allow users to choose the library they prefer while still providing a consistent interface for common functionality. However, it may also increase the complexity of the codebase and require additional maintenance to ensure that both implementations remain in sync.

Any advice would be greatly appreciated, as I want to ensure that the UALUI is both user-friendly and maintainable. I am also considering the possibility of creating a plugin system that allows users to extend the functionality of the UALUI by adding their own modules or libraries. This could provide additional flexibility and allow for a wider range of use cases.

I need to narrow down my design choices and make some decisions on how to
proceed with the UALUI. I would like to hear your thoughts on the following
questions:

1. Should I use a single SIO structure to manage both NCurses and NotCurses contexts, or should I create separate context structures for each library?

2. Should I make the underlying libraries (NCurses and NotCurses) completely opaque to the user, or should I allow users to access the underlying libraries directly if they choose to do so?

3. Should I provide two separate libraries for my program, allowing users to choose between NCurses and NotCurses, or should I focus on creating a single unified interface that abstracts away the differences between the two libraries?

How should I proceed? Thank you for your time and insights.




