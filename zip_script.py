from zipfile import ZipFile
import glob
from pathlib import Path

executable_path = 'build/CFGEditorPlusPlus.exe'
core_dlls = glob.glob('build/Qt6*.dll')
imageformats = glob.glob('build/imageformats/*.dll')
platforms = glob.glob('build/platforms/*.dll')
styles = glob.glob('build/styles/*.dll')

all_dlls = [*core_dlls, *imageformats, *platforms, *styles]

print(all_dlls)

with ZipFile('CFGEditor.zip', 'w') as zpf:
	for dll in all_dlls:
		zipname = dll.split('/', 1)[-1]
		zpf.write(dll, arcname=zipname)
	zpf.write(executable_path, arcname='CFGEditorPlusPlus.exe')
