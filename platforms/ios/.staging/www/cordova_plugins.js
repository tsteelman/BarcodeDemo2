cordova.define('cordova/plugin_list', function(require, exports, module) {
module.exports = [
    {
        "file": "plugins/org.apache.cordova.console/www/console-via-logger.js",
        "id": "org.apache.cordova.console.console",
        "clobbers": [
            "console"
        ]
    },
    {
        "file": "plugins/org.apache.cordova.console/www/logger.js",
        "id": "org.apache.cordova.console.logger",
        "clobbers": [
            "cordova.logger"
        ]
    },
    {
        "file": "plugins/com.phonegap.plugins.socketscan/www/socketscan.js",
        "id": "com.phonegap.plugins.socketscan.SocketScan",
        "clobbers": [
            "cordova.plugins.socketscan"
        ]
    }
];
module.exports.metadata = 
// TOP OF METADATA
{
    "org.apache.cordova.console": "0.2.9",
    "com.phonegap.plugins.socketscan": "1.1.0"
}
// BOTTOM OF METADATA
});