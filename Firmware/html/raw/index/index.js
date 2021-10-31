info = {}
ssids = []

var request = new XMLHttpRequest()
var request2 = new XMLHttpRequest()

window.onload = () => {
    updateNetInfo()
    updateSSIDs()
    prepareLinks()
}

function writeInfo() {
    var netInfo = document.getElementById("net-info")
    netInfo.children[0].textContent = info.ssid
    netInfo.children[1].textContent = info.ipaddr
    netInfo.children[2].textContent = "当前天气定位：" + info.useRegion
    var connectStatus = document.getElementById("net-info-status")
    if (info.isConnect) {
        connectStatus.innerHTML = '<span>已连接</span>'
        connectStatus.id = "net-info-status"
    } else {
        connectStatus.innerHTML = '<span>未连接</span>'
        connectStatus.id = "net-info-status-notok"
    }
}

function prepareLinks() {
    document.getElementById("update-firmware").onclick = () => {
        window.open("/update", "_self")
    }
    document.getElementById("save-btn").onclick = () => {
        let ssidObj = document.getElementById("wifi-list")
        let ssid = ssidObj[ssidObj.selectedIndex].text
        let pwd = document.getElementById("wifi-password").value
        let region = document.getElementById("custom-region").value
        let r = new XMLHttpRequest()
        setTimeout(() => {
            alert("保存成功！")
        }, 200);
        r.open('POST', '/commit', true)
        r.setRequestHeader("Content-type", "application/x-www-form-urlencoded")
        r.send(`ssid=${ssid}&pwd=${pwd}&region=${region}`)
    }
}


function updateNetInfo() {
    request.open("GET", "/getnetinfo", true)
    request.onreadystatechange = () => {
        if (request.readyState == 4) {
            info = JSON.parse(request.responseText)
            writeInfo()
        }
    }
    request.send(null)
}

function updateSSIDs() {
    request2.open("GET", "/getssids", true)
    request2.onreadystatechange = () => {
        if (request2.readyState == 4) {
            ssids = JSON.parse(request2.responseText)
            var ssidList = document.getElementById("wifi-list")
            ssidList.innerHTML = ""
            for (s of ssids) {
                ssidList.innerHTML += `<option>${s}</option>`
            }
        }
    }
    request2.send(null)
}