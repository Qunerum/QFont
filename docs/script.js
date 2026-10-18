let characters = [],
	activeCharId = null,
	nextCharId = 1,
	isMouseDown = false,
	drawState = 1,
	rightText = "";

function appendData(html = '', nl = true) {
	rightText += html;
	if (nl) rightText += '<br>';
	const logContainer = document.getElementById('rightLogContent');
	if (logContainer) {
		logContainer.innerHTML = rightText;
		logContainer.scrollTop = logContainer.scrollHeight;
	}
}

window.addEventListener('mousedown', () => { isMouseDown = true; });
window.addEventListener('mouseup', () => { isMouseDown = false; });

const zoomSlider = document.getElementById('zoomSlider'),
	zoomValue = document.getElementById('zoomValue');
let currentCellSize = 18;

zoomSlider.addEventListener('input', (e) => {
	currentCellSize = e.target.value;
	zoomValue.innerText = `${currentCellSize}px`;
	document.documentElement.style.setProperty('--cell-size', `${currentCellSize}px`);
	const allCells = document.querySelectorAll('.cell-btn');
	allCells.forEach(cell => {
		if (currentCellSize < 18) cell.classList.add('small-cell');
		else cell.classList.remove('small-cell');
	});
});
const previewZoomSlider = document.getElementById('previewZoomSlider');
	previewZoomSlider.addEventListener('input', () => { renderSidebarPreview(); });
function getActiveCharacter() { return characters.find(c => c.id === activeCharId); }
function characterExist(id) { return characters.some(char => char.id === id); }
function getIndex(id) { return characters.findIndex(char => char.id === id); }
function drawBitmapToCanvas(canvas, bitMap, rows, cols, scale = 1) {
	const ctx = canvas.getContext('2d');
	canvas.width = cols * scale;
	canvas.height = rows * scale;
	ctx.imageSmoothingEnabled = false;
	ctx.fillStyle = '#141414';
	ctx.fillRect(0, 0, canvas.width, canvas.height);
	const activeColor = getComputedStyle(document.documentElement).getPropertyValue('--clr-active-cell').trim() || '#aaa';
	ctx.fillStyle = activeColor;
	for (let r = 0; r < rows; r++) for (let c = 0; c < cols; c++) if (bitMap[r][c] === 1) ctx.fillRect(c * scale, r * scale, scale, scale);
}
function renderSidebarPreview() {
	const previewCanvas = document.getElementById('sidebarPreviewCanvas'),
	activeChar = getActiveCharacter();
	if (!activeChar) {
		previewCanvas.width = 10;
		previewCanvas.height = 10;
		const ctx = previewCanvas.getContext('2d');
		ctx.fillStyle = '#141414';
		ctx.fillRect(0, 0, 10, 10);
		return;
	}
	const scale = parseInt(previewZoomSlider.value, 10);
	drawBitmapToCanvas(previewCanvas, activeChar.bitMap, activeChar.rows, activeChar.cols, scale);
}
function addChar(id, w, h, bm) {
	const nc = {
		id: id,
		rows: h,
		cols: w,
		bitMap: bm
	};
	if (characterExist(id)) characters[getIndex(id)] = nc;
	else characters.push(nc);
	selectCharacter(id);
}

document.getElementById('addCharBtn').addEventListener('click', () => {
	const rows = parseInt(document.getElementById('newRows').value, 10),
		cols = parseInt(document.getElementById('newCols').value, 10);
	if (isNaN(rows) || rows < 1 || rows > 127 || isNaN(cols) || cols < 1 || cols > 127) {
		alert("Dimensions must be within the range of 1x1 to 127x127!");
		return;
	}
	let bitMap = [];
	for (let r = 0; r < rows; r++) {
		let rowArr = [];
		for (let c = 0; c < cols; c++) rowArr.push(0);
		bitMap.push(rowArr);
	}
	addChar(nextCharId++, cols, rows, bitMap);
});

