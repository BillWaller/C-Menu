#define __end_pgm                                                              \
    {                                                                          \
        static void end_pgm(void) {                                            \
            destroy_init(init);                                                \
            win_del();                                                         \
            destroy_curses();                                                  \
            restore_shell_tioctl();                                            \
            exit(EXIT_FAILURE);                                                \
        }
