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

    get_chart_data() {
        if (this.data == null) {
            return false;
        }

        return this.data.chart;
    }
}

let menuElements = {
    camera: document.getElementById("videoPage"),
    stats: document.getElementById("statsPage"),
}

let connection = null;
let flvPlayer = null;
let statsChart = null;
let isPanning = false;
let startX = 0;
let startY = 0;

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
    
    setup_chart();
    setInterval(check_online, 100);
    setInterval(update_statistics, 100);
    
    hide_element(document.getElementById("login"));
    show_element(document.getElementById("main"));
    show_element(document.getElementById("nav"));
    toggle_view(menuElements.camera);
    return true;
}

function toggle_view(view) {
    for (let element in menuElements) {
        if (menuElements[element] == view) continue;

        hide_element(menuElements[element]);
    }

    show_element(view);
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
        /*for (let key in elements) {
            elements[key][0].innerHTML = "0" + elements[key][1]
        }*/
        return;
    }

    console.log(connection.get_chart_data());
    statsChart.data = connection.get_chart_data();

    statsChart.update();

    /*// Update elements
    for (let key in elements) {
        // Check if json data isn't null.
        if (connection.get_data()[key] != undefined && elements[key][0] != undefined && elements[key][0] != null) {
            elements[key][0].innerHTML = connection.get_data()[key] + elements[key][1];
        }
    }*/
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
            url: connection.api_url + '/live/stream.flv?id=' + connection.login[0] + '&session=' + connection.login[1]
        });
        flvPlayer.attachMediaElement(videoElement);
        flvPlayer.load();
        flvPlayer.play();
    }
}

function reset_chart() {
    statsChart.destroy();

    create_chart();
}

function create_chart() {
    const statsChartCtx = document.getElementById('statsChart');

    const zoomOptions = {
        zoom: {
            wheel: {
                enabled: true,
            },
            pinch: {
                enabled: true
            },
            mode: 'x',
        },
        pan: {
            enabled: true,
            mode: 'xy',
            modifierKey: null
        }
    };

    statsChart = new Chart(statsChartCtx, {
        type: 'line',
        data: {
            labels: [Date.now()],
            datasets: []
        },
        options: {
            scales: {
                y: {
                    beginAtZero: true,
                    position: 'left'
                },
                x: {
                    type: 'time'
                },
                yBig: {
                    beginAtZero: true,
                    position: 'right'
                }
            },
            responsive: true,
            animation: false,
            plugins: {
                zoom: zoomOptions
            },
        }
    });
}

function setup_chart() {
    create_chart();

    const canvas = document.getElementById('statsChart');

    canvas.addEventListener('mousedown', (e) => {
        isPanning = true;
        startX = e.clientX;
        startY = e.clientY;
    });

    canvas.addEventListener('mousemove', (e) => {
        if (!isPanning) return;

        const deltaX = e.clientX - startX;
        const deltaY = e.clientY - startY;

        statsChart.pan({ x: deltaX, y: deltaY });

        startX = e.clientX;
        startY = e.clientY;
    });

    canvas.addEventListener('mouseup', () => {
        isPanning = false;
    });

    canvas.addEventListener('mouseleave', () => {
        isPanning = false;
    });
}

// Change this line back to hide_element(document.getElementById("main")); when finished testing.
hide_element(document.getElementById("main"));
hide_element(document.getElementById("nav"));
hide_element(document.getElementById("login-ell"));

Chart.register(ChartZoom);