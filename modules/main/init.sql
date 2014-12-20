CREATE TABLE functions (
	name		text		UNIQUE NOT NULL,
	params		int			DEFAULT 1,
	doctype		text		DEFAULT 'text/xml',
	param_type	text		DEFAULT 'QUERY_STRING',
	templt_path	text		DEFAULT '/',
	description	text		DEFAULT 'no description',
	cert_key	text		DEFAULT 'empty',
	enabled		boolean		NOT NULL DEFAULT 'off'
);

CREATE TABLE connect(
	proc_name	text				NOT NULL,
	pid_init	int				NOT NULL,
	server_name	text				DEFAULT 'unknown',
	server_info	text				DEFAULT 'unknown',
	server_soft	text				DEFAULT 'unknown',
	server_addr	inet				NOT NULL,
	status		boolean				DEFAULT 'on',
	time_init	timestamp with time zone	DEFAULT CURRENT_TIMESTAMP,
	time_close	timestamp with time zone	DEFAULT NULL,
	PRIMARY KEY (time_init, pid_init, server_addr)
);

--=======================================================================================================================--
--
--			Initialization of all available functions
--
--========================================================================================================================
--DROP FUNCTION initialize(text);
CREATE OR REPLACE FUNCTION initialize(new_proc_name text, new_pid int, new_server_name text, new_server_info text, new_server_soft text, new_server_addr inet) RETURNS setof functions AS $$
DECLARE
	tmp_table functions;
BEGIN
	INSERT INTO connect (proc_name, pid_init, server_name, server_info, server_soft, server_addr) VALUES(new_proc_name, new_pid, new_server_name, new_server_info, new_server_soft, new_server_addr);
	FOR tmp_table IN EXECUTE
		'SELECT * FROM functions WHERE enabled = ''on'''
	LOOP
		RETURN next tmp_table;
	END LOOP;
	RETURN;
END $$ LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION uninitialize(old_pid int, old_server_addr inet) returns void AS $$
BEGIN
	UPDATE connect SET status = 'off' WHERE pid_init = old_pid AND server_addr = old_server_addr;
	RETURN;
END $$ LANGUAGE 'plpgsql';
