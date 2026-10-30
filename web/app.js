'use strict';
const $ = id => document.getElementById(id);
const catalog = [['', 'Empty drive']];
let diskRecords = [], libraryAssets = new Map(), diskDownloadBaseURL;
let profile, profiles, running = false, paused = false, starting = false;
let api, mounted = ['', ''], mountedNames = ['', ''], workingDisks = [new Map(),new Map()], sessionId = crypto.randomUUID();
let harddiskName = '', restoring = false, mounting = 0;
let noticeTimer;
const message = (text, level='info') => {
  const status=$('status'); status.textContent=text;
  clearTimeout(noticeTimer); delete status.dataset.highlight;
  status.dataset.level=level; status.setAttribute('aria-live',level==='info'?'polite':'assertive');
  if(level==='warning' || level==='error') {
    // Restart the short flash for a new notice, then hold for seven seconds.
    void status.offsetWidth;
    status.dataset.highlight='true';
    const flashTime=matchMedia('(prefers-reduced-motion: reduce)').matches?0:2000;
    noticeTimer=setTimeout(()=>delete status.dataset.highlight,flashTime+7000);
  }
};
const log = text => {
  $('log').textContent += String(text) + '\n';
  if ($('log').textContent.length > 30000) $('log').textContent = $('log').textContent.slice(-20000);
};
const fail = error => { message(error.message || String(error),'error'); log(error.stack || error); };
async function bytes(url, label='file') {
  const response = await fetch(url);
  if (!response.ok) throw new Error('Could not load ' + url + ' (' + response.status + ')');
  const encoding=response.headers.get('Content-Encoding');
  const total=encoding && encoding!=='identity'?0:Number(response.headers.get('Content-Length')), reader=response.body?.getReader();
  if(!reader) return new Uint8Array(await response.arrayBuffer());
  const chunks=[]; let loaded=0, lastUpdate=0;
  const progress=()=>message('Loading '+label+': '+(total?Math.min(100,Math.floor(loaded/total*100))+'% · ':'')+(loaded/1048576).toFixed(1)+(total?' / '+(total/1048576).toFixed(1):'')+' MB');
  progress();
  while(true) {
    const {done,value}=await reader.read(); if(done) break;
    chunks.push(value); loaded+=value.length;
    if(performance.now()-lastUpdate>150) {progress();lastUpdate=performance.now();}
  }
  progress();
  const data=new Uint8Array(loaded); let offset=0;
  for(const chunk of chunks) { data.set(chunk,offset);offset+=chunk.length; }
  return data;
}
async function floppyBytes(name) {
  const asset = libraryAssets.get(name);
  const url = asset ? (window.WANG_DISK_BASE_URL ? new URL(asset.objectKey,window.WANG_DISK_BASE_URL).href : asset.localPath) : 'media/'+name;
  message('Downloading '+name+'...');
  let data;
  try { data=await bytes(url,name); } catch(error) { throw new Error('Disk download failed: '+name+'. '+error.message+' Check the URL, connection and R2 CORS policy.'); }
  if(asset) {
    const hash = [...new Uint8Array(await crypto.subtle.digest('SHA-256',data))].map(n=>n.toString(16).padStart(2,'0')).join('');
    if(hash !== asset.sha256) throw new Error('Checksum mismatch for '+name+'; disk was not inserted.');
  }
  message('Downloaded and verified '+name);
  return normaliseFloppy(data,name);
}
const db = new Promise((resolve, reject) => {
  const request = indexedDB.open('wang-browser-disks', 1);
  request.onupgradeneeded = () => request.result.createObjectStore('sessions', {keyPath:'id'});
  request.onsuccess = () => resolve(request.result);
  request.onerror = () => reject(request.error);
});
db.catch(log);
async function database(action, value) {
  const store = (await db).transaction('sessions', action === 'put' ? 'readwrite' : 'readonly');
  return new Promise((resolve,reject) => {
    const req = store.objectStore('sessions')[action](...(value === undefined ? [] : [value]));
    store.oncomplete = () => resolve(req.result);
    store.onerror = () => reject(store.error);
    store.onabort = () => reject(store.error || new Error('Disk save aborted'));
  });
}
async function listSaves() {
  const saved = await database('getAll');
  $('saved').replaceChildren(new Option('Fresh starting disks',''));
  for (const entry of saved.filter(s => s.profile === profile.id).sort((a,b)=>b.stamp-a.stamp))
    $('saved').add(new Option(new Date(entry.stamp).toLocaleString(),entry.id));
}
function setProfile(p) {
  if (starting || running) return;
  profile = p;
  const url=new URL(location.href); url.searchParams.set('machine',p.id); history.replaceState(null,'',url.pathname+url.search);
  $('machine-name').textContent = p.name;
  $('details').textContent = p.description;
  $('start').disabled = !p.available;
  message(p.available ? 'Ready to start' : 'Awaiting hardware inventory');
  for (const button of $('machines').children) button.setAttribute('aria-current', button.dataset.id === p.id);
  $('drive-a').value = p.floppyA || '';
  $('drive-b').value = p.floppyB || '';
  $('drive-b-row').hidden = p.floppyCount < 2;
  $('drive-b').disabled = p.floppyCount < 2;
  $('upload-label').textContent = 'Load a local floppy into '+(p.floppyCount < 2 ? 'A' : 'B');
  $('winchester-panel').hidden = !p.winchester;
  $('winchester-choice-row').hidden = !p.available;
  $('winchester-spec').textContent = p.available ? "ST-225 - 20 MB. Changes to C are included when you save this session's disks. Choose the starting disk before starting the machine." : 'Winchester model and capacity await the hardware inventory.';
  $('winchester-image').disabled = !p.available;
  $('winchester-image').value = p.harddisk || 'ST225-ready.chd';
  harddiskName = p.harddisk || '';
  describeWinchester();
  updateDriveOptions(0);updateDriveOptions(1);
  displayDrive(0,p.floppyA || '');displayDrive(1,p.floppyB || '');
  listSaves().then(()=>{const saved=sessionStorage.getItem('wang-turned-off');if(saved) {sessionStorage.removeItem('wang-turned-off');if([...$('saved').options].some(o=>o.value===saved)) {$('saved').value=saved;$('saved').onchange();}message('Machine off. '+(saved==='discarded'?'Start a new session when ready.':'Your disks are saved; Start restores them.'));}}).catch(log);
}
function describeWinchester() {
  const notes = {
    'ST225-ready.chd':'Boots from C with Wang System 2.01 already installed. Floppy A starts empty.',
    'ST225-initialized.chd':'Initialized and empty. Boot System 2.01 in A, then install the OS onto C.',
    'ST225-uninitialized.chd':'No disk structure yet. Boot System 2.01 in A, enter DOS, swap A to Winchester Utilities (MFI), then run INITW.'
  };
  $('winchester-state').textContent = profile.available ? notes[$('winchester-image').value] : "Winchester media will be added with this machine's original ROMs.";
}
function normaliseFloppy(data, name) {
  if (name.toLowerCase().endsWith('.mfi')) {
    if (new TextDecoder().decode(data.slice(0,15)) !== 'MAMEFLOPPYIMAGE') throw new Error('Invalid MFI header');
    const view = new DataView(data.buffer,data.byteOffset,data.byteLength);
    const geometry = view.getUint32(16,true), cylinders = geometry & 0x3fffffff;
    if (cylinders > 42) {
      if ((geometry >>> 30) || cylinders > 84 || cylinders % 2) throw new Error('Unsupported captured floppy geometry');
      view.setUint32(16,0x40000000 + cylinders/2,true);
    }
  }
  return data;
}
function setPaused(value) {
  paused = value; api.pause(value ? 1 : 0);
  $('pause').textContent = value ? 'Resume' : 'Pause';
  message(value ? 'Paused' : 'Running in your browser');
}
function displayDrive(drive, name) {
  const select=$(drive?'drive-b':'drive-a'), toggle=$(drive?'drive-b-format':'drive-a-format');
  const format=libraryAssets.get(name)?.format || name.split('.').pop().toLowerCase();
  if(['mfi','img'].includes(format) && toggle.value!==format) setFormat(drive,format);
  updateDriveOptions(drive);
  if([...select.options].some(o=>o.value===name)) select.value=name;
}
function setFormat(drive, format) {
  const button=$(drive?'drive-b-format':'drive-a-format');
  button.value=format; button.dataset.format=format;
  button.setAttribute('aria-pressed',String(format==='mfi'));
  button.setAttribute('aria-label','Drive '+(drive?'B':'A')+' format: '+format.toUpperCase()+'. Switch to '+(format==='mfi'?'IMG':'MFI'));
  button.title=format==='mfi'?'MFI selected (recommended). Switch to IMG sector images.':'IMG selected. Switch to MFI flux images.';
  button.innerHTML=format==='mfi'?'<strong>MFI</strong> <span aria-hidden="true">⇄</span> IMG':'MFI <span aria-hidden="true">⇄</span> <strong>IMG</strong>';
  updateDriveOptions(drive);
}
function updateDriveOptions(drive) {
  const select=$(drive?'drive-b':'drive-a'), format=$(drive?'drive-b-format':'drive-a-format').value;
  const current=running?mountedNames[drive]:select.value;
  select.replaceChildren(new Option(running?'Choose a '+format.toUpperCase()+' disk…':(profile?.winchester?'No floppy (boot C)':'No floppy inserted'),''));
  for(const [file,image] of libraryAssets) {
    if(image.format!==format || (format==='mfi' && !image.preferred) || (!running && !image.bootable)) continue;
    select.add(new Option(image.label,file));
  }
  if(running) for(const name of workingDisks[drive].keys()) {
    if(name.toLowerCase().endsWith('.'+format) && ![...select.options].some(o=>o.value===name)) select.add(new Option(name,name));
  }
  select.value=[...select.options].some(o=>o.value===current)?current:'';
  const inserted=running?mountedNames[drive]:select.value;
  $(drive?'drive-b-state':'drive-a-state').textContent=inserted?(running?'Inserted: ':'Selected: ')+inserted:'Empty drive';
  $(drive?'eject-b':'eject-a').disabled=!inserted || drive>=(profile?.floppyCount || 0) || !!mounting || starting&&!running;
}
function updateBootOptions() {updateDriveOptions(0);updateDriveOptions(1);}
function showPowerLabel(off) {
  if(running) $('start').textContent=off?'Turn Off':'Machine running';
}
function enableControls() {
  running = true; starting=false; updateBootOptions();
  workingDisks.forEach((library,drive)=>{ for(const name of library.keys()) displayDrive(drive,name); });
  mountedNames.forEach((name,drive)=>displayDrive(drive,name));
  for (const element of document.querySelectorAll('[data-key],#pause,#reset,#save,#upload,#drive-a')) element.disabled = false;
  $('drive-b').disabled = profile.floppyCount < 2;
  $('winchester-image').disabled = true;
  if(profile.winchester) {
    if(![...$('winchester-image').options].some(o=>o.value===harddiskName)) $('winchester-image').add(new Option('Saved Winchester',harddiskName));
    $('winchester-image').value = harddiskName;
    if(restoring) $('winchester-state').textContent = 'Using your saved Winchester C, including its previous changes.';
  }
  $('saved').disabled = true;
  $('start').textContent = 'Machine running';
  $('start').disabled=false; $('start').classList.add('running');
  $('start').setAttribute('aria-label','Machine running. Turn off this Wang');
  message('Running in your browser');
  $('canvas').focus();
}
async function start() {
  if (starting || running || !profile.available) return;
  if(!$('saved').value && $('drive-a').value && !libraryAssets.get($('drive-a').value)?.bootable) {
    message('Bootability is not confirmed for this floppy. Boot C or choose a confirmed bootable disk first.','warning'); return;
  }
  starting = true; $('start').disabled = true;
  $('winchester-image').disabled = true;
  for (const button of $('machines').children) button.disabled = true;
  try {
    message('Loading firmware and your starting disks...');
    const restored = $('saved').value ? await database('get',$('saved').value) : null;
    restoring = !!restored;
    const files = [];
    for (const rom of profile.roms) files.push(['/roms/'+rom,await bytes('media/'+rom,'firmware '+rom)]);
    const a = $('drive-a').value, b = profile.floppyCount > 1 ? $('drive-b').value : '';
    harddiskName = $('winchester-image').value;
    if (restored) {
      for (const file of restored.files) files.push([file.path,new Uint8Array(file.bytes)]);
      mounted = [...restored.mounted];
      if(profile.floppyCount < 2) mounted[1] = '';
      harddiskName = restored.harddiskName || 'saved-winchester';
      mountedNames = restored.names || mounted.map(p=>p.split('/').pop().replace(/^[AB]-(?:\d+-)?/,''));
      workingDisks = restored.library ? restored.library.map(entries=>new Map(entries)) : mounted.map((path,i)=>new Map(path ? [[mountedNames[i],path]] : []));
      if(mounted[0] && !libraryAssets.get(mountedNames[0])?.bootable) { mounted[0]=''; mountedNames[0]=''; log('Saved floppy A retained in your disk library; reinsert after boot because bootability is not confirmed.'); }
    } else {
      mounted = [a ? '/session/media/A-'+a : '', b ? '/session/media/B-'+b : ''];
      mountedNames = [a,b];
      workingDisks = mounted.map((path,i)=>new Map(path ? [[mountedNames[i],path]] : []));
      if(a) files.push([mounted[0],await floppyBytes(a)]);
      if(b) files.push([mounted[1],await floppyBytes(b)]);
      if(profile.winchester) files.push(['/session/media/Winchester.chd',await bytes('media/'+harddiskName,'Winchester C')]);
    }
    const args = [profile.driver,'-bios',profile.bios,'-rompath','/roms',...profile.machineArgs,
      '-cfg_directory','/session/cfg','-nvram_directory','/session/nvram',
      '-diff_directory','/session/diff','-skip_gameinfo','-window','-resolution','800x600','-nothrottle','-noreadconfig','-video','accel','-sound','none','-nojoystick'];
    if(profile.winchester) args.push('-hard','/session/media/Winchester.chd');
    if(mounted[0]) args.push('-flop1',mounted[0]);
    if(profile.floppyCount > 1 && mounted[1]) args.push('-flop2',mounted[1]);
    const wasm=await bytes(window.WANG_WASM_URL || 'runtime/wangweb.wasm','Wang emulator');
    window.Module = {
      // This optimized runtime supports the instantiate hook, not wasmBinary.
      // Reuse the streamed download so slow connections fetch the WASM only once.
      instantiateWasm(imports,receive) {
        WebAssembly.instantiate(wasm,imports).then(({instance,module})=>{
          receive(instance,module); delete Module.instantiateWasm;
        }).catch(error=>fail(new Error('Could not start the emulator: '+error.message+'. Reload the page to try again.')));
        return {};
      },
      canvas:$('canvas'), arguments:args,
      print:log, printErr:log, locateFile:name=>name === 'wangweb.wasm' && window.WANG_WASM_URL ? window.WANG_WASM_URL : 'runtime/'+name,
      onAbort:reason=>fail(new Error('Emulator stopped: '+reason)),
      preRun:[function() {
        // SDL defaults to #window, swallowing keystrokes intended for page inputs.
        ENV.SDL_EMSCRIPTEN_KEYBOARD_ELEMENT = '#canvas';
        const fs = Module.FS;
        for(const dir of ['/roms','/session/media','/session/cfg','/session/nvram','/session/diff']) fs.mkdirTree(dir);
        for(const [path,data] of files) { fs.mkdirTree(path.slice(0,path.lastIndexOf('/'))); fs.writeFile(path,data); }
      }],
      onRuntimeInitialized() {
        api = {
          ready:Module.cwrap('wang_ready','number',[]),
          time:Module.cwrap('wang_time','number',[]),
          text:Module.cwrap('wang_text','string',[]),
          key:Module.cwrap('wang_key','number',['number','number','number']),
          floppy:Module.cwrap('wang_floppy','number',['number','string']),
          error:Module.cwrap('wang_last_error','string',[]),
          pause:Module.cwrap('wang_pause',null,['number']),
          reset:Module.cwrap('wang_reset',null,[]),
          flush:Module.cwrap('wang_flush',null,[])
        };
        const wait = setInterval(()=>{ if(api.ready()) { clearInterval(wait); enableControls(); } },100);
      }
    };
    message('Starting '+profile.name+'...');
    const script = document.createElement('script');
    script.src='runtime/wangweb.js';
    script.onerror=()=>fail(new Error('Browser emulator build is missing. Run the web build and staging steps.'));
    document.body.append(script);
  } catch(error) { fail(error); starting=false; $('start').disabled=false; }
}
async function mount(drive, name, localBytes) {
  if(!running || drive >= profile.floppyCount) return;
  const select = $(drive ? 'drive-b' : 'drive-a');
  mounting++; select.disabled = true;
  $(drive?'eject-b':'eject-a').disabled=true;
  try {
    let path = '';
    if(name) {
      path = !localBytes && workingDisks[drive].get(name);
      if(!path) {
        const data = localBytes ? normaliseFloppy(localBytes,name) : await floppyBytes(name);
        path = '/session/media/'+(drive?'B-':'A-')+Date.now()+'-'+name.replace(/[^\w.-]/g,'_');
        Module.FS.writeFile(path,data);
      }
    }
    if(!api.floppy(drive,path)) {
      const rawError=api.error() || 'Unable to change floppy';
      const isLayoutError=/identify image file format|incompatible image|corrupted data/i.test(rawError);
      const error=isLayoutError ? 'Image format not supported by MAME: '+name+'. The file was '+(localBytes?'read':'downloaded')+' successfully, but could not be mounted.' : rawError;
      const note=libraryAssets.get(name)?.formatNote;
      if(note) log(name+': '+note);
      log('MAME image loader: '+rawError);
      const restored=api.floppy(drive,mounted[drive] || '');
      throw new Error(error+(restored?' Previous disk restored.':' Unable to restore the previous disk; reinsert it.'));
    }
    if(name) workingDisks[drive].set(name,path);
    mounted[drive] = path;
    mountedNames[drive] = name;
    displayDrive(drive,name);
    message(name ? 'Inserted '+name+' in '+(drive?'B':'A') : 'Drive '+(drive?'B':'A')+' empty');
  } catch(error) { fail(error); }
  finally { mounting--; select.disabled=false; updateDriveOptions(drive); }
}
$('saved').onchange = ()=>{
  $('winchester-image').disabled = !!$('saved').value || !profile.available;
  if($('saved').value && profile.winchester) $('winchester-state').textContent = 'Your saved Winchester will be restored; the starting disk selection is ignored.';
  else describeWinchester();
};
$('winchester-image').onchange = ()=>{
  harddiskName = $('winchester-image').value;
  $('drive-a').value = harddiskName === 'ST225-ready.chd' ? '' : 'system-2.01.mfi';
  displayDrive(0,$('drive-a').value); describeWinchester();
};
$('start').onclick = ()=>{ if(!running) return start(); if(mounting) {message('Wait for the disk change to finish before turning off.','warning');return;} $('power-dialog').showModal(); $('power-save').focus(); };
$('start').onmouseenter=()=>showPowerLabel(true); $('start').onmouseleave=()=>showPowerLabel(document.activeElement===$('start'));
$('start').onfocus=()=>showPowerLabel(true); $('start').onblur=()=>showPowerLabel($('start').matches(':hover'));
$('pause').onclick = ()=>setPaused(!paused);
$('reset').onclick = ()=>{ if(mounted[0] && !libraryAssets.get(mountedNames[0])?.bootable) { message('Eject floppy A before Reset: its bootability is not confirmed.','warning'); return; } api.reset(); if(paused) setPaused(false); $('canvas').focus(); };
$('fullscreen').onclick = ()=>$('canvas').requestFullscreen().catch(fail);
$('canvas').onclick = ()=>$('canvas').focus();
for(const button of document.querySelectorAll('[data-key]')) {
  const [row,mask] = button.dataset.key.split(',').map(Number);
  button.onclick = ()=>{ if(!running || paused) return; api.key(row,mask,1); setTimeout(()=>api.key(row,mask,0),100); $('canvas').focus(); };
}
for(const [drive,id] of [[0,'a'],[1,'b']]) {
  $('drive-'+id+'-format').onclick=()=>setFormat(drive,$('drive-'+id+'-format').value==='mfi'?'img':'mfi');
  $('eject-'+id).onclick=()=>{if(running) mount(drive,'');else { $('drive-'+id).value='';updateDriveOptions(drive); }};
}
for(const [drive,id] of [[0,'a'],[1,'b']]) $('drive-'+id).onchange=()=>running?mount(drive,$('drive-'+id).value):updateDriveOptions(drive);
$('upload').onchange = async()=>{
  const file=$('upload').files[0]; if(!file) return;
  await mount(profile.floppyCount < 2 ? 0 : 1,file.name,new Uint8Array(await file.arrayBuffer()));
  $('upload').value='';
};
async function saveSession() {
  if(!running) return false;
  if(mounting) {message('Wait for the disk change to finish before saving.','warning');return false;}
  const wasPaused=paused; setPaused(true); $('save').disabled=true;
  try {
    api.flush();
    const fs=Module.FS, files=[];
    function walk(path) {
      for(const name of fs.readdir(path)) {
        if(name==='.' || name==='..') continue;
        const child=path+'/'+name, stat=fs.stat(child);
        if(fs.isDir(stat.mode)) walk(child); else files.push({path:child,bytes:fs.readFile(child)});
      }
    }
    walk('/session');
    await database('put',{id:sessionId,profile:profile.id,stamp:Date.now(),files,harddiskName,mounted:[...mounted],names:[...mountedNames],library:workingDisks.map(map=>[...map])});
    await listSaves();
    message('Disk files saved in this browser');
    return true;
  } catch(error) { fail(error);return false; }
  finally { if(!wasPaused) { paused=false; api.pause(0); $('pause').textContent='Pause'; } $('save').disabled=false; }
}
$('save').onclick=saveSession;
$('power-cancel').onclick=()=>$('power-dialog').close();
async function turnOff(save) {
  if(mounting) {message('Wait for the disk change to finish before turning off.','warning');return;}
  const buttons=$('power-dialog').querySelectorAll('button');buttons.forEach(b=>b.disabled=true);
  if(save && !await saveSession()) {buttons.forEach(b=>b.disabled=false);$('power-dialog').close();return;}
  api.pause(1);sessionStorage.setItem('wang-turned-off',save?sessionId:'discarded');location.reload();
}
$('power-save').onclick=()=>turnOff(true);
$('power-discard').onclick=()=>turnOff(false);
for(const id of ['quick-help','help-link']) $(id).onclick=()=>{$('quick-help-dialog').showModal();$('help-close').focus();};
$('help-close').onclick=()=>$('quick-help-dialog').close();
function openLibrary(id='') {
  $('library-search').value=id;
  $('library-filter').value=id?'all':'ready';
  renderLibrary(); $('disk-library').showModal(); $('library-search').focus();
}
function renderLibrary() {
  const query=$('library-search').value.trim().toLowerCase(), filter=$('library-filter').value, format=$('library-format').value;
  const visible=[];
  for(const disk of diskRecords) {
    const images=disk.images || (disk.emulator?[disk.emulator]:[]);
    for(const image of images.length?images:[null]) {
      if(image && format!=='all' && (image.format || image.file.split('.').pop())!==format) continue;
      if(filter==='ready' && !image) continue;
      if(filter==='validated' && image?.status!=='tested') continue;
      if(filter==='bootable' && !image?.bootable) continue;
      if(filter==='captured' && !disk.captured) continue;
      if(![disk.id,disk.label,disk.notes,image?.label,image?.file].join(' ').toLowerCase().includes(query)) continue;
      visible.push({disk,image});
    }
  }
  $('library-count').textContent=visible.length+' images / records shown';
  $('library-results').replaceChildren();
  for(const {disk,image} of visible) {
    const row=document.createElement('tr');row.dataset.disk=disk.id;if(image) row.dataset.image=image.file;
    const availability=image?(image.availability || 'Validated'):disk.captured?'No non-empty IMG available':disk.catalogStatus+'; no local capture';
    const boot=image?(image.bootable?'Bootable':'Not confirmed'):'—';
    for(const text of [disk.id,(image?.status==='tested'?image.label:disk.label)+(image?' — '+image.file:''),availability,boot]) {
      const cell=document.createElement('td');cell.textContent=text;row.append(cell);
    }
    if(image?.formatNote) { const note=document.createElement('small');note.className='image-format-note';note.textContent=image.formatNote;row.children[2].append(note); }
    const actions=document.createElement('td');actions.className='library-actions';
    if(image) for(let drive=0;drive<profile.floppyCount;drive++) {
      const button=document.createElement('button');button.textContent='Insert in '+(drive?'B':'A');
      button.disabled=!profile.available || (!running && !image.bootable);
      button.title=!running && !image.bootable?'Bootability not confirmed. Boot C or a confirmed system disk first.':'';
      button.onclick=async()=>{
        button.disabled=true;
        if(running) await mount(drive,image.file); else displayDrive(drive,image.file);
        $('disk-library').close();
      };
      actions.append(button);
    }
    if(image && !running && !image.bootable) {
      const note=document.createElement('small');note.textContent='Insert after boot; bootability is not confirmed.';actions.append(note);
    }
    const link=document.createElement('a');link.href=disk.recoveryURL;link.textContent=disk.id.startsWith('WANG')?'Recovery record':'Software archive';link.target='_blank';link.rel='noopener';actions.append(link);
    row.append(actions);$('library-results').append(row);
  }
}
$('browse-library').onclick=()=>openLibrary();
$('library-close').onclick=()=>$('disk-library').close();
$('library-search').oninput=renderLibrary;
$('library-filter').onchange=renderLibrary;
$('library-format').onclick=()=>{const button=$('library-format');const format=button.value==='mfi'?'img':'mfi';button.value=format;button.dataset.format=format;button.setAttribute('aria-pressed',String(format==='mfi'));button.setAttribute('aria-label','Library format: '+format.toUpperCase()+'. Switch to '+(format==='mfi'?'IMG':'MFI'));button.innerHTML=format==='mfi'?'<strong>MFI</strong> <span aria-hidden="true">⇄</span> IMG':'MFI <span aria-hidden="true">⇄</span> <strong>IMG</strong>';renderLibrary();};
(async()=>{
  const release=await (await fetch('version.json')).json();
  $('release-version').textContent='v'+release.version;
  $('release-version').title=release.type+' release, revision '+release.revision;
  document.title='Use a Wang '+release.version+' — Save The WANG';
  profiles=await (await fetch('profiles/machines.json')).json();
  const manifest=await (await fetch('profiles/disks.json')).json();
  diskRecords=manifest.disks; diskDownloadBaseURL=manifest.downloadBaseURL;
  for(const disk of diskRecords) for(const image of disk.images || (disk.emulator?[disk.emulator]:[])) {
    if(libraryAssets.has(image.file)) throw new Error('Duplicate disk-library filename: '+image.file);
    libraryAssets.set(image.file,image); catalog.push([image.file,image.label]);
  }
  $('library-summary').textContent = manifest.stats.physicalRecords+' numbered catalog records; '+manifest.stats.capturedDisks+' captured; '+manifest.stats.availableImages+' available images: '+manifest.stats.mfiImages+' MFI flux images and '+manifest.stats.experimentalImages+' experimental IMGs (plus native system media).' ;
  const requestedDisk=new URLSearchParams(location.search).get('disk');
  for(const select of [$('drive-a'),$('drive-b')]) {
    for(const [value,label] of catalog) select.add(new Option(label,value));
    select.disabled=false;
  }
  for(const p of profiles) {
    const button=document.createElement('button');
    button.dataset.id=p.id;
    const title=document.createElement('strong'); title.textContent=p.name;
    const subtitle=document.createElement('small'); subtitle.textContent=(p.floppyCount>1?'A + B':'A + C')+' · '+(p.available?'Browser prototype':'ROMs pending');
    button.append(title,subtitle); button.onclick=()=>setProfile(p); $('machines').append(button);
  }
  setProfile(profiles.find(p=>p.id===new URLSearchParams(location.search).get('machine')) || profiles[0]);
  if(requestedDisk) openLibrary(requestedDisk);
})().catch(fail);

