<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') { 
    http_response_code(204); 
    exit; 
}
// Check all potential locations where stats.json might reside
$candidates = [
    __DIR__ . '/stats.json',           // Same folder as api.php
    __DIR__ . '/../stats.json',        // Root project folder (where your 14 sessions are!)
    __DIR__ . '/../data/stats.json',   // Subfolder data/
    __DIR__ . '/data/stats.json'
];

$statsFile = __DIR__ . '/stats.json'; // fallback default
foreach ($candidates as $candidate) {
    if (file_exists($candidate) && filesize($candidate) > 10) {
        $statsFile = $candidate;
        break;
    }
}

// Telemetry file path
$telemetryFile = file_exists(__DIR__ . '/../telemetry.json') 
    ? __DIR__ . '/../telemetry.json' 
    : __DIR__ . '/telemetry.json';
function readLocked($f) {
    if (!file_exists($f)) return '';
    $h = @fopen($f, 'r');
    if (!$h) return '';
    flock($h, LOCK_SH);
    $s = stream_get_contents($h);
    flock($h, LOCK_UN);
    fclose($h);
    return $s;
}

if (!file_exists($telemetryFile)) {
    file_put_contents($telemetryFile, json_encode([
        'heart_rate' => 'No finger is detected',
        'steadiness' => 'STOPPED',
        'timestamp'  => time(),
        'online'     => false
    ]), LOCK_EX);
}

if (!file_exists($statsFile)) {
    file_put_contents($statsFile, json_encode([]), LOCK_EX);
}

// -------------------------------------------------------------
// POST Handling (Saving Results & Ingesting Telemetry)
// -------------------------------------------------------------
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $rawInput = file_get_contents('php://input');
    $jsonData = json_decode($rawInput, true);

    // Save session results from UI
    if ($jsonData && ($jsonData['action'] ?? '') === 'save_result'
        && isset($jsonData['record']) && is_array($jsonData['record'])) {
        
        $history = json_decode(readLocked($statsFile), true) ?: [];
        array_unshift($history, $jsonData['record']);
        $history = array_slice($history, 0, 50); // Keep last 50 runs
        
        file_put_contents($statsFile, json_encode($history, JSON_PRETTY_PRINT), LOCK_EX);
        echo json_encode(['status' => 'success']);
        exit;
    }

    // ESP32 Sensor Telemetry Ingestion
    $hr = $_POST['heart_rate'] ?? ($jsonData['heart_rate'] ?? 'No finger is detected');
    $st = $_POST['steadiness'] ?? ($jsonData['steadiness'] ?? 'STOPPED');

    $payload = [
        'heart_rate' => $hr,
        'steadiness' => $st,
        'timestamp'  => time(),
        'online'     => true
    ];

    file_put_contents($telemetryFile, json_encode($payload), LOCK_EX);   
    echo json_encode(['status' => 'received']);
    exit;
}

// -------------------------------------------------------------
// GET Handling (History & Live Telemetry)
// -------------------------------------------------------------
$action = $_GET['action'] ?? 'get_telemetry';

if ($action === 'get_history') {
    $rawStats = readLocked($statsFile);
    echo !empty($rawStats) ? $rawStats : '[]';
    exit; // Crucial: Prevents falling through to telemetry output
}

// Telemetry Poller: Mark offline if no packets in last 4 seconds
$data = json_decode(readLocked($telemetryFile), true);
if (!$data || (time() - ($data['timestamp'] ?? 0)) > 4) {
    $data = is_array($data) ? $data : [];
    $data['online']     = false;
    $data['heart_rate'] = 'No sensor stream';
    $data['steadiness'] = 'STOPPED';
}

echo json_encode($data);
exit;