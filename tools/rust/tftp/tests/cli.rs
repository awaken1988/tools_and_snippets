use assert_cmd::prelude::*; // Add methods on commands
use predicates::prelude::*; // Used for writing assertions
use std::{process::Command, thread}; // Run programs
use assert_fs::prelude::*;

#[test]
fn client_server_read() -> Result<(), Box<dyn std::error::Error>> {
    let server_root = assert_fs::TempDir::new().unwrap();
    let client_root = assert_fs::TempDir::new().unwrap();

    let transfer_path = server_root.child("download.bin");    
    transfer_path.write_binary(&[0,1,2,3]).unwrap();

    let mut cmd_path = Command::cargo_bin("tftp").unwrap().get_program().to_os_string();

    //start server
    let server_hndl = {
        let mut cmd_path = cmd_path.clone();
        let server_root = server_root.path().to_path_buf();
        thread::spawn(move || {    
            std::process::Command::new(cmd_path)
                .arg("server")
                .arg("--rootdir").arg(server_root)
                .arg("--exit-with-client")
                .output().unwrap();
        })
    };
    let client_hndl = {
        let mut cmd_path = cmd_path.clone();
        thread::spawn(move || {    
            std::process::Command::new(cmd_path)
                .arg("client")
                .arg("--exit-with-client")
                .output().unwrap();
        })
    };

    server_hndl.join();
    client_hndl.join();

    Ok(())
}