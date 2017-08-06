use std::io;
use std::fs::{self, DirEntry};
use std::path::Path;

fn walkdir(dir: &Path)
{
    if dir.is_file()  {
        println!("{}", dir.to_str().unwrap());
    }
}

fn main()
{
    walkdir(Path::new("/usr/bin/"));
}
