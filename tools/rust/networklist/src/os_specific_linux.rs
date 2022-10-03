
    use std::process::Command;
    use std::io::{stdout, Write};
    use std::fs;
    use std::ffi::OsString;
    use std::ffi::OsStr;
    use tempfile;
    use std::fmt::Write as FmtWrite;

    use crate::NetlistItem;

    #[link(name = "c")]
    extern "C" {
        fn geteuid() -> u32;
        fn getegid() -> u32;
    }

    pub fn prepare() {
        let mut my = String::new(); 
        
        unsafe {
            my = format!("/run/user/{}/nlmnt", geteuid());
        }

        std::fs::create_dir_all(&my);
    }

    pub fn execute_script(aScriptContent: &str) -> (bool, i32, String) {
        let mut exec_path: String;
        {
            let mut tmpscript = tempfile::Builder::new().suffix(".sh").tempfile().unwrap();
            let (mut file, mut path) = tmpscript.keep().unwrap();
            exec_path = path.to_str().unwrap().to_string();
            let mut psdrive_additional = String::new();

            write!(file, "{}", aScriptContent);
        }

        let result = Command::new("bash")
            .arg(exec_path.clone())
            .output().unwrap();

        let mut output_str = exec_path;

        output_str += aScriptContent;
        output_str += &String::from_utf8_lossy(&result.stdout[..]);
        output_str += &String::from_utf8_lossy(&result.stderr[..]);
        output_str += "------------------------\n";

        return (result.status.success(), result.status.code().unwrap_or(-1), output_str);
    }

    pub fn handle_smb(aItem: &NetlistItem) -> (bool, i32, String) {
        let mut ps_script = String::new();
        let mut psdrive_additional = String::new();

        write!(ps_script, "#!/bin/bash\n");
        write!(ps_script, "# {} \n", aItem.parsed._original);

        let mut options = Vec::new();

        if let Some(user) = &aItem.parsed.user {
            options.push("username=".to_string()+&user.clone());
        }

        write!(ps_script, "mount -t cifs");

        if !options.is_empty() {
            write!(ps_script, " -o {} ", options.join("-"));
        }

        //remote
        write!(ps_script, " \"//{host}{path}\"",  
            host=aItem.parsed.host,
            path=aItem.parsed.path);           

        //mountpoint
        write!(ps_script, "{}{}", aItem.parsed.host, aItem.parsed.path);


       return execute_script(&ps_script);
    }

    pub fn handle_ssh(aItem: &NetlistItem) -> (bool, i32, String) {
        let mut ps_script = String::new();

        write!(ps_script, "#!/bin/bash\n");
        write!(ps_script, "# {} \n", aItem.parsed._original);
        write!(ps_script, "# host={} \n", aItem.parsed.host);
        
        if let Some(user) = &aItem.parsed.user {
            write!(ps_script, "# host={} \n", user);
        }
        write!(ps_script, "ssh ");
        if let Some(user) = &aItem.parsed.user {
            write!(ps_script, "{}@", user);
        }
        write!(ps_script, "{}", aItem.parsed.host);

        if let Some(port) = aItem.parsed.port {
            write!(ps_script, "\"-p\" \"{}\"", port);
        }

        return execute_script(&ps_script);
    }
