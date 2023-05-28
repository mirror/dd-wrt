<?php
#Copyright (C) BlueWave Projects and Services 2016-2023
#This software is released under the GNU GPL license.

if (isset($argv[1])) {$remote_url=$argv[1];} else {echo "missing argument\n"; exit(1);}
if (isset($argv[2])) {$action=$argv[2];} else {echo "missing argument\n"; exit(1);}
if (isset($argv[3])) {$gatewayhash=$argv[3];} else {echo "missing argument\n"; exit(1);}
if (isset($argv[4])) {$user_agent=$argv[4];} else {echo "missing argument\n"; exit(1);}
if (isset($argv[5])) {$payload=$argv[5];} else {$payload="none";}

$payload=base64_encode($payload);

$_p = array (
	"auth_get"=>$action,
	"gatewayhash"=>$gatewayhash,
	"payload"=>$payload
);

$response=SendPostData($_p, $remote_url, $user_agent);
echo "$response";

function SendPostData($_p, $remote_url, $user_agent) {
	$fields_string = http_build_query($_p);
	$headers="Content-type: application/x-www-form-urlencoded\r\n"."Content-Length: ".strlen($fields_string)."\r\n";

	$context_options = array (
		'http' => array (
			'method' => 'POST',
			'header' => $headers,
			'user_agent' => $user_agent,
			'content'=> $fields_string
		)
	);

	$context = stream_context_create($context_options);

	//open the stream and get the response
	$fp = fopen($remote_url, 'r', false, $context);
	$response = "";

	if ($fp == TRUE) {
		$response = trim(stream_get_contents($fp));
	} else {
		return "ERROR: Failed_to_open_stream_to: [$remote_url]";
	}
	return $response;

}

?>
