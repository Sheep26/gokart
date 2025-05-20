class Connection {
    login = []

    constructor(api_url = "", login) {
        this.data = null;
        this.api_url = api_url;
        this.login = login;
    }

    async update_data() {
        // Implementation for updating data

        try {
            // Request data from api
            const response = await fetch(this.api_url + "/api/get_data", {
                method: "GET",
                headers: {
                    "session": this.login[1], // Session
                    "id": this.login[0] // Id
                }
            });

            if (!response.ok) {
              throw new Error(`Response status: ${response.status}`);
            }
        
            const json = await response.json();

            // Set data variable
            this.data = json;

            return json;
        } catch (error) {
            console.error(error.message);
        }
    }

    get_data() {
        if (this.data == null) {
            return null;
        }

        return this.data.data;
    }

    get_online() {
        if (this.data == null) {
            return false;
        }

        return this.data.online;
    }
}

// [element, unit_of_mesurement(str)]
// We store the elements we are using to display data here so we don't have to call for them later.
let elements = {
    speed: [document.getElementById("speed-data"), "km/h"],
    speed_avg: [document.getElementById("speed-data-avg"), "km/h"],
    speed_max: [document.getElementById("speed-data-max"), "km/h"],
    rpm: [document.getElementById("rpm-data"), "rpm"],
    rpm_avg: [document.getElementById("rpm-data-avg"), "rpm"],
    rpm_max: [document.getElementById("rpm-data-max"), "rpm"],
    power: [document.getElementById("power-data"), "W"],
    power_avg: [document.getElementById("power-data-avg"), "W"],
    power_max: [document.getElementById("power-data-max"), "W"],
    throttle: [document.getElementById("throttle-data"), "%"],
    throttle_avg: [document.getElementById("throttle-data-avg"), "%"],
    throttle_max: [document.getElementById("throttle-data-max"), "%"],
}

let connection = null;
let flvPlayer = null;

// Hide/Show elements.
function hide_element(element) {element.style.display = "none";}
function show_element(element) {element.style.display = "block";}

async function login(api_url, username, passwd) {
    // Login here, return session.
    const response = await fetch(api_url + "/api/login", {
        method: "GET",
        headers: {
            "username": username,
            "passwd": passwd
        }
    });

    if (!response.ok) {
        return false;
    }

    var text = (await response.text()).split(",");
    
    connection = new Connection(api_url, text);

    setInterval(update_statistics, 100);
    setInterval(check_online, 100);
    
    hide_element(document.getElementById("login"));
    show_element(document.getElementById("main"));
    return true;
}

function check_login() {
    login(document.getElementById("server-ip").value, document.getElementById("username").value, document.getElementById("password").value).then(result => {
        if (!result) {
            login_ell = document.getElementById("login-ell")
            show_element(login_ell);
            login_ell.innerHTML = "Invalid login."
        }
    });
}

function update_statistics() {
    connection.update_data();
    // Check if online
    if (!connection.get_online()) {
        // Set element data to 0.
        for (let key in elements) {
            elements[key][0].innerHTML = "0" + elements[key][1]
        }
        return;
    }

    // Update elements
    for (let key in elements) {
        // Check if json data isn't null.
        if (connection.get_data()[key] != undefined) {
            elements[key][0].innerHTML = connection.get_data()[key] + elements[key][1];
        }
    }
}

function check_online() {
    if (connection.get_online()) {
        if (flvPlayer == null) {
            create_flv();
        }
    } else if (flvPlayer != null) {
        flvPlayer.destroy();
        flvPlayer = null;
    }
}

function create_flv() {
    if (flvjs.isSupported()) {
        var videoElement = document.getElementById('videoElement');
        flvPlayer = flvjs.createPlayer({
            type: 'flv',
            url: connection.api_url + '/live/stream.flv?id=' + connection.login[0] + "&session=" + connection.login[1]
        });
        flvPlayer.attachMediaElement(videoElement);
        flvPlayer.load();
        flvPlayer.play();
    }
}

hide_element(document.getElementById("main"));
hide_element(document.getElementById("login-ell"));