--TEST--
Test for MongoLog
--SKIPIF--
<?php if (!MONGO_STREAMS) { echo "skip This test requires streams support"; } ?>
<?php require "tests/utils/replicaset.inc"; ?>
--FILE--
<?php
require "tests/utils/server.inc";
function error_handler($code, $message)
{
	echo $message, "\n";
}

set_error_handler('error_handler');

MongoLog::setModule(MongoLog::ALL);
MongoLog::setLevel(MongoLog::ALL);

$config = MongoShellServer::getReplicasetInfo();
$m = new MongoClient("mongodb://" . $config["hosts"][0], array("replicaSet" => $config["rsname"]));

MongoLog::setModule(0);
MongoLog::setLevel(0);
?>
--EXPECTF--
PARSE   INFO: Parsing mongodb://%s:%d
PARSE   INFO: - Found node: %s:%d
PARSE   INFO: - Connection type: STANDALONE
PARSE   INFO: - Found option 'replicaSet': '%s'
PARSE   INFO: - Switching connection type: REPLSET
CON     INFO: mongo_get_read_write_connection: finding a REPLSET connection (read)
CON     INFO: connection_create: creating new connection for %s:%d
CON     FINE: Connecting to tcp://%s:%d (%s:%d;%s;.;%d) without connection timeout
CON     INFO: stream_connect: Not establishing SSL for %s:%d
CON     FINE: Setting stream timeout to 30.000000
CON     INFO: ismaster: start
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     FINE: send_packet: read from header: %d
CON     FINE: send_packet: data_size: %d
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     FINE: ismaster: %sminWireVersion%s
CON     FINE: ismaster: %smaxWireVersion%s
CON     FINE: ismaster: %smaxBsonObjectSize%s
CON     FINE: ismaster: %smaxMessageSizeBytes%s
CON     FINE: ismaster: %smaxWriteBatchSize%s
CON     INFO: ismaster: set name: REPLICASET, ismaster: 1, secondary: 0, is_arbiter: 0
CON     FINE: ismaster: added tag dc:ny
CON     FINE: ismaster: added tag server:0
CON     INFO: ismaster: last ran at %d
CON     INFO: get_server_version: start
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     FINE: send_packet: read from header: %d
CON     FINE: send_packet: data_size: %d
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     INFO: get_server_version: server version: %s
CON     INFO: is_ping: pinging %s:%d;%s;.;%d
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     FINE: send_packet: read from header: %d
CON     FINE: send_packet: data_size: %d
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     INFO: is_ping: last pinged at %d; time: %dms
CON     FINE: discover_topology: checking ismaster for %s:%d;%s;.;%d
CON     FINE: found connection %s:%d;%s;.;%d (looking for %s:%d;%s;.;%d)
CON     INFO: ismaster: start
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     FINE: send_packet: read from header: %d
CON     FINE: send_packet: data_size: %d
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     FINE: ismaster: %sminWireVersion%s
CON     FINE: ismaster: %smaxWireVersion%s
CON     FINE: ismaster: %smaxBsonObjectSize%s
CON     FINE: ismaster: %smaxMessageSizeBytes%s
CON     FINE: ismaster: %smaxWriteBatchSize%s
CON     INFO: ismaster: set name: REPLICASET, ismaster: 1, secondary: 0, is_arbiter: 0
CON     FINE: ismaster: added tag dc:ny
CON     FINE: ismaster: added tag server:0
CON     FINE: ismaster: the server name matches what we thought it'd be (%s:%d).
CON     FINE: ismaster: the found replicaset name matches the expected one (REPLICASET).
CON     INFO: found host: %s:%d
CON     INFO: found host: %s:%d
CON     INFO: found host: %s:%d
CON     INFO: found host: %s:%d (passive)
CON     INFO: ismaster: last ran at %d
CON     FINE: discover_topology: ismaster worked
CON     FINE: found connection %s:%d;%s;.;%d (looking for %s:%d;%s;.;%d)
CON     INFO: discover_topology: found new host: %s:%d
CON     INFO: connection_create: creating new connection for %s:%d
CON     FINE: Connecting to tcp://%s:%d (l%s:%d;%s;.;%d) without connection timeout
CON     INFO: stream_connect: Not establishing SSL for %s:%d
CON     FINE: Setting stream timeout to 30.000000
CON     INFO: ismaster: start
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     FINE: send_packet: read from header: %d
CON     FINE: send_packet: data_size: %d
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     FINE: ismaster: %sminWireVersion%s
CON     FINE: ismaster: %smaxWireVersion%s
CON     FINE: ismaster: %smaxBsonObjectSize%s
CON     FINE: ismaster: %smaxMessageSizeBytes%s
CON     FINE: ismaster: %smaxWriteBatchSize%s
CON     INFO: ismaster: set name: REPLICASET, ismaster: 0, secondary: 1, is_arbiter: 0
CON     FINE: ismaster: added tag dc:sf
CON     FINE: ismaster: added tag server:2
CON     INFO: ismaster: last ran at %d
CON     INFO: get_server_version: start
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     FINE: send_packet: read from header: %d
CON     FINE: send_packet: data_size: %d
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     INFO: get_server_version: server version: %s
CON     INFO: is_ping: pinging %s:%d;%s;.;%d
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     FINE: send_packet: read from header: %d
CON     FINE: send_packet: data_size: %d
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     INFO: is_ping: last pinged at %d; time: %dms
CON     FINE: ismaster: skipping: last ran at %d, now: %d, time left: 15
CON     INFO: discover_topology: found new host: %s:%d
CON     INFO: connection_create: creating new connection for %s:%d
CON     FINE: Connecting to tcp://%s:%d (l%s:%d;%s;.;%d) without connection timeout
CON     INFO: stream_connect: Not establishing SSL for %s:%d
CON     FINE: Setting stream timeout to 30.000000
CON     INFO: ismaster: start
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     FINE: send_packet: read from header: %d
CON     FINE: send_packet: data_size: %d
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     FINE: ismaster: %sminWireVersion%s
CON     FINE: ismaster: %smaxWireVersion%s
CON     FINE: ismaster: %smaxBsonObjectSize%s
CON     FINE: ismaster: %smaxMessageSizeBytes%s
CON     FINE: ismaster: %smaxWriteBatchSize%s
CON     INFO: ismaster: set name: REPLICASET, ismaster: 0, secondary: 1, is_arbiter: 0
CON     FINE: ismaster: added tag dc:ny
CON     FINE: ismaster: added tag server:1
CON     INFO: ismaster: last ran at %d
CON     INFO: get_server_version: start
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     FINE: send_packet: read from header: %d
CON     FINE: send_packet: data_size: %d
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     INFO: get_server_version: server version: %s
CON     INFO: is_ping: pinging %s:%d;%s;.;%d
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     FINE: send_packet: read from header: %d
CON     FINE: send_packet: data_size: %d
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     INFO: is_ping: last pinged at %d; time: %dms
CON     FINE: ismaster: skipping: last ran at %d, now: %d, time left: 15
CON     INFO: discover_topology: found new host: %s:%d
CON     INFO: connection_create: creating new connection for %s:%d
CON     FINE: Connecting to tcp://%s:%d (l%s:%d;%s;.;%d) without connection timeout
CON     INFO: stream_connect: Not establishing SSL for %s:%d
CON     FINE: Setting stream timeout to 30.000000
CON     INFO: ismaster: start
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     FINE: send_packet: read from header: %d
CON     FINE: send_packet: data_size: %d
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     FINE: ismaster: %sminWireVersion%s
CON     FINE: ismaster: %smaxWireVersion%s
CON     FINE: ismaster: %smaxBsonObjectSize%s
CON     FINE: ismaster: %smaxMessageSizeBytes%s
CON     FINE: ismaster: %smaxWriteBatchSize%s
CON     INFO: ismaster: set name: REPLICASET, ismaster: 0, secondary: 1, is_arbiter: 0
CON     FINE: ismaster: added tag dc:sf
CON     FINE: ismaster: added tag server:3
CON     INFO: ismaster: last ran at %d
CON     INFO: get_server_version: start
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     FINE: send_packet: read from header: %d
CON     FINE: send_packet: data_size: %d
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     INFO: get_server_version: server version: %s
CON     INFO: is_ping: pinging %s:%d;%s;.;%d
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     FINE: send_packet: read from header: %d
CON     FINE: send_packet: data_size: %d
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     INFO: is_ping: last pinged at %d; time: %dms
CON     FINE: ismaster: skipping: last ran at %d, now: %d, time left: 15
CON     FINE: discover_topology: checking ismaster for %s:%d;%s;.;%d
CON     FINE: found connection %s:%d;%s;.;%d (looking for %s:%d;%s;.;%d)
CON     INFO: ismaster: start
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     FINE: send_packet: read from header: %d
CON     FINE: send_packet: data_size: %d
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     FINE: ismaster: %sminWireVersion%s
CON     FINE: ismaster: %smaxWireVersion%s
CON     FINE: ismaster: %smaxBsonObjectSize%s
CON     FINE: ismaster: %smaxMessageSizeBytes%s
CON     FINE: ismaster: %smaxWriteBatchSize%s
CON     INFO: ismaster: set name: REPLICASET, ismaster: 0, secondary: 1, is_arbiter: 0
CON     FINE: ismaster: added tag dc:sf
CON     FINE: ismaster: added tag server:2
CON     FINE: ismaster: the server name matches what we thought it'd be (%s:%d).
CON     FINE: ismaster: the found replicaset name matches the expected one (REPLICASET).
CON     INFO: found host: %s:%d
CON     INFO: found host: %s:%d
CON     INFO: found host: %s:%d
CON     INFO: found host: %s:%d (passive)
CON     INFO: ismaster: last ran at %d
CON     FINE: discover_topology: ismaster worked
CON     FINE: found connection %s:%d;%s;.;%d (looking for %s:%d;%s;.;%d)
CON     FINE: found connection %s:%d;%s;.;%d (looking for %s:%d;%s;.;%d)
CON     FINE: found connection %s:%d;%s;.;%d (looking for %s:%d;%s;.;%d)
CON     FINE: found connection %s:%d;%s;.;%d (looking for %s:%d;%s;.;%d)
CON     FINE: discover_topology: checking ismaster for %s:%d;%s;.;%d
CON     FINE: found connection %s:%d;%s;.;%d (looking for %s:%d;%s;.;%d)
CON     INFO: ismaster: start
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     FINE: send_packet: read from header: %d
CON     FINE: send_packet: data_size: %d
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     FINE: ismaster: %sminWireVersion%s
CON     FINE: ismaster: %smaxWireVersion%s
CON     FINE: ismaster: %smaxBsonObjectSize%s
CON     FINE: ismaster: %smaxMessageSizeBytes%s
CON     FINE: ismaster: %smaxWriteBatchSize%s
CON     INFO: ismaster: set name: REPLICASET, ismaster: 0, secondary: 1, is_arbiter: 0
CON     FINE: ismaster: added tag dc:ny
CON     FINE: ismaster: added tag server:1
CON     FINE: ismaster: the server name matches what we thought it'd be (%s:%d).
CON     FINE: ismaster: the found replicaset name matches the expected one (REPLICASET).
CON     INFO: found host: %s:%d
CON     INFO: found host: %s:%d
CON     INFO: found host: %s:%d
CON     INFO: found host: %s:%d (passive)
CON     INFO: ismaster: last ran at %d
CON     FINE: discover_topology: ismaster worked
CON     FINE: found connection %s:%d;%s;.;%d (looking for %s:%d;%s;.;%d)
CON     FINE: found connection %s:%d;%s;.;%d (looking for %s:%d;%s;.;%d)
CON     FINE: found connection %s:%d;%s;.;%d (looking for %s:%d;%s;.;%d)
CON     FINE: found connection %s:%d;%s;.;%d (looking for %s:%d;%s;.;%d)
CON     FINE: discover_topology: checking ismaster for %s:%d;%s;.;%d
CON     FINE: found connection %s:%d;%s;.;%d (looking for %s:%d;%s;.;%d)
CON     INFO: ismaster: start
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     FINE: send_packet: read from header: %d
CON     FINE: send_packet: data_size: %d
CON     FINE: No timeout changes for %s:%d;%s;.;%d
CON     FINE: ismaster: %sminWireVersion%s
CON     FINE: ismaster: %smaxWireVersion%s
CON     FINE: ismaster: %smaxBsonObjectSize%s
CON     FINE: ismaster: %smaxMessageSizeBytes%s
CON     FINE: ismaster: %smaxWriteBatchSize%s
CON     INFO: ismaster: set name: REPLICASET, ismaster: 0, secondary: 1, is_arbiter: 0
CON     FINE: ismaster: added tag dc:sf
CON     FINE: ismaster: added tag server:3
CON     FINE: ismaster: the server name matches what we thought it'd be (%s:%d).
CON     FINE: ismaster: the found replicaset name matches the expected one (REPLICASET).
CON     INFO: found host: %s:%d
CON     INFO: found host: %s:%d
CON     INFO: found host: %s:%d
CON     INFO: found host: %s:%d (passive)
CON     INFO: ismaster: last ran at %d
CON     FINE: discover_topology: ismaster worked
CON     FINE: found connection %s:%d;%s;.;%d (looking for %s:%d;%s;.;%d)
CON     FINE: found connection %s:%d;%s;.;%d (looking for %s:%d;%s;.;%d)
CON     FINE: found connection %s:%d;%s;.;%d (looking for %s:%d;%s;.;%d)
CON     FINE: found connection %s:%d;%s;.;%d (looking for %s:%d;%s;.;%d)
REPLSET FINE: finding candidate servers
REPLSET FINE: - all servers
REPLSET FINE: - collect any
REPLSET FINE: filter_connections: adding connections:
REPLSET FINE: - connection: type: %s, socket: 42, ping: %d, hash: %s:%d;%s;.;%d
REPLSET FINE:   - tag: %s:%s
REPLSET FINE:   - tag: %s:%s
REPLSET FINE: - connection: type: %s, socket: 42, ping: %d, hash: %s:%d;%s;.;%d
REPLSET FINE:   - tag: %s:%s
REPLSET FINE:   - tag: %s:%s
REPLSET FINE: - connection: type: %s, socket: 42, ping: %d, hash: %s:%d;%s;.;%d
REPLSET FINE:   - tag: %s:%s
REPLSET FINE:   - tag: %s:%s
REPLSET FINE: - connection: type: %s, socket: 42, ping: %d, hash: %s:%d;%s;.;%d
REPLSET FINE:   - tag: %s:%s
REPLSET FINE:   - tag: %s:%s
REPLSET FINE: filter_connections: done
REPLSET FINE: limiting to servers with same replicaset name
REPLSET FINE: - connection: type: %s, socket: 42, ping: %d, hash: %s:%d;%s;.;%d
REPLSET FINE:   - tag: %s:%s
REPLSET FINE:   - tag: %s:%s
REPLSET FINE: - connection: type: %s, socket: 42, ping: %d, hash: %s:%d;%s;.;%d
REPLSET FINE:   - tag: %s:%s
REPLSET FINE:   - tag: %s:%s
REPLSET FINE: - connection: type: %s, socket: 42, ping: %d, hash: %s:%d;%s;.;%d
REPLSET FINE:   - tag: %s:%s
REPLSET FINE:   - tag: %s:%s
REPLSET FINE: - connection: type: %s, socket: 42, ping: %d, hash: %s:%d;%s;.;%d
REPLSET FINE:   - tag: %s:%s
REPLSET FINE:   - tag: %s:%s
REPLSET FINE: limiting to servers with same replicaset name: done
REPLSET FINE: limiting by credentials
REPLSET FINE: - connection: type: %s, socket: 42, ping: %d, hash: %s:%d;%s;.;%d
REPLSET FINE:   - tag: %s:%s
REPLSET FINE:   - tag: %s:%s
REPLSET FINE: - connection: type: %s, socket: 42, ping: %d, hash: %s:%d;%s;.;%d
REPLSET FINE:   - tag: %s:%s
REPLSET FINE:   - tag: %s:%s
REPLSET FINE: - connection: type: %s, socket: 42, ping: %d, hash: %s:%d;%s;.;%d
REPLSET FINE:   - tag: %s:%s
REPLSET FINE:   - tag: %s:%s
REPLSET FINE: - connection: type: %s, socket: 42, ping: %d, hash: %s:%d;%s;.;%d
REPLSET FINE:   - tag: %s:%s
REPLSET FINE:   - tag: %s:%s
REPLSET FINE: limiting by credentials: done
REPLSET FINE: sorting servers by priority and ping time
REPLSET FINE: - connection: type: %s, socket: 42, ping: %d, hash: %s:%d;%s;.;%d
REPLSET FINE:   - tag: %s:%s
REPLSET FINE:   - tag: %s:%s
REPLSET FINE: - connection: type: %s, socket: 42, ping: %d, hash: %s:%d;%s;.;%d
REPLSET FINE:   - tag: %s:%s
REPLSET FINE:   - tag: %s:%s
REPLSET FINE: - connection: type: %s, socket: 42, ping: %d, hash: %s:%d;%s;.;%d
REPLSET FINE:   - tag: %s:%s
REPLSET FINE:   - tag: %s:%s
REPLSET FINE: - connection: type: %s, socket: 42, ping: %d, hash: %s:%d;%s;.;%d
REPLSET FINE:   - tag: %s:%s
REPLSET FINE:   - tag: %s:%s
REPLSET FINE: sorting servers: done
REPLSET FINE: selecting near servers
REPLSET FINE: selecting near servers: nearest is %dms
REPLSET FINE: - connection: type: %s, socket: 42, ping: %d, hash: %s:%d;%s;.;%d
REPLSET FINE:   - tag: %s:%s
REPLSET FINE:   - tag: %s:%s
REPLSET FINE: - connection: type: %s, socket: 42, ping: %d, hash: %s:%d;%s;.;%d
REPLSET FINE:   - tag: %s:%s
REPLSET FINE:   - tag: %s:%s
REPLSET FINE: - connection: type: %s, socket: 42, ping: %d, hash: %s:%d;%s;.;%d
REPLSET FINE:   - tag: %s:%s
REPLSET FINE:   - tag: %s:%s
REPLSET FINE: - connection: type: %s, socket: 42, ping: %d, hash: %s:%d;%s;.;%d
REPLSET FINE:   - tag: %s:%s
REPLSET FINE:   - tag: %s:%s
REPLSET FINE: selecting near server: done
REPLSET INFO: pick server: random element %d
REPLSET INFO: - connection: type: %s%s, socket: 42, ping: %d, hash: %s:%d;%s;.;%d
REPLSET INFO:   - tag: %s:%s
REPLSET INFO:   - tag: %s:%s
