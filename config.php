    <?php 
define('DB_HOST'    , 'localhost'); 
define('DB_USERNAME', 'suhu'); 
define('DB_PASSWORD', 'bismillah123'); 
define('DB_NAME'    , 'suhu');

define('POST_DATA_URL', 'http://10.10.1.23/sensordata.php');

//PROJECT_API_KEY is the exact duplicate of, PROJECT_API_KEY in NodeMCU sketch file
//Both values must be same
define('PROJECT_API_KEY', 'hello world');


//set time zone for your country
date_default_timezone_set('Asia/Jakarta');

// Connect with the database 
$db = new mysqli(DB_HOST, DB_USERNAME, DB_PASSWORD, DB_NAME); 
 
// Display error if failed to connect 
if ($db->connect_errno) { 
    echo "Connection to database is failed: ".$db->connect_error;
    exit();
}
