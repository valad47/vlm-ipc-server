return {
       pkgname = "ipc-server",
       pkgver = 1,
       luau_files = {"ipc-server.luau"},
       c_files = {"server.c"},
       c_args = {"-ggdb"}
}
