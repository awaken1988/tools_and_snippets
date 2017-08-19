use std::io;
use std::fs::{self, DirEntry};
use std::path::Path;
use std::collections::HashMap;
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

//impl PathTree {
    fn full_path(ptree: &PathTree) -> String
    {
        let mut ret = String::new();

        let parent = (ptree.parent.as_ref()).unwrap();
        let inner_str = full_path(&*parent.borrow());
        //let ddd: () = &*ptree.parent.unwrap().borrow();

        ret = inner_str + "/" + &ret;

        return ret;
    }
//}



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

fn compare_dir(left: &PathTree, right: &PathTree)
{
    for (iPath, iChild) in left.children.iter() {
        if !right.children.contains_key(iPath)  {
            println!("-{}", full_path(&*iChild.borrow()))
        } else {
            compare_dir(&*left.children[iPath].borrow(), &*right.children[iPath].borrow());
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

    compare_dir(&*left.borrow(), &*right.borrow());
}
