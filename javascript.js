// ************************************************
window.addEventListener('DOMContentLoaded', init, false);

function init() {
    Promise.all([
        fetch('firmware/versions.json').then(response => response.json()),
        fetch('firmware/releases.json').then(response => response.json())
    ])
    .then(([versions, releases]) => {
        GenerateSelectList(versions, releases);
        // Process releases if needed
        checkSupported(); 
        resetCheckboxes();
        handleTemplates();
    })
    .catch(error => console.error('Error loading versions:', error));
}

function showSerialHelp() {
    document.getElementById('coms').innerHTML = `Hit "Install" and select the correct COM port.<br><br>
    You might be missing the drivers for your board.<br>
    Here are drivers for chips commonly used in ESP boards:<br>
    <a href="https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers" target="_blank">CP2102 (square chip)</a><br>
    <a href="https://github.com/nodemcu/nodemcu-devkit/tree/master/Drivers" target="_blank">CH34x (rectangular chip)</a><br><br>
    Make sure your USB cable supports data transfer.<br><br>
    `;
}

function getRepositoryName() {
    const url = new URL(top.location.href);
    const pathSegments = url.pathname.split('/');
    const firstPathSegment = pathSegments.length > 1 ? pathSegments[1] : '';
    return firstPathSegment;
}

function checkSupported() {
    if (document.getElementById('web-install-button').hasAttribute('install-unsupported')) unsupported();
    else setManifest();
}

function unsupported() {
    
    document.getElementById('flasher').innerHTML = `Sorry, your browser is not yet supported!<br>
    Please try on Desktop Chrome or Edge.<br>
    Find binary files here:<br>
    <a href="https://github.com/tobiasfaust/` + getRepositoryName() + `/releases" target="_blank">
    <button class="btn" slot="activate">GitHub Releases</button>
    </a>`
}

function resetCheckboxes() {
    const radioButtons = document.querySelectorAll('input[type="radio"]');
    radioButtons.forEach(radio => {
        radio.checked = false;
        radio.disabled = false;
    });
}

// json: {path: 'firmware/v2.5.0-237-development/manifest_all.json', version: 'v2.5.0-237', stage: 'development', build: 237}
function GenerateSelectList(versions, releases) {
    const select = document.getElementById('versions');
    const stages = {};

    // Group by stage, only include development versions
    versions.forEach(obj => {
        if (obj.stage == "development") {
            if (!stages[obj.stage]) {
                stages[obj.stage] = [];
            }
            stages[obj.stage].push(obj);
        }
    });

    // Group by stage, for all releases
    releases.forEach(obj => {
        if (!stages[obj.stage]) {
                stages[obj.stage] = [];
            }
            stages[obj.stage].push(obj);
    });

    // Sort each stage by build number in descending order
    for (const stage in stages) {
        stages[stage].sort((a, b) => b.build - a.build);
    }

    // Create optgroups and options
    for (const stage in stages) {
        const optgroup = document.createElement('optgroup');
        optgroup.label = stage;
        stages[stage].forEach(obj => {
            const option = document.createElement('option');
            option.value = obj.path;
            option.text = obj.version + " (Build " + obj.build + ")";
            optgroup.appendChild(option);
        });
        select.appendChild(optgroup);
    }
}

function setManifest() {
    var sel = document.getElementById('versions');
    var opt = sel.options[sel.selectedIndex];
    var m = opt.value;
    document.getElementById('web-install-button').setAttribute('manifest', m);
}

function handleTemplates() {
    const elem = {"template": "REPOSITORY", value: getRepositoryName() };
    
    // gehe durch elem durch und suche in allen Objekten mit innerHTML den value aus "template" 
    // eingeschlossen in {{ }} und ersetze diesen mit dem Wert aus value

    document.querySelectorAll('*').forEach(node => {
        if (node.innerHTML.includes('{{' + elem.template + '}}')) {
            while (node.innerHTML.includes('{{' + elem.template + '}}')) {
                node.innerHTML = node.innerHTML.replace('{{' + elem.template + '}}', elem.value);
            }
        }
    });
}