-- remote_server.ads - Main package for use by NymphRPC clients (Spec).
--
-- 2017/07/01, Maya Posch
-- (c) Nyanko.ws


package NymphRemoteServer is
	
	function init(logger : in LogFunction, level : in integer, timeout: in integer) return Boolean;
	function sync(handle : in integer, result : out string) return Boolean;
	procedure setLogger();
	function shutdown() return Boolean;
	function connect(host : in string, port : in integer, handle : out integer, data : in out, 
						result : out string);
	function connect(url : in string, handle : out integer, 
	
private
	--
	
end NymphRemoteServer;
