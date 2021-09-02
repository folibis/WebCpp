<?php
$name = isset($_GET['name']) ? $_GET['name'] : 'Unknown name';
$action = isset($_GET['action']) ? $_GET['action'] : 'Unknown action';

echo("$name, $action");
?>