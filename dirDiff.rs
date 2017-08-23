use std::io;
use std::fs::{self, DirEntry};
use std::path::Path;
use std::path::PathBuf;
use std::collections::HashMap;
use std::collections::HashSet;
use std::fmt;
use std::rc::Rc;
use std::cell::RefCell;

#[derive(Debug)]
enum DiffCause
{
    ADDED,
    REMOVED,
    FILE_TIME,
    FILE_SIZE,
    DIR_TO_FILE,
    FILE_TO_DIR,
}


struct DiffItem
{
    fs_item: PathTree,
    cause: DiffCause,
}

impl fmt::Display for DiffItem {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {

        write!(f, "{:?} {}", self.cause, PathTree::full_path(&self.fs_item) );

        return Ok(());
    }
}

//-----------------------------------------------------------
//
// PathTree stores the directory structure
//
//-----------------------------------------------------------
#[derive(Debug)]
struct PathTree {
    children: HashMap<String, Rc<RefCell<PathTree>>>,
    parent: Option<Rc<RefCell<PathTree>>>,
    name : String,
    is_dir : bool,
    path: PathBuf,
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
    fn walkprint(&self, depth: u32, f: &mut fmt::Formatter)
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

    fn walkdir(dir: &Path) -> Rc<RefCell<PathTree>>
    {
        let mut ret = Rc::new(RefCell::new(PathTree{ name: dir.file_name().unwrap().to_str().unwrap().to_string(),
                                parent: None,
                                is_dir: false,
                                children: HashMap::new(),
                                path: dir.to_path_buf()}));
        if dir.is_file()  {
            //println!("{}", dir.to_str().unwrap());

        } else {
            let dir_entries = fs::read_dir(dir.to_str().unwrap()).unwrap();
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

    fn compare_dir(left: &PathTree, right: &PathTree, diff_list: &mut Vec<DiffItem>)
    {
        let mut diff_map : HashSet<String> = HashSet::new();
        let sides = [(left, right), (right, left)];

        for (iNum, iSide) in sides.iter().enumerate()
        {
            if diff_map.contains(&iSide.0.name.clone()) {
                continue;
            };

            let prefix = { if 0 == iNum {"- "} else { "+ " } };
            diff_map.insert(iSide.0.name.clone());
            for (iPath, iChild) in iSide.0.children.iter() {
                let curr_left = iChild.borrow();
                //check if file/dir is avail on each side
                if !iSide.1.children.contains_key(iPath)  {

                    println!("{}{}",prefix,  PathTree::full_path(&*curr_left));
                    if 0 == iNum {
                        diff_list.push( DiffItem{ fs_item: curr_left.clone(), cause: DiffCause::REMOVED }) ;
                    } else {
                        diff_list.push( DiffItem{ fs_item: curr_left.clone(), cause: DiffCause::ADDED }) ;
                    }
                    continue;
                }

                let curr_right = iSide.1.children[iPath].borrow();

                //check file/dir changed
                if( iNum == 0 && curr_left.is_dir != curr_right.is_dir) {
                    let cause = {   if curr_left.is_dir {DiffCause::DIR_TO_FILE}
                                    else {DiffCause::FILE_TO_DIR}};
                    diff_list.push( DiffItem{ fs_item: curr_left.clone(), cause: cause }) ;
                }

                PathTree::compare_dir(&*iSide.0.children[iPath].borrow(), &*iSide.1.children[iPath].borrow(), diff_list);

            }
        }

    }

    fn full_path(ptree: &PathTree) -> String
    {
        let mut ret = ptree.name.clone();

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








fn main()
{
    let left = PathTree::walkdir(Path::new("./left"));
    let right = PathTree::walkdir(Path::new("./right"));

    //example print dir
    //println!("{}", *left.borrow());
    //println!("{}", *right.borrow());

    //example: print type of variable
    //let ddd: () = *left.borrow();

    let mut diff_list: Vec<DiffItem> = Vec::new();
    PathTree::compare_dir(&*left.borrow(), &*right.borrow(), &mut diff_list );

    for iDiff in diff_list.iter() {
        println!("{}", iDiff);
    }
}
