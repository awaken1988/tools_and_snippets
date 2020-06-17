
    use std::process::Command;
    use std::io::{stdout, Write};
    use std::fs;
    use std::ffi::OsString;
    use std::ffi::OsStr;
    use tempfile;
    use std::fmt::Write as FmtWrite;

    use crate::NetlistItem;

    pub fn execute_script(aScriptContent: &str) -> (bool, String) {
        let mut exec_path: String;
        {
            let mut tmpscript = tempfile::Builder::new().suffix(".ps1").tempfile().unwrap();
            let (mut file, mut path) = tmpscript.keep().unwrap();
            exec_path = path.to_str().unwrap().to_string();
            let mut psdrive_additional = String::new();

            write!(file, "{}", aScriptContent);
        }

        let result = Command::new("powershell.exe")
            .arg("-ExecutionPolicy").arg("Bypass")
            .arg("-File").arg(exec_path)
            .output().unwrap();

        if result.status.success() {
            return (true, String::from_utf8_lossy(&result.stdout[..]).to_string())
        }
        else {
            return (false, String::from_utf8_lossy(&result.stderr[..]).to_string())
        }
    }

    pub fn handle_smb(aItem: &NetlistItem) {
        let mut ps_script = String::new();
        let mut psdrive_additional = String::new();

        write!(ps_script, "# {} \n", aItem.parsed._original);

        if let Some(user) = &aItem.parsed.user {
            write!(ps_script, "$cred = Get-Credential -UserName '{user}' -Message 'Password' \n", user=user);
            psdrive_additional = format!("{} -Credential $cred", psdrive_additional);
        }

        write!(ps_script, "New-PSDrive -Persist -PSProvider 'FileSystem' {} ", psdrive_additional);

        if let Some(drive_letter) = aItem.name_value.get("drive") {
            write!(ps_script, "-Name '{drive}' ", drive=drive_letter);
        }

        write!(ps_script, "-Root '\\\\{host}{path}' \n",  
            host=aItem.parsed.host,
            path=aItem.parsed.path.replace("/", "\\"),);           

        let (success, output) = execute_script(&ps_script);
        
        //use erro type here no (bool, ...) tuple
        // if !success {
        //
        //}

        //aOutput.add_row( vec![ String::from_utf8_lossy(&result.stdout[..]).to_string() ] );
        //aOutput.add_row( vec![ String::from_utf8_lossy(&result.stderr[..]).to_string() ] );
        //aOutput.incremsent_selection(1000);
    }

    pub fn handle_ssh(aItem: &NetlistItem) {
        let mut ps_script = String::new();

        write!(ps_script, "# {} \n", aItem.parsed._original);
        write!(ps_script, "# host={} \n", aItem.parsed.host);
        if let Some(user) = &aItem.parsed.user {
            write!(ps_script, "# host={} \n", user);
        }
        
        write!(ps_script, "Start-Process -FilePath ssh -ArgumentList @(\"");
        if let Some(user) = &aItem.parsed.user {
            write!(ps_script, "{}@", user);
        }
        write!(ps_script, "{}", aItem.parsed.host);
        write!(ps_script, "\"");

        if let Some(port) = aItem.parsed.port {
            write!(ps_script, ", \"-p\", \"{}\"", port);
        }

        write!(ps_script, ")");

        execute_script(&ps_script);
    }