function selectCharacter(id) {
	activeCharId = id;
	calculate(getActiveCharacter());
	renderCharsList();
	renderGrid();
	renderSidebarPreview();
}
function removeCharacter(id, event) {
	event.stopPropagation();
	characters = characters.filter(c => c.id !== id);
	if (activeCharId === id) activeCharId = characters.length > 0 ? characters[0].id : null;
	renderCharsList();
	renderGrid();
	renderSidebarPreview();
}
function renderCharsList() {
	const listContainer = document.getElementById('charsList');
	listContainer.innerHTML = '';
	if (characters.length === 0) {
		listContainer.innerHTML = '<div style="color: #7a7c7d; font-size: 0.85rem; padding: 5px;">No characters...</div>';
		return;
	}
	characters.forEach(char => {
		const item = document.createElement('div');
		item.classList.add('char-item');
		if (char.id === activeCharId) item.classList.add('active');
		const miniCanvas = document.createElement('canvas');
		miniCanvas.classList.add('char-mini-canvas');
		drawBitmapToCanvas(miniCanvas, char.bitMap, char.rows, char.cols, 1);
		item.appendChild(miniCanvas);
		const idInput = document.createElement('input');
		idInput.type = 'text';
		idInput.classList.add('char-id-input');
		idInput.value = '0x' + Number(char.id).toString(16).toUpperCase().padStart(8, '0');
		idInput.addEventListener('click', (e) => e.stopPropagation());
		idInput.addEventListener('change', (e) => {
			let val = e.target.value.trim(),
				parsedVal = parseInt(val, val.toLowerCase().startsWith('0x') ? 16 : 10);
			if (!isNaN(parsedVal)) char.id = parsedVal;
			selectCharacter(parsedVal);
		});
		item.appendChild(idInput);
		const dimsSpan = document.createElement('span');
		dimsSpan.classList.add('char-dims');
		dimsSpan.innerText = `${char.cols}x${char.rows}`;
		item.appendChild(dimsSpan);
		const delBtn = document.createElement('button');
		delBtn.classList.add('char-del-btn');
		delBtn.innerText = '×';
		delBtn.title = 'Remove char';
		delBtn.addEventListener('click', (e) => removeCharacter(char.id, e));
		item.appendChild(delBtn);
		item.addEventListener('click', () => selectCharacter(char.id));
		listContainer.appendChild(item);
	});
}
function renderGrid() {
	const gridContainer = document.getElementById('gridContainer'),
	activeCharInfo = document.getElementById('activeCharInfo');
	gridContainer.innerHTML = '';
	const activeChar = getActiveCharacter();
	if (!activeChar) {
		activeCharInfo.innerText = "No character selected";
		return;
	}
	const hexId = '0x' + Number(activeChar.id).toString(16).toUpperCase().padStart(8, '0');
	activeCharInfo.innerText = `You're editing: ${hexId} (${activeChar.cols}x${activeChar.rows})`;
	gridContainer.style.gridTemplateColumns = `repeat(${activeChar.cols}, var(--cell-size))`;
	for (let r = 0; r < activeChar.rows; r++) {
		for (let c = 0; c < activeChar.cols; c++) {
			const btn = document.createElement('button');
			btn.classList.add('cell-btn');
			if (currentCellSize < 18) btn.classList.add('small-cell');
			const val = activeChar.bitMap[r][c];
			btn.innerText = val;
			if (val === 1) btn.classList.add('active');
			function updateCellState(newState) {
				activeChar.bitMap[r][c] = newState;
				if (newState === 1) {
					btn.innerText = '1';
					btn.classList.add('active');
				} else {
					btn.innerText = '0';
					btn.classList.remove('active');
				}
				renderSidebarPreview();
				renderCharsList();
			}
			btn.addEventListener('mousedown', (e) => {
				e.preventDefault();
				drawState = activeChar.bitMap[r][c] === 0 ? 1 : 0;
				updateCellState(drawState);
			});
			btn.addEventListener('mouseenter', () => { if (isMouseDown) updateCellState(drawState); });
			gridContainer.appendChild(btn);
		}
	}
}

window.addEventListener('beforeunload', (event) => {
	if (characters.length > 0) {
		event.preventDefault();
		event.returnValue = '';
	}
});

class BinaryWriter {
	constructor(initialCapacity = 1024) {
		this.buffer = new ArrayBuffer(initialCapacity);
		this.view = new DataView(this.buffer);
		this.offset = 0;
	}
	_ensureCapacity(bytesNeeded) {
		if (this.offset + bytesNeeded > this.buffer.byteLength) {
			const newBuffer = new ArrayBuffer(this.buffer.byteLength * 2);
			new Uint8Array(newBuffer).set(new Uint8Array(this.buffer));
			this.buffer = newBuffer;
			this.view = new DataView(this.buffer);
		}
	}
	addUint8(value) {
		this._ensureCapacity(1);
		this.view.setUint8(this.offset, value);
		this.offset += 1;
	}
	addInt8(value) {
		this._ensureCapacity(1);
		this.view.setInt8(this.offset, value);
		this.offset += 1;
	}
	getBuffer() { return this.buffer.slice(0, this.offset); }
}

function saveAndDownload(buffer, filename = 'font.qf') {
	const blob = new Blob([buffer], { type: 'application/octet-stream' }),
	url = URL.createObjectURL(blob),
	a = document.createElement('a');
	a.href = url;
	a.download = filename;
	document.body.appendChild(a);
	a.click();
	document.body.removeChild(a);
	URL.revokeObjectURL(url);
}
function isEmpty(char, r) {
	for (let c = 0; c < char.cols; c++) if (char.bitMap[r][c]) return false;
	return true;
}
function cntEnd(char) {
	let x = char.rows;
	for (let r = char.rows-1; r >= 0; r--) {
		if (!isEmpty(char, r)) return x;
		x--;
	}
	return 0;
}

let cd = [];

