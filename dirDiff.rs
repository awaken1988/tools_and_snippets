use std::io;
use std::fs::{self, DirEntry};
use std::path::Path;
use std::collections::HashMap;
use std::collections::HashSet;
use std::fmt;
use std::rc::Rc;
use std::cell::RefCell;

#[derive(Debug)]
struct PathTree {
    children: HashMap<String, Rc<RefCell<PathTree>>>,
    parent: Option<Rc<RefCell<PathTree>>>,
    name : String,
    is_dir : bool,
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
}

impl fmt::Display for PathTree {

    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        self.walkprint(0, f);

        return Ok(());
    }
}


fn full_path(ptree: &PathTree) -> String
{
    let mut ret = String::new();

    match (ptree.parent.as_ref()) {
        None    => return ret,
        Some(x) => {
            ret = ptree.name.clone();
            let inner_str = full_path(&*x.borrow());
            return format!("{}/{}", inner_str, ret);
        }
    }

    return ret;
}

fn walkdir(dir: &Path) -> Rc<RefCell<PathTree>>
{
    let mut ret = Rc::new(RefCell::new(PathTree{ name: dir.file_name().unwrap().to_str().unwrap().to_string(),
                            parent: None,
                            is_dir: false,
                            children: HashMap::new()}));
    if dir.is_file()  {
        //println!("{}", dir.to_str().unwrap());
    } else {
        let dir_entries = fs::read_dir(dir.to_str().unwrap()).unwrap();
        ret.borrow_mut().is_dir = true;

        for entry in dir_entries {
            let child = walkdir(& entry.unwrap().path());

            let name = child.borrow().name.clone();

            ret.borrow_mut().children.insert(name, child.clone());
            child.borrow_mut().parent = Some(ret.clone());
        }
    }
    return ret;
}

fn compare_dir(left: &PathTree, right: &PathTree, diff_prefix: &String)
{
    let mut diff_map : HashSet<String> = HashSet::new();
    let sides = [(left, right), (right, left)];

    for (iNum, iSide) in sides.iter().enumerate()
    {
        if diff_map.contains(&iSide.0.name.clone()) {
            continue;
        };

        let prefix = { if 0 == iNum {"-"} else { "+" } };
        diff_map.insert(iSide.0.name.clone());
        for (iPath, iChild) in iSide.0.children.iter() {
            if !iSide.1.children.contains_key(iPath)  {
                println!("{}{}",prefix,  full_path(&*iChild.borrow()))
            } else {
                compare_dir(&*iSide.0.children[iPath].borrow(), &*iSide.1.children[iPath].borrow(), &"-".to_string());
            }
        }
    }

}

fn main()
{
    let left = walkdir(Path::new("./left"));
    let right = walkdir(Path::new("./right"));

    //example print dir
    //println!("{}", *left.borrow());
    //println!("{}", *right.borrow());

    //example: print type of variable
    //let ddd: () = *left.borrow();

    compare_dir(&*left.borrow(), &*right.borrow(), &"?".to_string() );
}
