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

use difflist::*;

#[derive(Debug)]
pub struct PathTree {
    pub children: HashMap<String, Rc<RefCell<PathTree>>>,
    pub parent: Option<Rc<RefCell<PathTree>>>,
    pub name : String,
    pub is_dir : bool,
    pub path: PathBuf,
}

impl Clone for PathTree {
    fn clone(&self) -> PathTree
    {
        PathTree{
            children: self.children.clone(),
            parent: self.parent.clone(),
            name: self.name.clone(),
            is_dir: self.is_dir.clone(),
            path: self.path.clone(),
        }
    }
}

impl PathTree {
    pub fn walkprint(&self, depth: u32, f: &mut fmt::Formatter)
    {
        for i in 0..depth {
            write!(f, "    ");
        }

        write!(f, "{}\n", self.name);

        if self.is_dir {
            for child in self.children.values() {
                child.borrow().walkprint( depth+1, f );
            }
        }
    }

    pub fn walkdir(dir: &Path) -> Rc<RefCell<PathTree>>
    {
        let mut ret = Rc::new(RefCell::new(PathTree{ name: dir.file_name().unwrap().to_string_lossy().to_string(),
                                parent: None,
                                is_dir: false,
                                children: HashMap::new(),
                                path: dir.to_path_buf()}));
        if dir.is_file()  {
            //println!("{}", dir.to_str().unwrap());

        } else {
            let dir_entries =  match fs::read_dir(dir.to_str().unwrap()) {
                Ok(result) => result,
                Err(e) => return ret
            };

            ret.borrow_mut().is_dir = true;

            for entry in dir_entries {
                let child = PathTree::walkdir(& entry.unwrap().path());

                let name = child.borrow().name.clone();

                ret.borrow_mut().children.insert(name, child.clone());
                child.borrow_mut().parent = Some(ret.clone());
            }
        }
        return ret;
    }

    pub fn add_subdir(path: &PathTree, cause: DiffCause, diff_list: &mut Vec<DiffItem>)
    {
        if( !path.is_dir ) {
            return;
        }

        for (iPath, iChild) in path.children.iter() {
            let curr_child = iChild.borrow();
            if !curr_child.is_dir {
                diff_list.push( DiffItem{ fs_item: curr_child.clone(), cause: cause.clone() }) ;
            } else {
                PathTree::add_subdir(&curr_child, cause.clone(), diff_list);
            }
        }
    }

    fn compare_file(left: &PathTree,
                    right: &PathTree,
                    diff_list: &mut Vec<DiffItem>) -> bool
    {
        if let (Ok(left_meta), Ok(right_meta)) =
            (fs::metadata(left.path.clone()), fs::metadata(right.path.clone())){

                let is_file_only = left_meta.is_file() && right_meta.is_file();

                //check if file size changed
                if( left_meta.len() != right_meta.len()
                    && is_file_only  ) {
                    let leftnum = Wrapping(left_meta.len() as i64);
                    let rightnum = Wrapping(right_meta.len() as i64);

                    diff_list.push( DiffItem{ fs_item: left.clone(), cause: DiffCause::FILESIZE((rightnum-leftnum).0) }) ;
                    return true;
                }

                //check for dateimte
                if is_file_only  {
                    let left_time = left_meta.modified().unwrap();
                    let right_time = right_meta.modified().unwrap();

                    if( left_time != right_time && left_meta.is_file() && right_meta.is_file()) {
                        diff_list.push( DiffItem{ fs_item: left.clone(), cause: DiffCause::MODIFIED_TIME }) ;
                        return true;
                    }
                }
        } else {
            diff_list.push( DiffItem{ fs_item: left.clone(), cause: DiffCause::ACCESS_ERROR }) ;
            return true;
        }

        return false;
    }

    pub fn compare_dir(left: &PathTree, right: &PathTree, diff_list: &mut Vec<DiffItem>)
    {
        let mut diff_map : HashSet<String> = HashSet::new();
        let sides = [(left, right), (right, left)];

        for (iNum, iSide) in sides.iter().enumerate()
        {
            let prefix = { if 0 == iNum {"- "} else { "+ " } };
            
            for (iPath, iChild) in iSide.0.children.iter() {

                if diff_map.contains(&iPath.clone()) {
                    continue;
                };
                diff_map.insert(iPath.clone());

                let curr_left = iChild.borrow();
                //check if file/dir is avail on each side
                
                println!("{:?} {:?}", iNum, iChild.borrow().path );
                
                if !iSide.1.children.contains_key(iPath)  {

                    //println!("{}{}",prefix,  PathTree::full_path(&*curr_left));
                    if 0 == iNum {
                        diff_list.push( DiffItem{ fs_item: curr_left.clone(), cause: DiffCause::REMOVED }) ;
                        PathTree::add_subdir(&*curr_left, DiffCause::REMOVED, diff_list);
                        continue;
                    } else {
                        diff_list.push( DiffItem{ fs_item: curr_left.clone(), cause: DiffCause::ADDED }) ;
                        PathTree::add_subdir(&*curr_left, DiffCause::ADDED, diff_list);
                        continue;
                    }
                }

                let curr_right = iSide.1.children[iPath].borrow();

                if 0 == iNum {

                    // dir changed to file or otherwise
                    if( curr_left.is_dir && !curr_right.is_dir ) {
                        diff_list.push( DiffItem{ fs_item: curr_left.clone(), cause: DiffCause::DIR_TO_FILE }) ;
                        continue;
                    } else if ( !curr_left.is_dir && curr_right.is_dir ) {
                        diff_list.push( DiffItem{ fs_item: curr_left.clone(), cause: DiffCause::FILE_TO_DIR }) ;
                        continue; 
                    }

                    //compare file
                    //if !curr_left.is_dir && PathTree::compare_file(&*curr_left, &*curr_right, diff_list) {
                    //    continue;
                    //}

                    PathTree::compare_dir(&*iSide.0.children[iPath].borrow(), &*iSide.1.children[iPath].borrow(), diff_list);
                }
            }

        }

    }

    pub fn full_path(ptree: &PathTree) -> String
    {
        let mut ret = ptree.name.clone();use std::io;

        match (ptree.parent.as_ref()) {
            None    => return String::new(),
            Some(x) => {
                let inner_str = PathTree::full_path(&*x.borrow());
                let ref separator = { if inner_str.len() < 1 {""} else {"/"}};

                return format!("{}{}{}", inner_str, separator, ret);
            }
        }

        return ret;
    }
}

impl fmt::Display for PathTree {

    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        self.walkprint(0, f);

        return Ok(());
    }
}