function calculate(char) {
	cd = [];
	rightText = '';
	if (characters.length == 0) return;
	const i0 = char.id >> 24 & 0xFF,
		i1 = char.id >> 16 & 0xFF,
		i2 = char.id >> 8 & 0xFF,
		i3 = char.id & 0xFF;
	let sc = true,
		startAt = 0,
		goForNext = cntEnd(char);
	cd.push(i0);
	cd.push(i1);
	cd.push(i2);
	cd.push(i3);
	cd.push(char.cols);
	cd.push(char.rows);
	appendData(`${i0},${i1},${i2},${i3}, ${char.cols},${char.rows}, `,0);
	for (let r = 0; r < char.rows; r++) {
		let l = 0,
			v = char.bitMap[r][0],
			td = [];
		if (isEmpty(char, r) && sc) {
			startAt++;
			continue;
		} else if (sc) {
			goForNext -= startAt;
			cd.push(startAt);
			cd.push(goForNext);
			appendData(`${startAt},${goForNext},`);
			sc = false;
		}
		for (let c = 0; c < char.cols; c++) {
			if (char.bitMap[r][c] == v) l++;
			else {
				v = !v;
				td.push(v ? l : -l);
				l = 1;
			}
		}
		l = v ? -l : l;
		if (l < 0) td.push(l);
		else if (isEmpty(char, r) && r < startAt + goForNext) td.push(0);
		if (!isEmpty(char, r) && r < startAt + goForNext) {
			cd.push(td.length);
			appendData(`${cd.at(-1)}, `,0);
		}
		for (let i = 0; i < td.length; i++) {
			cd.push(td[i]);
			appendData(`${cd.at(-1)},`,0);
		}
		appendData();
	}
	if (sc) {
		startAt = 0;
		cd.push(startAt);
		cd.push(goForNext);
		appendData(`${startAt},${goForNext},`);
	}
	rightText = rightText.replace(/(?:<br\s*\/?>)+$/gi, '').slice(0, -1);
	appendData('',0);
}

document.getElementById('calculateBtn').addEventListener('click', () => calculate(getActiveCharacter()));
document.getElementById('generateBtn').addEventListener('click', () => {
	calculate(getActiveCharacter());
	if (characters.length == 0) {
		alert('You must create at least one character!');
		return;
	}
	const writer = new BinaryWriter();
	characters.forEach((char) => {
		calculate(char);
		for (let i = 0; i < cd.length; i++) {
			if (i < 8) writer.addUint8(cd[i]);
			else writer.addInt8(cd[i]);
		}
	});
	const binaryData = writer.getBuffer();
	saveAndDownload(binaryData);
});
document.getElementById('saveAsBtn').addEventListener('click', async () => {
	if (characters.length == 0) {
		alert('You must create at least one character!');
		return;
	}
	const writer = new BinaryWriter();
	characters.forEach((char) => {
		calculate(char);
		for (let i = 0; i < cd.length; i++) {
			if (i < 8) writer.addUint8(cd[i]);
			else writer.addInt8(cd[i]);
		}
	});
	const binaryData = writer.getBuffer();
	if ('showSaveFilePicker' in window) {
		const options = {
			suggestedName: 'font.qf',
			types: [{
				description: 'QFont font',
				accept: { 'application/octet-stream': ['.qf'] },
			}],
		};
		const handle = await window.showSaveFilePicker(options),
			writable = await handle.createWritable();
		await writable.write(binaryData);
		await writable.close();
	} else {
		alert('Your browser does not support choosing a custom save path. Falling back to standard download.');
		saveAndDownload(binaryData, 'font.qf');
	}
});

const loadBtn = document.getElementById('loadFileBtn'),
	fileInput = document.getElementById('fileInput');
loadBtn.addEventListener('click', () => { fileInput.click(); });

fileInput.addEventListener('change', (event) => {
	const file = event.target.files[0];
	if (!file) return;
	const reader = new FileReader();
	reader.onload = function(e) {
		const arrayBuffer = e.target.result;
		parseQFontFile(arrayBuffer);
	};
	reader.readAsArrayBuffer(file);
	fileInput.value = '';
});
let offset = 0;
function readInt(data) { return data.getInt8(offset++); }
function readUint(data) { return data.getUint8(offset++); }
function parseQFontFile(arrayBuffer) {
	const d = new DataView(arrayBuffer);
	offset = 0;
	while (offset < d.byteLength) {
		const id =
			readUint(d) << 24 & 0xFF |
			readUint(d) << 16 & 0xFF |
			readUint(d) << 8 & 0xFF  |
			readUint(d) & 0xFF,
			sizeX = readUint(d),
			sizeY = readUint(d),
			startAt = readUint(d),
			goNext = readUint(d);
		let bm = [];
		for (let i = 0; i < sizeY; i++) {
			let z = [];
			for (let j = 0; j < sizeX; j++) z.push(0);
			bm.push(z);
		}
		for (let y = 0; y < goNext; y++) {
			const len = readUint(d);
			let i = 0;
			for (let x = 0; x < len; x++) {
				let v = readInt(d)
				const im = v < 0;
				if (im) v = -v;
				for (let t = 0; t < v; t++) {
					bm[y + startAt][i] = im ? 1 : 0;
					i++;
				}
			}
		}
		addChar(id, sizeX, sizeY, bm)
	}
}
