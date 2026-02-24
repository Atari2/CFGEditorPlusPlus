using System.IO;
using System.IO.Compression;
using IWshRuntimeLibrary;
using System.Diagnostics;
using System.Reflection;

namespace ICFGEditor {
    public static class Methods {
        public static void ExtractUnzip(string installPathText) {
            var prop = Properties.Resources.CFGEditor;
            var zipath = Path.Combine(installPathText, "CFGEditor.zip");
            var destpath = Path.Combine(installPathText, "CFGEditor");
            {
                using (var fileStream = System.IO.File.Create(zipath)) {
                    fileStream.Write(prop, 0, prop.Length);
                }
            }
            ZipFile.ExtractToDirectory(zipath, destpath);
            System.IO.File.Delete(zipath);
            WshShell shell = new WshShell();
            string shortcutAddress = Path.Combine(installPathText, "CFGEditor.lnk");
            IWshShortcut shortcut = shell.CreateShortcut(shortcutAddress) as IWshShortcut;
            shortcut.Description = "Shortcut for CFGEditor";
            shortcut.TargetPath = $"{destpath}/CFGEditorPlusPlus.exe";
            shortcut.Save();
        }
    }
    static class Program {
        static void Main() {
            string installPathText = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
            Methods.ExtractUnzip(installPathText);
            ProcessStartInfo info = new ProcessStartInfo(Path.Combine(installPathText, "CFGEditor.lnk")) {
                UseShellExecute = true
            };
            Process.Start(info);
        }
    }
}
