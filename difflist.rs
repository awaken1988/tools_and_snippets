use std::io;
use std::fs::{self, DirEntry};
use std::path::Path;
use std::path::PathBuf;
use std::collections::HashMap;
use std::collections::HashSet;
use std::fmt;
use std::rc::Rc;
use std::cell::RefCell;
use std::num::Wrapping;

use pathtree::*;

#[derive(Debug,Clone)]
pub enum DiffCause
{
    ADDED,
    REMOVED,
    FILE_TIME,
    FILE_SIZE,
    DIR_TO_FILE,
    FILE_TO_DIR,
    FILESIZE(i64),
    MODIFIED_TIME,
}


pub struct DiffItem
{
    pub fs_item: PathTree,
    pub cause: DiffCause,
}

impl fmt::Display for DiffItem {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {

        let mut filetype = String::from("FILE");

        if self.fs_item.is_dir {
            filetype = String::from("DIR ")
        }

        let cause = format!("{:?}", self.cause);
        write!(f, "{} | {: <16} | {}",filetype, cause, PathTree::full_path(&self.fs_item) );

        return Ok(());
    }
}
