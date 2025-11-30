#include <nvim/api/buffer.h>
#include <nvim/api/tabpage.h>
#include <nvim/api/vim.h>
#include <nvim/api/window.h>
#include <nvim/msgpack_rpc/helpers.h>
#include <nvim/msgpack_rpc/server.h>

// Example function to print a message in Neovim
void hello_world() {
    nvim_out_write("Hello from C plugin!\n", strlen("Hello from C plugin!\n"));
}

// Entry point for the plugin
int main(int argc, char **argv) {
    // Initialize Neovim
    nvim_init();

    // Register your function
    nvim_register_function("HelloWorld", hello_world);

    // Start the Neovim event loop
    nvim_event_loop();

    return 0;
}

// gcc -shared -o myplugin.so -fPIC myplugin.c -I/path/to/neovim/include
//
// lua require('ffi').load('/path/to/myplugin.so')
//
// :call HelloWorld()
// 5. Tips for Development
// Use msgpack-rpc for more advanced communication between Neovim and your
// plugin. Consider using libuv for asynchronous tasks. Debugging can be done
// using tools like gdb or lldb. While C plugins are powerful, they require more
// effort compared to Lua. If you're looking for simplicity and flexibility, Lua
// might be a better choice for most Neovim plugins. However, for
// performance-critical tasks, C is an excellent option!
//
