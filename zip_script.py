from zipfile import ZipFile, ZIP_DEFLATED
import glob
import sys
from pathlib import Path

build_dir = Path(sys.argv[1]) if len(sys.argv) > 1 else Path('build')

executable_path = build_dir / 'CFGEditorPlusPlus.exe'
core_dlls = glob.glob(str(build_dir / 'Qt6*.dll'))
imageformats = glob.glob(str(build_dir / 'imageformats' / '*.dll'))
platforms = glob.glob(str(build_dir / 'platforms' / '*.dll'))
styles = glob.glob(str(build_dir / 'styles' / '*.dll'))

all_dlls = [*core_dlls, *imageformats, *platforms, *styles]

print(all_dlls)

with ZipFile('CFGEditor.zip', 'w', ZIP_DEFLATED) as zpf:
	for dll in all_dlls:
		zipname = str(Path(dll).relative_to(build_dir))
		zpf.write(dll, arcname=zipname)
	zpf.write(str(executable_path), arcname='CFGEditorPlusPlus.exe')

print('Zip created')
