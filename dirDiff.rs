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
use std::env;

mod difflist;
mod pathtree;

use pathtree::*;
use difflist::*;

//-----------------------------------------------------------
//
// PathTree stores the directory structure
//
//-----------------------------------------------------------




fn main()
{
    let mut left = String::new();
    let mut right = String::new();

    for (i, arg) in env::args().enumerate() {
        if 0 == i {continue;}

        match &arg {

            _ => {
                if left.is_empty() {
                    left = arg;
                } else {
                    right = arg;
                }
            }

        }
    }

    let left_tree = PathTree::walkdir(Path::new(&left));
    let right_tree = PathTree::walkdir(Path::new(&right));

    let mut diff_list: Vec<DiffItem> = Vec::new();
    PathTree::compare_dir(&*left_tree.borrow(), &*right_tree.borrow(), &mut diff_list );

    for iDiff in diff_list.iter() {
        println!("{}", iDiff);
    }
}
