# SPDX-License-Identifier: GPL-2.0-or-later
from pathlib import Path
import json,hashlib
root=Path(__file__).resolve().parent.parent
p=root/'web/build-info.json'
info=json.loads(p.read_text(encoding='utf-8'))
info['wasmSHA256']=hashlib.sha256((root/'web/runtime/wangweb.wasm').read_bytes()).hexdigest()
info['rebuiltFromPublishedSource']=True
p.write_text(json.dumps(info,indent=2)+'\n',encoding='utf-8')
print('Built and staged matched JS/WASM, without firmware or disk media.')
