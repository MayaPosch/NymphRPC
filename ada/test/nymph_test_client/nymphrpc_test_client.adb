-- nymph_test_client.ada - Test client application using the NymphRPC library.
	
--	Revision 0
	
--	Features:
				- 
				
--	Notes:
				-
				
--	2019/04/04, Maya Posch	: Initial version.
--	(c) Nyanko.ws

with Ada.Text_IO;
use Ada.Text_IO;

with NymphRemoteServer;

procedure logFunction(level: in integer, text: in string)
begin
	-- 
	put_line(level'Image & " - " & text);
end logFunction;


procedure Main is
begin
	-- Just connect to the remote server and disconnect afterwards.
	Integer timeout = 5000;
	string result;
	if NymphRemoteServer.connect() not true then
		put_line("Connecting to remote server failed: " & result);
		NymphRemoteServer.disconnect(handle, result);
		NymphRemoteServer.shutdown();
		return;
	end if;
	
	-- Send message and wait for response.
	-- TODO: 
	
	-- Shutdown.
	NymphRemoteServer.disconnect(handle, result);
	NymphRemoteServer.shutdown();	
end Main;
