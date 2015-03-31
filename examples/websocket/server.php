<?php
$server = new swoole_websocket_server("0.0.0.0", 9501);
//$server = new swoole_websocket_server("0.0.0.0", 9501, SWOOLE_BASE);
//$server->set(['worker_num' => 4]);

$server->on('open', function (swoole_websocket_server $_server, swoole_http_request $request) {
    echo "server#{$_server->worker_pid}: handshake success with fd#{$request->fd}\n";
	
//    var_dump($request);
});

$server->on('message', function (swoole_websocket_server $_server, $frame) {
    var_dump($frame);
    echo "received ".strlen($frame->data)." bytes\n";
    //echo "receive from {$fd}:{$data},opcode:{$opcode},fin:{$fin}\n";
    $_server->push($frame->fd, "this is server");
	$_server->close($frame->fd);
});

$server->on('close', function ($_server, $fd) {
    echo "client {$fd} closed\n";
});

$server->on('request', function (swoole_http_request $request, swoole_http_response $response) {
    $response->end(<<<HTML
    <h1>Swoole WebSocket Server</h1>
    <script>
var wsServer = 'ws://127.0.0.1:9501';
var websocket = new WebSocket(wsServer);
websocket.onopen = function (evt) {
	console.log("Connected to WebSocket server.");
};

websocket.onclose = function (evt) {
	console.log("Disconnected");
};

websocket.onmessage = function (evt) {
	console.log('Retrieved data from server: ' + evt.data);
};

websocket.onerror = function (evt, e) {
	console.log('Error occured: ' + evt.data);
};
</script>
HTML
    );
});

$server->start();
