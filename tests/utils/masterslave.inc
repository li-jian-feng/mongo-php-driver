<?php
require_once "tests/utils/server.inc";

try {
    $server = new MongoShellServer;
    $server->getMasterSlaveConfig();
    if (isset($needs)) {
        $needsOp = isset($needsOp) ? $needsOp : '>=';
        $serverversion = $server->getServerVersion("MASTERSLAVE");
        if ( ! version_compare($serverversion, $needs, $needsOp)) {
            throw new Exception("Needs MongoDB $needsOp $needs, but is running $serverversion");
        }
    }
    $server->close();
} catch(Exception $e) {
    exit("skip " . $e->getMessage());
}
