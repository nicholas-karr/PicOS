let info = document.getElementById("info");
let log = document.getElementById("log");
let print = (str) => { info.innerText += str };

const trackpad = document.getElementById("trackpad");

let inputState = {
    mouseX: 0,
    mouseY: 0,
    keys: ""
}

let prevInputState = {
    
}

let debugVals = {};
function updateDebugVals() {
    log.innerHTML = "";
    for (const k in debugVals) {
        log.innerHTML += `${k}: ${debugVals[k]}<br>`;
    }
}
function putDebugVal(k, v) {
    debugVals[k] = v;
    updateDebugVals();
}

function mouseMove(e) {
    const rect = trackpad.getBoundingClientRect();
    let pos = { x: e.clientX - rect.left, y: e.clientY - rect.top };
    pos = { x: pos.x > 1280 ? 1280 : pos.x, y: pos.y > 720 ? 720 : pos.y };

    inputState.mouseX = pos.x;
    inputState.mouseY = pos.y;
    //debugVals["MouseX"] = pos.x;
    //debugVals["MouseY"] = pos.y;
    //updateDebugVals();
}
function mouseLeave() {}

let port = null;
let writer = null;

function tick() {
    writer.write("001008" + inputState.mouseX.toString().padStart(4, '0') + inputState.mouseY.toString().padStart(4, '0'));
}

async function main() {

    print("PicOS Graphical Control");

    const ports = await navigator.serial.getPorts();
    if (ports.length == 0) {
        port = await navigator.serial.requestPort();
    }
    else {
        port = ports[0];
    }

    console.log(port, ports)

    await port.open({ baudRate: 115200 });

    const textEncoder = new TextEncoderStream();
    const writableStreamClosed = textEncoder.readable.pipeTo(port.writable);

    writer = textEncoder.writable.getWriter();

    writer.write("003000");


    setInterval(tick, 10);
}

