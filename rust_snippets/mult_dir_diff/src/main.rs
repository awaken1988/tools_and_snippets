

use std::path::Path;
use std::fs::read_dir;
use std::fs::DirEntry;

fn list_dir(dirs: &[&Path], 
            fun_file: &Fn(DirEntry), 
            fun_dir: &Fn(DirEntry)) 
{
    for iPath in dirs {
        println!("{:?}",iPath);
        for iEntry in read_dir(iPath).unwrap() {
            let iEntry = iEntry.unwrap();
            let iMeta  = iEntry.metadata().unwrap();
            
            if iMeta.is_dir() {
                fun_dir(iEntry);
            }
            else if iMeta.is_file() {
                fun_file(iEntry);
            }
            else {
                println!("[WRN] ignored: {:?}", iEntry);
            }
        }
    }
}

fn main() {
    println!("Hello, world!");


    let dirs = [Path::new("/home/martin/"), ];
    list_dir( &dirs, 
    &|file| {
        println!("[DBG] add file: {:?}", &file.path());
    },
    &|dir| {
        println!("[DBG] add dir:  {:?}", &dir.path());
    }  );

}
