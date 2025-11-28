local ipc_server = {}
      ipc_server.LOAD_SO = true
      ipc_server.SO_PREFIX = "vlm_"

function ipc_server.create_socket() end
function ipc_server.accept_client() end
function ipc_server.recv() end
function ipc_server.send() end
function ipc_server.close() end

function ipc_server.epoll_create() end
function ipc_server.epoll_wait() end
function ipc_server.epoll_add() end

return ipc_server