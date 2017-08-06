use std::io;
use std::fs::{self, DirEntry};
use std::path::Path;
use std::collections::HashMap;
use std::fmt;

#[derive(Debug)]
struct PathTree {
    children: HashMap<String, Box<PathTree>>,
    //parent: Option<Box<PathTree>>,
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
                child.walkprint( depth+1, f );
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



fn walkdir(dir: &Path) -> Box<PathTree>
{
    let mut ret = PathTree{ name: dir.file_name().unwrap().to_str().unwrap().to_string(),
                            is_dir: false,
                            children: HashMap::new()};
    if dir.is_file()  {
        println!("{}", dir.to_str().unwrap());
    } else {
        let dir_entries = fs::read_dir(dir.to_str().unwrap()).unwrap();
        ret.is_dir = true;

        for entry in dir_entries {
            let child = walkdir(& entry.unwrap().path());

            ret.children.insert(child.name.clone(), child);
        }
    }
    return Box::new(ret);
}

fn main()
{


    let path_tree = walkdir(Path::new("E:/[music]/Beastie Boys/"));
    //walkprint(&path_tree, 0);
    println!("{}", path_tree);

}
