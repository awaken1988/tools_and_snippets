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
use std::cmp::Ordering;

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
    #[derive(Debug)]
    enum Order {NAME, TIME};

    let mut left = String::new();
    let mut right = String::new();

    let mut ord = Order::NAME;



    for (i, arg) in env::args().enumerate() {
        if 0 == i {continue;}

        match &arg {

            _ if arg.starts_with("--order=") => {
                let x = arg.replace("--order=", "");
                println!("{:?}", x);
                match(x)  {
                    _ if x == "name" => {
                        ord = Order::NAME;
                    }
                    _ if x == "time" => {
                        ord = Order::TIME;
                    }
                    _ => {
                        panic!("argument for --order is wrong");
                    }
                }
            }

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
   
    println!("diff {:?}", left);
    println!("diff {:?}", right);
    PathTree::compare_dir(&*left_tree.borrow(), &*right_tree.borrow(), &mut diff_list );

    println!("{:?}", ord);

    match ord {
        Order::NAME => {   
            diff_list.sort_by(|a0,a1| a0.fs_item.path.cmp(&a1.fs_item.path));   
        }
        Order::TIME => {
            panic!("not implemented");
            diff_list.sort_by(|a0: &DiffItem,a1: &DiffItem| {

                //let a0_meta = fs:metadata(a0.fs_item.path.clone());
                //let a1_meta = fs:metadata(a1.fs_item.path.clone());

                Ordering::Equal
            });
        }
    }
    

    for iDiff in diff_list.iter() {
       println!("{}", iDiff);
    }
}