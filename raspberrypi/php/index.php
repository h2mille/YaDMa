<?php 
class MyDB extends SQLite3 {
  function __construct() {
     $this->open("/home/pi/MY_DB.db");
  }
}
$db = new MyDB();
$key1 = $_POST['KEY1'];
$decrypt_key = exec('/home/pi/aes_decrypt '.$key1);
$key2 = $_POST['KEY2'];
$decrypt_key_2 = exec('/home/pi/aes_decrypt '.$key2);



//echo("SELECT name,card FROM users where card=\"$decrypt_key\"");
//echo("\n");
$result = $db->query("SELECT name,card, start_time, stop_time, permission FROM users WHERE card=\"$decrypt_key\"");

if (!$result) {
  echo($db->lastErrorMsg());
}

date_default_timezone_set('UTC');
$time_now = strtotime(date('Y-m-d',time()));
$result = $result->fetcharray();
$time_start = strtotime($result['start_time']);
$time_stop = strtotime($result['stop_time']);

if($time_now<$time_start or $time_now>$time_stop){
    echo("out of time");
   die();
}

$room = substr($decrypt_key_2,-2);
echo((int)$room);
echo("\n");
$room = (int)$room;
if(($result['permission'] & (1<<$room))==0){
   echo("access forbidden");
   die();
}

$encrypt_key = exec('/home/pi/aes_encrypt_back '.$decrypt_key_2);
echo($encrypt_key);
?>
