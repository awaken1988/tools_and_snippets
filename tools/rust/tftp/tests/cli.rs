use assert_cmd::prelude::*; // Add methods on commands
use predicates::prelude::*; // Used for writing assertions
use std::{process::Command, thread, time::Duration, fs::{File, self}, path::Path}; // Run programs
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
            println!("server started");
            std::process::Command::new(cmd_path)
                .arg("server")
                .arg("--rootdir").arg(server_root)
                .arg("--exit-with-client")
                .output().unwrap();
        })
    };

    thread::sleep(Duration::from_secs(2));

    let client_hndl = {
        let mut cmd_path = cmd_path.clone();
        thread::spawn(move || {    
            println!("client started");
            std::process::Command::new(cmd_path)
                .arg("client")
                .arg("--remote").arg("127.0.0.1")
                .arg("--read").arg("download.bin")
                .output().unwrap();
        })
    };

    
    client_hndl.join();
    println!("client ready");
    
    server_hndl.join();
    println!("server ready");

    let server_file_path = server_root.join("download.bin");
    let client_file_path = client_root.join("download.bin");

    compare(&server_file_path, &client_file_path);

    Ok(())
}

fn compare(l: &Path, r: &Path) -> bool {
    let l = fs::read(l).unwrap();
    let r = fs::read(r).unwrap();  

    return l == r;
}