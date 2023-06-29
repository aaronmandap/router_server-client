package.path = package.path .. ";/etc/tr069/?.lua"
tr069 = require("tr069")
--require("uci")

local parameter_list = {}

local function get_value(param)
    local uri = param:gsub("InternetGatewayDevice[.]X_SC_MyServer[.]","")
    uri = uri:lower()
    tr069.debug_log("Get value = "..uri)
    return tr069.get_uci("my_server_config", "my_server_section", uri)
end

local function set_value(param, value)
    local uri = param:gsub("InternetGatewayDevice[.]X_SC_MyServer[.]","")
    uri = uri:lower()
    return tr069.set_uci("my_server_config", "my_server_section", uri, value)
end

local function request_log(param)
    local log_f = io.popen("/sbin/logread -e router_server | sed \'s/daemon.info router_server\[[0-9]*\]//g\'")
    if not log_f then
        error(cwmp_error.request_denied)
    end
    local log = log_f:read("*a")
    log_f:close()
    return log
 end

parameter_list["InternetGatewayDevice.X_SC_MyServer.Enable"] = {get_value, set_value}
parameter_list["InternetGatewayDevice.X_SC_MyServer.RequestLog"] = {request_log, set_value} 

local function register_parameters()
    return tr069.get_parameter_list(parameter_list)
end

local function set_parameter(uri, value)
    return tr069.set_node(parameter_list, uri, value)
end

local function get_parameter(uri)
    return tr069.get_node(parameter_list, uri)
end

local function sync()
end

return {
    node = "InternetGatewayDevice.X_SC_MyServer.",
    parameters = register_parameters,
    get = get_parameter,
    set = set_parameter,
    sync = sync
} 
