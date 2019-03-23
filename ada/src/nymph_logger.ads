-- nymph_logging.ads - Main package for use by NymphRPC clients (Spec).
--
-- 2017/07/01, Maya Posch
-- (c) Nyanko.ws

type LogFunction is not null access procedure (level: in integer, text: in string);



