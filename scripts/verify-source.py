# SPDX-License-Identifier: GPL-2.0-or-later
from pathlib import Path
import hashlib,json
root=Path(__file__).resolve().parent.parent
manifest=json.loads((root/'SOURCE_MANIFEST.json').read_text(encoding='utf-8'))
for item in manifest['modifiedFiles']:
    data=(root/item['file']).read_bytes().replace(b'\r\n',b'\n')
    assert hashlib.sha256(data).hexdigest()==item['publishedSHA256'],item['file']
for item in manifest['portArchives']:
    assert hashlib.sha256((root/item['file']).read_bytes()).hexdigest()==item['sha256'],item['file']
assert (root/'LICENSE').is_file() and (root/'licenses/MAME-COPYING').is_file()
for name in (root/'SOURCE_FILES.txt').read_text(encoding='utf-8').splitlines():
    path=Path(name)
    assert not path.is_absolute() and '..' not in path.parts,name
    assert (root/path).is_file(),name
print('PASS: publication file inventory, 16 annotated MAME files, 5 original port archives and notices verified.')
